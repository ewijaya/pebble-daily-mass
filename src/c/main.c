#include <pebble.h>
#include "pmr.h"
#include "calendar.h"
#include "glance.h"
#include "planner.h"

static Window *menu_window, *reader_window, *options_window;
static MenuLayer *menu;
static SimpleMenuLayer *options_menu;
static TextLayer *title, *season, *celebration, *footer;
static Layer *body_layer;
static ScrollLayer *scroll;
static Pmr db;
static ResHandle resource, calendar_resource;
static Calendar *calendar;
static CalDay day;
static bool day_valid, details_mode;
static uint8_t reading_part;
static char citations[CAL_MAX_READINGS][CAL_CITATION_SIZE];
static char date_text[32], season_text[64], celebration_text[160], day_name[128], day_details[1600];
static int32_t browsing_date=-1; // -1 follows today; other values are absolute civil dates.
static int shown_year, shown_month, shown_day;
static int clock_day;
static bool clock_evening, evening_offer;
static int32_t evening_date=-1;
static bool paragraph_rubric[64];
static Window *date_window;
static SimpleMenuLayer *date_menu;
static Window *prompt_window;
static TextLayer *prompt_text;
static SimpleMenuLayer *prompt_menu;
static char prompt_message[192];
enum { PROMPT_EVENING, PROMPT_NOTICE };
static uint8_t prompt_mode;
static AppTimer *date_repeat_timer;
static int date_repeat_direction;
static void load_reading_part(uint8_t part);
static void refresh_date(bool force);
static void open_dates(ClickRecognizerRef recognizer, void *context);
static void open_evening(void);
static void step_date(int delta);

// Long readings are split at paragraph boundaries. Overflow is an error.
static char reading[8192];
// Separate drawing boxes allow a real paragraph gap smaller than one text line.
static struct { char *text; int16_t y, height; } paragraphs[64];
static size_t paragraph_count;
static GFont body_font;
static GFont rubric_font;
static bool styled_reading;
static int strip_height;
static int selected;
enum { FONT_SIZE_KEY = 1, RETIRED_PLACE_KEY = 2, FOOTER_HEIGHT = 22 };
static int font_size;
static int viewport_height, max_offset, page_step;
static char footer_text[48];
static uint32_t startup_ms;
static bool first_menu_draw;
static struct { GColor fill, text, ink; } theme;

static void refresh_theme(void) {
  // Dark ink on white pages; gold strips need black lettering for contrast.
  theme.text=GColorWhite;
  if (!day_valid) theme.fill=GColorDarkGray;
  else switch (day.color) {
    case CAL_GREEN: theme.fill=GColorDarkGreen; break;
    case CAL_PURPLE: theme.fill=GColorIndigo; break;
    case CAL_WHITE: theme.fill=GColorChromeYellow; theme.text=GColorBlack; break;
    case CAL_RED: theme.fill=GColorDarkCandyAppleRed; break;
    case CAL_ROSE: theme.fill=GColorRoseVale; break;
    default: theme.fill=GColorDarkGray; break;
  }
  theme.ink=day_valid && day.color==CAL_WHITE ? GColorWindsorTan : theme.fill;
  if (title) {
    text_layer_set_background_color(title,theme.fill);
    text_layer_set_text_color(title,theme.text);
  }
  if (season) text_layer_set_text_color(season,theme.ink);
  if (celebration) text_layer_set_text_color(celebration,theme.ink);
  if (menu) menu_layer_set_highlight_colors(menu,theme.fill,theme.text);
  if (date_menu) menu_layer_set_highlight_colors(simple_menu_layer_get_menu_layer(date_menu),theme.fill,theme.text);
  if (options_menu) menu_layer_set_highlight_colors(simple_menu_layer_get_menu_layer(options_menu),theme.fill,theme.text);
  if (prompt_menu) menu_layer_set_highlight_colors(simple_menu_layer_get_menu_layer(prompt_menu),theme.fill,theme.text);
}

static uint32_t now_ms(void) {
  time_t seconds;
  uint16_t milliseconds;
  time_ms(&seconds, &milliseconds);
  return (uint32_t)seconds * 1000 + milliseconds;
}
static const char *reading_titles[] = {"FIRST READING", "PSALM", "SECOND READING", "SEQUENCE", "ACCLAMATION", "GOSPEL"};
static const char *menu_titles[] = {"First Reading", "Psalm", "Second Reading", "Sequence", "Acclamation", "Gospel"};
static const char *season_names[] = {"Ordinary Time", "Advent", "Christmas Time", "Lent", "Paschal Triduum", "Easter Time"};
static const char *rank_names[] = {"Weekday", "Sunday", "Memorial", "Optional Memorial", "Feast", "Solemnity", "Commemoration"};

static bool is_rubric(size_t index) { return styled_reading && paragraph_rubric[index]; }

static void prepare_display_text(char *text) {
  // System Gothic fonts lack U+211F (response sign). Preserve the database,
  // but use the conventional ASCII R. on screen. The following dot is retained.
  char *src = text, *dest = text;
  while (*src) {
    if (strncmp(src, "\xe2\x84\x9f", 3) == 0) {
      *dest++ = 'R';
      src += 3;
    } else {
      *dest++ = *src++;
    }
  }
  *dest = 0;
}

static bool split_paragraphs(void) {
  paragraph_count = 0;
  char *start = reading;
  while (*start) {
    if (paragraph_count == ARRAY_LENGTH(paragraphs)) return false;
    paragraphs[paragraph_count++].text = start;
    char *end = strstr(start, "\n\n");
    if (!end) break;
    *end = 0;
    start = end + 2;
    while (*start == '\n') ++start;
  }
  return true;
}

static void draw_body(Layer *layer, GContext *ctx) {
  graphics_context_set_text_color(ctx, GColorBlack);
  int width = layer_get_bounds(layer).size.w;
  int top = -scroll_layer_get_content_offset(scroll).y;
  if (styled_reading && top < strip_height) {
    graphics_context_set_fill_color(ctx, theme.fill);
    graphics_fill_rect(ctx, GRect(0, 0, width, strip_height), 0, GCornerNone);
    graphics_context_set_text_color(ctx, theme.text);
    graphics_draw_text(ctx, reading_titles[day.readings[selected].kind], rubric_font,
        GRect(6, 1, width - 12, strip_height),
        GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  }
  for (size_t i = 0; i < paragraph_count; ++i) {
    // Only render paragraphs intersecting the viewport; preserve internal lines.
    if (paragraphs[i].y + paragraphs[i].height < top ||
        paragraphs[i].y > top + viewport_height) continue;
    bool rubric = is_rubric(i);
    // Printed-missal rubrics remain red; the title strip carries the day's color.
    graphics_context_set_text_color(ctx, rubric ? GColorDarkCandyAppleRed : GColorBlack);
    graphics_draw_text(ctx, paragraphs[i].text, rubric ? rubric_font : body_font,
        GRect(6, paragraphs[i].y, width - 12, paragraphs[i].height + 8),
        GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
  }
}

static size_t read_resource(void *context, uint32_t offset, uint8_t *out, size_t size) {
  return resource_load_byte_range(resource, offset, out, size);
}

static bool ensure_database(void) {
  if (db.ready) return true;
  uint32_t start = now_ms();
  resource = resource_get_handle(RESOURCE_ID_READINGS_DB);
  bool ok = resource && pmr_open(&db, read_resource, NULL, resource_size(resource));
  APP_LOG(APP_LOG_LEVEL_INFO, "Database ready=%d: %lu ms", ok, (unsigned long)(now_ms() - start));
  return ok;
}

static void move_to(int offset) {
  if (!scroll || !footer || page_step <= 0) return;
  if (offset < 0) offset = 0;
  if (offset > max_offset) offset = max_offset;
  scroll_layer_set_content_offset(scroll, GPoint(0, -offset), false);
  int pages = 1 + (max_offset + page_step - 1) / page_step;
  int page = offset == max_offset ? pages : 1 + offset / page_step;
  if (!details_mode && day_valid && selected < day.count && day.readings[selected].parts > 1)
    snprintf(footer_text, sizeof(footer_text), "%d/%d  Part %u/%u", page, pages, reading_part+1, day.readings[selected].parts);
  else snprintf(footer_text, sizeof(footer_text), "%d/%d  Select: options", page, pages);
  text_layer_set_text(footer, footer_text);
  APP_LOG(APP_LOG_LEVEL_INFO, "Reader size=%s offset=%d/%d page=%d/%d",
          font_size ? "XL" : "L", offset, max_offset, page, pages);
}

static void layout_reading(void) {
  if (!body_layer || !scroll || !footer) return;
  // Retain approximate progress when changing size; new readings start at top.
  int previous_max = max_offset;
  int previous_offset = -scroll_layer_get_content_offset(scroll).y;
  GRect bounds = layer_get_bounds(window_get_root_layer(reader_window));
  body_font = fonts_get_system_font(
      font_size ? FONT_KEY_GOTHIC_28_BOLD : FONT_KEY_GOTHIC_24_BOLD);
  rubric_font = fonts_get_system_font(
      font_size ? FONT_KEY_GOTHIC_24_BOLD : FONT_KEY_GOTHIC_18_BOLD);
  strip_height = font_size ? 30 : 24;
  int height = styled_reading ? strip_height + 4 : 0;
  for (size_t i = 0; i < paragraph_count; ++i) {
    GSize size = graphics_text_layout_get_content_size(paragraphs[i].text,
        is_rubric(i) ? rubric_font : body_font,
        GRect(0, 0, bounds.size.w - 12, 16000),
        GTextOverflowModeWordWrap, GTextAlignmentLeft);
    paragraphs[i].y = height;
    paragraphs[i].height = size.h;
    height += size.h;
    if (i + 1 < paragraph_count) height += 8;
  }
  layer_set_frame(body_layer, GRect(0, 0, bounds.size.w, height));
  layer_mark_dirty(body_layer);
  if (height < viewport_height) height = viewport_height;
  scroll_layer_set_content_size(scroll, GSize(bounds.size.w, height));
  max_offset = height - viewport_height;
  // Overlap by more than one line so clipped edge text is readable next page.
  page_step = viewport_height - (font_size ? 36 : 32);
  int offset = previous_max ? previous_offset * max_offset / previous_max : 0;
  move_to(offset == max_offset ? offset : (offset / page_step) * page_step);
}

static void page_up(ClickRecognizerRef recognizer, void *context) {
  if (scroll && page_step > 0) {
    int offset = -scroll_layer_get_content_offset(scroll).y;
    if (offset == 0 && !details_mode && reading_part > 0) {
      load_reading_part(reading_part-1); move_to(max_offset);
    } else move_to(offset > 0 ? ((offset - 1) / page_step) * page_step : 0);
  }
}

static void page_down(ClickRecognizerRef recognizer, void *context) {
  if (!scroll || page_step <= 0) return;
  int offset = -scroll_layer_get_content_offset(scroll).y;
  if (offset == max_offset && !details_mode && reading_part+1 < day.readings[selected].parts)
    load_reading_part(reading_part+1);
  else move_to((offset / page_step + 1) * page_step);
}

static void jump_top(ClickRecognizerRef recognizer, void *context) {
  if (!details_mode && reading_part) load_reading_part(0);
  move_to(0);
}
static void jump_bottom(ClickRecognizerRef recognizer, void *context) {
  if (!details_mode && reading_part+1 < day.readings[selected].parts)
    load_reading_part(day.readings[selected].parts-1);
  move_to(max_offset);
}

static void open_options(ClickRecognizerRef recognizer, void *context) {
  if (scroll && body_layer && footer) window_stack_push(options_window, true);
}

static void reader_click_config(void *context) {
  // Own Up/Down explicitly: native ScrollLayer repeats would conflict with jumps.
  window_single_click_subscribe(BUTTON_ID_UP, page_up);
  window_single_click_subscribe(BUTTON_ID_DOWN, page_down);
  window_long_click_subscribe(BUTTON_ID_UP, 700, jump_top, NULL);
  window_long_click_subscribe(BUTTON_ID_DOWN, 700, jump_bottom, NULL);
  window_single_click_subscribe(BUTTON_ID_SELECT, open_options);
}

static void choose_option(int index, void *context) {
  if (index < 2) {
    font_size = index;
    if (persist_write_int(FONT_SIZE_KEY, font_size) < 0)
      APP_LOG(APP_LOG_LEVEL_ERROR, "Could not save font size");
    layout_reading();
  } else {
    if (index == 2) jump_top(NULL, NULL); else jump_bottom(NULL, NULL);
  }
  window_stack_pop(true);
}

static const SimpleMenuItem option_items[] = {
  {.title = "Large", .subtitle = "24 px bold", .callback = choose_option},
  {.title = "Extra Large", .subtitle = "28 px bold", .callback = choose_option},
  {.title = "Jump to top", .subtitle = "Shortcut: hold Up", .callback = choose_option},
  {.title = "Jump to bottom", .subtitle = "Shortcut: hold Down", .callback = choose_option},
};
static const SimpleMenuSection option_section = {.items = option_items, .num_items = 4};

static void options_load(Window *window) {
  GRect bounds = layer_get_bounds(window_get_root_layer(window));
  options_menu = simple_menu_layer_create(bounds, window, &option_section, 1, NULL);
  if (options_menu) {
    menu_layer_set_normal_colors(simple_menu_layer_get_menu_layer(options_menu), GColorWhite, GColorBlack);
    menu_layer_set_highlight_colors(simple_menu_layer_get_menu_layer(options_menu), theme.fill, theme.text);
    layer_add_child(window_get_root_layer(window), simple_menu_layer_get_layer(options_menu));
    simple_menu_layer_set_selected_index(options_menu, font_size, false);
  }
}

static void options_unload(Window *window) {
  if (options_menu) simple_menu_layer_destroy(options_menu);
  options_menu = NULL;
}

static void load_reading_part(uint8_t part) {
  uint32_t start_ms = now_ms();
  bool ok = true;
  memset(paragraph_rubric, 0, sizeof(paragraph_rubric));
  reading_part = part;
  if (details_mode) {
    snprintf(reading, sizeof(reading), "%s\n\n%s\n\nGeneral Roman Calendar\nEpiphany: January 6\nAscension and Corpus Christi: Thursday\nSunday cycle %c; weekday cycle %s\n\nOffline calendar: 2020-2037\nHold Select on the menu to browse dates.",
        date_text, day_details, day_valid ? 'A'+(day.cycles&3) : '?', day_valid && (day.cycles&4) ? "II" : "I");
  } else {
    uint16_t refs[CAL_MAX_REFS], count=0;
    ok = day_valid && selected < day.count && calendar_part(calendar, &day.readings[selected], part, refs, &count) && ensure_database();
    size_t used=0, paragraph=0;
    reading[0]=0;
    if (ok && citations[selected][0]) {
      used=snprintf(reading,sizeof(reading),"%s",citations[selected]);
      paragraph_rubric[paragraph++]=true;
    }
    for (uint16_t i=0;ok && i<count;i++) {
      if (paragraph >= ARRAY_LENGTH(paragraph_rubric) || sizeof(reading)-used <= 2) {ok=false;break;}
      if (used) {reading[used++]='\n';reading[used++]='\n';}
      uint32_t length;
      ok=pmr_segment(&db,refs[i]&0x7fff,reading+used,sizeof(reading)-used,&length);
      if (ok) {used+=length;paragraph_rubric[paragraph++]=(refs[i]&0x8000)!=0;}
    }
  }
  if (ok) prepare_display_text(reading);
  else snprintf(reading,sizeof(reading),"Reading unavailable.\n\nThe bundled reading could not be loaded. Press Back to return.");
  if (!split_paragraphs()) {
    ok=false;
    snprintf(reading,sizeof(reading),"Reading unavailable. Too many paragraphs. Press Back to return.");
    split_paragraphs();
  }
  styled_reading=ok && !details_mode;
  max_offset=0;
  if (scroll) scroll_layer_set_content_offset(scroll,GPointZero,false);
  layout_reading();
  APP_LOG(APP_LOG_LEVEL_INFO,"Reading %d part %u: ok=%d prepared=%lu ms heap=%u", selected, part+1, ok, (unsigned long)(now_ms()-start_ms),(unsigned)heap_bytes_free());
}

static void reader_load(Window *window) {
  GRect bounds = layer_get_bounds(window_get_root_layer(window));
  viewport_height = bounds.size.h - FOOTER_HEIGHT;
  max_offset = 0;
  scroll = scroll_layer_create(GRect(0, 0, bounds.size.w, viewport_height));
  body_layer = layer_create(GRect(0, 0, bounds.size.w, 16000));
  footer = text_layer_create(GRect(0, viewport_height, bounds.size.w, FOOTER_HEIGHT));
  if (!scroll || !body_layer || !footer) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Reader UI allocation failed");
    return;
  }
  layer_set_update_proc(body_layer, draw_body);
  scroll_layer_add_child(scroll, body_layer);
  layer_add_child(window_get_root_layer(window), scroll_layer_get_layer(scroll));
  scroll_layer_set_shadow_hidden(scroll, true);
  text_layer_set_font(footer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(footer, GTextAlignmentCenter);
  text_layer_set_background_color(footer, GColorBlack);
  text_layer_set_text_color(footer, GColorWhite);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(footer));
  load_reading_part(0);
}

static void reader_unload(Window *window) {
  if (footer) text_layer_destroy(footer);
  if (body_layer) layer_destroy(body_layer);
  if (scroll) scroll_layer_destroy(scroll);
  body_layer = NULL;
  scroll = NULL;
  footer = NULL;
}

static void select_reading(MenuLayer *layer, MenuIndex *index, void *context) {
  if (evening_offer && !index->row) {open_evening();return;}
  selected = index->row-(evening_offer ? 1 : 0);
  details_mode = selected >= day.count;
  window_stack_push(reader_window, true);
}

static uint16_t menu_row_count(MenuLayer *layer, uint16_t section, void *context) {
  return day.count + 1 + (evening_offer ? 1 : 0);
}

static int16_t menu_row_height(MenuLayer *layer, MenuIndex *index, void *context) {
  return 44;
}

static void main_menu_up(ClickRecognizerRef recognizer, void *context) {
  menu_layer_set_selected_next(menu, true, MenuRowAlignCenter, false);
}

static void main_menu_down(ClickRecognizerRef recognizer, void *context) {
  menu_layer_set_selected_next(menu, false, MenuRowAlignCenter, false);
}

static void repeat_date(void *context) {
  date_repeat_timer=NULL;
  if (window_stack_get_top_window()!=menu_window) return;
  step_date(date_repeat_direction);
  date_repeat_timer=app_timer_register(450,repeat_date,NULL);
}
static void stop_date_repeat(ClickRecognizerRef recognizer, void *context) {
  if (date_repeat_timer) app_timer_cancel(date_repeat_timer);
  date_repeat_timer=NULL;
}
static void hold_previous(ClickRecognizerRef recognizer, void *context) {
  stop_date_repeat(NULL,NULL);date_repeat_direction=-1;repeat_date(NULL);
}
static void hold_next(ClickRecognizerRef recognizer, void *context) {
  stop_date_repeat(NULL,NULL);date_repeat_direction=1;repeat_date(NULL);
}

static void main_menu_select(ClickRecognizerRef recognizer, void *context) {
  MenuIndex index = menu_layer_get_selected_index(menu);
  select_reading(menu, &index, NULL);
}

static void main_menu_click_config(void *context) {
  // Native row alignment keeps longer Sunday/solemnity menus reachable.
  window_single_click_subscribe(BUTTON_ID_UP, main_menu_up);
  window_single_click_subscribe(BUTTON_ID_DOWN, main_menu_down);
  window_long_click_subscribe(BUTTON_ID_UP,700,hold_previous,stop_date_repeat);
  window_long_click_subscribe(BUTTON_ID_DOWN,700,hold_next,stop_date_repeat);
  window_single_click_subscribe(BUTTON_ID_SELECT, main_menu_select);
  window_long_click_subscribe(BUTTON_ID_SELECT,700,open_dates,NULL);
}

static void menu_draw_row(GContext *ctx, const Layer *cell, MenuIndex *index, void *context) {
  if (!first_menu_draw) {
    first_menu_draw = true;
    APP_LOG(APP_LOG_LEVEL_INFO, "Startup first menu draw: %lu ms", (unsigned long)(now_ms() - startup_ms));
  }
  GRect bounds = layer_get_bounds(cell);
  bool highlighted = menu_cell_layer_is_highlighted(cell);
  int row=(int)index->row-(evening_offer ? 1 : 0);
  graphics_context_set_fill_color(ctx, highlighted ? theme.fill : GColorWhite);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_text_color(ctx, highlighted ? theme.text : GColorBlack);
  graphics_draw_text(ctx, row<0 ? "Evening Mass" : row < day.count ? menu_titles[day.readings[row].kind] : "Day details", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
      GRect(10, 0, bounds.size.w - 20, 24), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
  GColor reference_ink=row>=0 && row<day.count ? GColorDarkCandyAppleRed : theme.ink;
  graphics_context_set_text_color(ctx, highlighted ? theme.text : reference_ink);
  graphics_draw_text(ctx, row<0 ? "Tomorrow's readings" : row < day.count ? (citations[row][0] ? citations[row] : "Read text") : "Celebration & calendar", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
      GRect(10, 24, bounds.size.w - 20, 20), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
}

static size_t read_calendar_resource(void *context, uint32_t offset, uint8_t *out, size_t size) {
  return resource_load_byte_range(calendar_resource, offset, out, size);
}

static void refresh_date(bool force) {
  time_t now=time(NULL);
  struct tm local=*localtime(&now);
  int today=calendar_day_number(local.tm_year+1900,local.tm_mon+1,local.tm_mday);
  bool after_four=local.tm_hour>=16;
  if (!force && today==clock_day && after_four==clock_evening) return;
  clock_day=today;
  clock_evening=after_four;
  if (browsing_date>=0) planner_date(browsing_date,&local);
  shown_year=local.tm_year+1900; shown_month=local.tm_mon+1;shown_day=local.tm_mday;
  strftime(date_text,sizeof(date_text),"%a %d %b %Y",&local);
  if (calendar && !calendar->ready) {
    calendar_resource=resource_get_handle(RESOURCE_ID_CALENDAR_DB);
    if (calendar_resource) calendar_open(calendar,read_calendar_resource,NULL,resource_size(calendar_resource));
  }
  evening_date=planner_evening(calendar,now);
  evening_offer=evening_date>=0 && (browsing_date<0 || browsing_date==today);
  day_valid=calendar && calendar_day(calendar,shown_year,shown_month,shown_day,&day);
  if (day_valid) {
    day_valid=calendar_string(calendar,day.name,day_name,sizeof(day_name)) &&
              calendar_string(calendar,day.details,day_details,sizeof(day_details));
    for (uint8_t i=0;day_valid && i<day.count;i++)
      day_valid=calendar_string(calendar,day.readings[i].citation,citations[i],sizeof(citations[i]));
  }
  if (!day_valid) {
    memset(&day,0,sizeof(day));
    snprintf(season_text,sizeof(season_text),"Calendar unavailable");
    snprintf(day_details,sizeof(day_details),"No calendar entry could be loaded for this date.\n\nThis edition covers 2020-2037. Check the watch date or update the app.");
    celebration_text[0]=0;
  } else {
    if (day.week && day.season!=2 && day.season!=4)
      snprintf(season_text,sizeof(season_text),"%s - Week %u",season_names[day.season],day.week);
    else snprintf(season_text,sizeof(season_text),"%s",season_names[day.season]);
    if (day.rank || !day.week || day.season==4 || day.season==2 || !day.count)
      snprintf(celebration_text,sizeof(celebration_text),"%s: %s",rank_names[day.rank],day_name);
    else celebration_text[0]=0;
  }
  refresh_theme();
  if (title) text_layer_set_text(title,date_text);
  if (season) text_layer_set_text(season,season_text);
  if (celebration) text_layer_set_text(celebration,celebration_text);
  if (menu) {
    int top=celebration_text[0] ? 94 : 52;
    GRect bounds=layer_get_bounds(window_get_root_layer(menu_window));
    layer_set_frame(menu_layer_get_layer(menu),GRect(0,top,bounds.size.w,bounds.size.h-top));
    menu_layer_reload_data(menu);
    menu_layer_set_selected_index(menu,(MenuIndex){0,0},MenuRowAlignTop,false);
  }
  APP_LOG(APP_LOG_LEVEL_INFO,"Calendar %04d-%02d-%02d: ok=%d readings=%u",shown_year,shown_month,shown_day,day_valid,day.count);
}

static void minute_tick(struct tm *tick_time, TimeUnits changed) {
  // Preserve an open reading across midnight; refresh when its menu reappears.
  if (window_stack_get_top_window()==menu_window) refresh_date(false);
}
static void menu_appear(Window *window) { refresh_date(false); }
static bool show_date(int32_t number) {
  if (number>=0 && (!calendar || !calendar->ready || (uint32_t)number<calendar->first_day ||
      (uint32_t)number-calendar->first_day>=calendar->day_count)) return false;
  browsing_date=number;
  refresh_date(true);
  return true;
}
static void step_date(int delta) {
  int32_t number=calendar_day_number(shown_year,shown_month,shown_day);
  show_date(number+delta);
}

static SimpleMenuItem prompt_items[2];
static const SimpleMenuSection prompt_section={.items=prompt_items,.num_items=2};
static void choose_prompt(int index, void *context) {
  if (prompt_mode==PROMPT_EVENING && !index) {
    // Recheck in case the prompt stayed open over midnight or a clock change.
    int32_t target=planner_evening(calendar,time(NULL));
    if (target>=0) show_date(target);
    else show_date(-1);
  }
  window_stack_pop(false);
}
static void prompt_load(Window *window) {
  Layer *root=window_get_root_layer(window);
  GRect bounds=layer_get_bounds(root);
  prompt_text=text_layer_create(GRect(6,3,bounds.size.w-12,120));
  if (prompt_text) {
    text_layer_set_font(prompt_text,fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text(prompt_text,prompt_message);
    text_layer_set_text_color(prompt_text,GColorBlack);
    layer_add_child(root,text_layer_get_layer(prompt_text));
  }
  prompt_items[0]=(SimpleMenuItem){.title=prompt_mode==PROMPT_EVENING ? "Open tomorrow" : "OK",.callback=choose_prompt};
  prompt_items[1]=(SimpleMenuItem){.title="Back",.callback=choose_prompt};
  prompt_menu=simple_menu_layer_create(GRect(0,126,bounds.size.w,bounds.size.h-126),window,&prompt_section,1,NULL);
  if (prompt_menu) {
    menu_layer_set_highlight_colors(simple_menu_layer_get_menu_layer(prompt_menu),theme.fill,theme.text);
    layer_add_child(root,simple_menu_layer_get_layer(prompt_menu));
  }
}
static void prompt_unload(Window *window) {
  if (prompt_menu) simple_menu_layer_destroy(prompt_menu);
  if (prompt_text) text_layer_destroy(prompt_text);
  prompt_menu=NULL;prompt_text=NULL;
}
static void open_evening(void) {
  struct tm tomorrow;
  evening_date=planner_evening(calendar,time(NULL));
  if (!planner_date(evening_date,&tomorrow)) return;
  char date[32];strftime(date,sizeof(date),"%a %d %b %Y",&tomorrow);
  snprintf(prompt_message,sizeof(prompt_message),"Tomorrow's Mass\n%s\nDay Mass readings.\nSeparate vigil texts\nare not included.",date);
  prompt_mode=PROMPT_EVENING;
  window_stack_push(prompt_window,true);
}
static void choose_date(int index, void *context) {
  int32_t current=calendar_day_number(shown_year,shown_month,shown_day), target=-1;
  if (index==5) {window_stack_pop(false);open_evening();return;}
  if (index==1 || index==2) target=current+(index==1 ? -1 : 1);
  else if (index==3) target=planner_sunday(current);
  else if (index==4) target=planner_holy_day(calendar,current);
  if ((index==4 && target<0) || !show_date(target)) {
    snprintf(prompt_message,sizeof(prompt_message),"Date unavailable.\nThis calendar covers\n2020-2037.");
    prompt_mode=PROMPT_NOTICE;
    window_stack_pop(false);window_stack_push(prompt_window,true);return;
  }
  window_stack_pop(true);
}
static const SimpleMenuItem date_items[] = {
  {.title="Today",.subtitle="Main menu: hold Up/Down",.callback=choose_date},
  {.title="Previous day",.callback=choose_date},
  {.title="Next day",.callback=choose_date},
  {.title="Next Sunday",.callback=choose_date},
  {.title="Next Holy Day",.subtitle="Feast or solemnity",.callback=choose_date},
  {.title="Evening Mass",.subtitle="Tomorrow's readings",.callback=choose_date},
};
static SimpleMenuSection date_section={.title="Date shortcuts",.items=date_items,.num_items=5};
static void date_load(Window *window) {
  date_section.num_items=planner_evening(calendar,time(NULL))>=0 ? 6 : 5;
  date_menu=simple_menu_layer_create(layer_get_bounds(window_get_root_layer(window)),window,&date_section,1,NULL);
  if (date_menu) {
    menu_layer_set_highlight_colors(simple_menu_layer_get_menu_layer(date_menu),theme.fill,theme.text);
    layer_add_child(window_get_root_layer(window),simple_menu_layer_get_layer(date_menu));
  }
}
static void date_unload(Window *window) { if (date_menu) simple_menu_layer_destroy(date_menu);date_menu=NULL; }
static void open_dates(ClickRecognizerRef recognizer, void *context) { window_stack_push(date_window,true); }

static void menu_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);
  title = text_layer_create(GRect(0, 0, bounds.size.w, 24));
  if (title) {
    text_layer_set_font(title, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(title, GTextAlignmentCenter);
    text_layer_set_text(title, date_text);
    layer_add_child(root, text_layer_get_layer(title));
  }
  season = text_layer_create(GRect(0, 26, bounds.size.w, 24));
  if (season) {
    text_layer_set_font(season, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(season, GTextAlignmentCenter);
    text_layer_set_text(season, season_text);
    layer_add_child(root, text_layer_get_layer(season));
  }
  celebration = text_layer_create(GRect(6, 49, bounds.size.w-12, 44));
  if (celebration) {
    text_layer_set_font(celebration,fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(celebration,GTextAlignmentCenter);
    text_layer_set_overflow_mode(celebration,GTextOverflowModeTrailingEllipsis);
    layer_add_child(root,text_layer_get_layer(celebration));
  }
  menu = menu_layer_create(GRect(0, 52, bounds.size.w, bounds.size.h - 52));
  if (menu) {
    menu_layer_set_callbacks(menu, NULL, (MenuLayerCallbacks){
        .get_num_rows = menu_row_count, .get_cell_height = menu_row_height,
        .draw_row = menu_draw_row, .select_click = select_reading});
    menu_layer_set_normal_colors(menu, GColorWhite, GColorBlack);
    menu_layer_pad_bottom_enable(menu, false);
    window_set_click_config_provider(window, main_menu_click_config);
    layer_add_child(root, menu_layer_get_layer(menu));
  }
  refresh_date(true);
}

static void menu_unload(Window *window) {
  if (menu) menu_layer_destroy(menu);
  if (title) text_layer_destroy(title);
  if (season) text_layer_destroy(season);
  if (celebration) text_layer_destroy(celebration);
  menu = NULL;
  title = season = celebration = NULL;
}

static void glance_reload(AppGlanceReloadSession *session, size_t limit, void *context) {
  if (!limit) return;
  // Reserve a permanent, date-neutral fallback when capacity permits.
  size_t days=limit>1 ? limit-1 : 1;
  if (days>GLANCE_DAYS) days=GLANCE_DAYS;
  time_t when=time(NULL);
  size_t added=0;
  for (;added<days;added++) {
    char subtitle[GLANCE_TEXT_SIZE];
    if (!glance_summary(calendar,when,subtitle)) break;
    time_t expires=glance_next_midnight(when);
    if (!expires) break;
    AppGlanceSlice slice={
      .layout={.icon=APP_GLANCE_SLICE_DEFAULT_ICON,.subtitle_template_string=subtitle},
      .expiration_time=expires,
    };
    AppGlanceResult result=app_glance_add_slice(session,slice);
    if (!added) APP_LOG(APP_LOG_LEVEL_INFO,"Glance today: %s until %lu (now %lu)",subtitle,(unsigned long)expires,(unsigned long)when);
    if (result!=APP_GLANCE_RESULT_SUCCESS) {
      APP_LOG(APP_LOG_LEVEL_WARNING,"Glance add failed: %d",result);
      return;
    }
    when=expires;
  }
  if (added<limit) {
    AppGlanceSlice fallback={
      .layout={.icon=APP_GLANCE_SLICE_DEFAULT_ICON,.subtitle_template_string="Open for today's readings"},
      .expiration_time=APP_GLANCE_SLICE_NO_EXPIRATION,
    };
    AppGlanceResult result=app_glance_add_slice(session,fallback);
    if (result!=APP_GLANCE_RESULT_SUCCESS) APP_LOG(APP_LOG_LEVEL_WARNING,"Glance fallback failed: %d",result);
  }
  APP_LOG(APP_LOG_LEVEL_INFO,"Glance saved: %u daily slices, capacity %u",(unsigned)added,(unsigned)limit);
}

int main(void) {
  startup_ms = now_ms();
  // Remove bookmarks left by the retired resume feature; keep the font preference.
  if (persist_exists(RETIRED_PLACE_KEY)) persist_delete(RETIRED_PLACE_KEY);
  font_size = persist_exists(FONT_SIZE_KEY) && persist_read_int(FONT_SIZE_KEY) == 0 ? 0 : 1;
  // The small offline calendar opens first; the scripture database stays lazy.
  calendar = calloc(1, sizeof(*calendar));
  menu_window = window_create();
  reader_window = window_create();
  options_window = window_create();
  date_window = window_create();
  prompt_window = window_create();
  if (menu_window && reader_window && options_window && date_window && prompt_window) {
    window_set_window_handlers(menu_window, (WindowHandlers){.load = menu_load, .unload = menu_unload, .appear = menu_appear});
    window_set_window_handlers(reader_window, (WindowHandlers){.load = reader_load, .unload = reader_unload});
    window_set_window_handlers(options_window, (WindowHandlers){.load = options_load, .unload = options_unload});
    window_set_window_handlers(date_window,(WindowHandlers){.load=date_load,.unload=date_unload});
    window_set_window_handlers(prompt_window,(WindowHandlers){.load=prompt_load,.unload=prompt_unload});
    tick_timer_service_subscribe(MINUTE_UNIT,minute_tick);
    window_set_background_color(reader_window, GColorWhite);
    window_set_click_config_provider(reader_window, reader_click_config);
    window_stack_push(menu_window, true);
    app_event_loop();
  }
  tick_timer_service_unsubscribe();
  stop_date_repeat(NULL,NULL);
  // Refresh on exit using the real clock, not the date-browsing selection.
  // The system advances the saved slices even while this app is closed.
  app_glance_reload(glance_reload,NULL);
  if (prompt_window) window_destroy(prompt_window);
  if (date_window) window_destroy(date_window);
  if (options_window) window_destroy(options_window);
  if (reader_window) window_destroy(reader_window);
  if (menu_window) window_destroy(menu_window);
  free(calendar);
}
