#include <pebble.h>
#include "pmr.h"
#include "calendar.h"
#include "glance.h"
#include "planner.h"

static Window *menu_window, *reader_window, *options_window;
static MenuLayer *menu;
static MenuLayer *options_menu;
static TextLayer *title, *season, *celebration;
static TextLayer *settings_version;
static Layer *body_layer, *ribbon_layer;
static Animation *ribbon_animation;
static int ribbon_length, ribbon_target;
static ScrollLayer *scroll;
static Pmr db;
static ResHandle resource, calendar_resource;
static Calendar *calendar;
static CalDay day;
static bool day_valid, details_mode, end_card;
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
static MenuLayer *date_menu;
static Window *prompt_window;
static TextLayer *prompt_text;
static ScrollLayer *prompt_scroll;
static TextLayer *prompt_footer;
static int prompt_max;
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
static void layout_menu(void);
static void open_options(ClickRecognizerRef recognizer, void *context);

// Long readings are split at paragraph boundaries. Overflow is an error.
enum { READING_CAPACITY = 8192 };
static char *reading; // Allocated only while the reader is open.
// Separate drawing boxes allow a real paragraph gap smaller than one text line.
static struct { char *text; int16_t y, height; } paragraphs[64];
static size_t paragraph_count;
static GFont body_font;
static GFont rubric_font;
static bool styled_reading;
static int strip_height;
static int selected;
enum { FONT_SIZE_KEY = 1, RETIRED_PLACE_KEY = 2, DARK_MODE_KEY = 3, FOOTER_HEIGHT = 22 };
static int font_size;
static bool dark_mode;
static char settings_summary[32];
static int viewport_height, max_offset, page_step;
static uint32_t startup_ms;
static bool first_menu_draw;
static struct { GColor fill, text, ink, paper, body, rubric; } theme;

static GFont text_font(void) {
  return fonts_get_system_font(font_size ? FONT_KEY_GOTHIC_28_BOLD : FONT_KEY_GOTHIC_24_BOLD);
}
static GFont small_font(void) {
  return fonts_get_system_font(font_size ? FONT_KEY_GOTHIC_24_BOLD : FONT_KEY_GOTHIC_18_BOLD);
}
static void style_menu(MenuLayer *layer) {
  if (!layer) return;
  menu_layer_set_normal_colors(layer,theme.paper,theme.body);
  menu_layer_set_highlight_colors(layer,theme.fill,theme.text);
}

static void refresh_theme(void) {
  // Dark ink on white pages; gold strips need black lettering for contrast.
  theme.paper=dark_mode ? GColorBlack : GColorWhite;
  theme.body=dark_mode ? GColorWhite : GColorBlack;
  theme.rubric=dark_mode ? GColorMelon : GColorDarkCandyAppleRed;
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
  if (dark_mode) {
    theme.ink=!day_valid ? GColorLightGray : day.color==CAL_GREEN ? GColorMintGreen :
        day.color==CAL_PURPLE ? GColorLavenderIndigo : day.color==CAL_WHITE ? GColorPastelYellow : theme.rubric;
  }
  Window *windows[]={menu_window,reader_window,options_window,date_window,prompt_window};
  for (size_t i=0;i<ARRAY_LENGTH(windows);++i)
    if (windows[i]) window_set_background_color(windows[i],theme.paper);
  if (title) {
    text_layer_set_background_color(title,theme.fill);
    text_layer_set_text_color(title,theme.text);
  }
  if (season) {text_layer_set_text_color(season,theme.ink);text_layer_set_background_color(season,GColorClear);}
  if (celebration) {text_layer_set_text_color(celebration,theme.ink);text_layer_set_background_color(celebration,GColorClear);}
  style_menu(menu);style_menu(date_menu);style_menu(options_menu);
  if (settings_version) text_layer_set_text_color(settings_version,dark_mode ? GColorLightGray : GColorDarkGray);
  if (body_layer) layer_mark_dirty(body_layer);
  snprintf(settings_summary,sizeof(settings_summary),"%s text · %s",font_size ? "XL" : "Large",dark_mode ? "Dark" : "Light");
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

static int reading_width(int width) {return width-14;}

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
  graphics_context_set_text_color(ctx, theme.body);
  int width = layer_get_bounds(layer).size.w;
  int top = -scroll_layer_get_content_offset(scroll).y;
  if (end_card) {
    bool next=selected+1<day.count;
    graphics_context_set_fill_color(ctx,theme.fill);
    graphics_fill_rect(ctx,GRect(0,0,width,strip_height),0,GCornerNone);
    graphics_context_set_text_color(ctx,theme.text);
    graphics_draw_text(ctx,next ? "NEXT READING" : "READINGS COMPLETE",rubric_font,
        GRect(4,0,width-8,strip_height),GTextOverflowModeTrailingEllipsis,GTextAlignmentCenter,NULL);
    graphics_context_set_text_color(ctx,theme.body);
    graphics_draw_text(ctx,next ? menu_titles[day.readings[selected+1].kind] : "End of readings",body_font,
        GRect(8,strip_height+8,width-16,64),GTextOverflowModeWordWrap,GTextAlignmentCenter,NULL);
    graphics_context_set_text_color(ctx,theme.rubric);
    graphics_draw_text(ctx,next ? citations[selected+1] : "Return to the menu with Back.",rubric_font,
        GRect(8,strip_height+76,width-16,viewport_height-strip_height-124),GTextOverflowModeTrailingEllipsis,GTextAlignmentCenter,NULL);
    graphics_context_set_text_color(ctx,theme.body);
    graphics_draw_text(ctx,"Up: return to reading",fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
        GRect(4,viewport_height-48,width-8,22),GTextOverflowModeWordWrap,GTextAlignmentCenter,NULL);
    graphics_draw_text(ctx,next ? "Down: next reading" : "Back: readings menu",fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
        GRect(4,viewport_height-26,width-8,26),GTextOverflowModeWordWrap,GTextAlignmentCenter,NULL);
    return;
  }
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
    graphics_context_set_text_color(ctx, rubric ? theme.rubric : theme.body);
    graphics_draw_text(ctx, paragraphs[i].text, rubric ? rubric_font : body_font,
        GRect(6, paragraphs[i].y, reading_width(width), paragraphs[i].height + 8),
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

static void ribbon_draw(Layer *layer,GContext *ctx) {
  int length=ribbon_length,x=layer_get_bounds(layer).size.w-5;
  if (length<=0) return;
  graphics_context_set_fill_color(ctx,GColorBulgarianRose);
  graphics_fill_rect(ctx,GRect(x,0,1,length),0,GCornerNone);
  graphics_context_set_fill_color(ctx,GColorDarkCandyAppleRed);
  // Pixel-exact swallowtail: a narrow stepped V, with no fifth edge pixel.
  int body=length>3 ? length-3 : length;
  graphics_fill_rect(ctx,GRect(x+1,0,3,body),0,GCornerNone);
  if (length>3) {
    graphics_fill_rect(ctx,GRect(x+1,body,1,1),0,GCornerNone);
    graphics_fill_rect(ctx,GRect(x+3,body,1,3),0,GCornerNone);
  }
}
static void ribbon_stop(void) {
  if (!ribbon_animation) return;
  Animation *animation=ribbon_animation;ribbon_animation=NULL;
  // SDK 3 owns scheduled animations, including their destruction on cancel.
  animation_unschedule(animation);
}
static void ribbon_update(int offset) {
  if (!ribbon_layer) return;
  ribbon_stop();
  bool visible=styled_reading && !details_mode && !end_card;
  layer_set_hidden(ribbon_layer,!visible);
  if (!visible) return;
  uint32_t height=max_offset+viewport_height;
  uint32_t parts=day.readings[selected].parts;
  uint32_t progress=reading_part*height+offset+viewport_height;
  uint32_t total=parts*height;
  if (progress>total) progress=total;
  int minimum=strip_height+12,maximum=viewport_height-2;
  if (minimum>maximum) minimum=maximum;
  ribbon_target=minimum+(maximum-minimum)*progress/total;
  ribbon_length=ribbon_target;layer_mark_dirty(ribbon_layer);
  APP_LOG(APP_LOG_LEVEL_INFO,"Ribbon part=%u/%u offset=%d height=%lu length=%d",reading_part+1,(unsigned)parts,offset,(unsigned long)height,ribbon_target);
}
static void ribbon_animate(Animation *animation,const AnimationProgress progress) {
  if (!ribbon_layer) return;
  ribbon_length=(uint32_t)ribbon_target*progress/ANIMATION_NORMALIZED_MAX;
  layer_mark_dirty(ribbon_layer);
}
static void ribbon_finished(Animation *animation,bool finished,void *context) {
  ribbon_animation=NULL;
  APP_LOG(APP_LOG_LEVEL_INFO,"Ribbon animation stopped finished=%d",finished);
}
static const AnimationImplementation ribbon_implementation={.update=ribbon_animate};
static void ribbon_open(void) {
  if (!ribbon_layer || !styled_reading || details_mode) return;
  unsigned before=heap_bytes_free();ribbon_animation=animation_create();
  if (!ribbon_animation) return;
  animation_set_implementation(ribbon_animation,&ribbon_implementation);
  animation_set_duration(ribbon_animation,200);
  animation_set_curve(ribbon_animation,AnimationCurveEaseOut);
  animation_set_handlers(ribbon_animation,(AnimationHandlers){.stopped=ribbon_finished},NULL);
  ribbon_length=0;layer_mark_dirty(ribbon_layer);
  APP_LOG(APP_LOG_LEVEL_INFO,"Ribbon animation heap=%u->%u",before,(unsigned)heap_bytes_free());
  if (!animation_schedule(ribbon_animation)) {
    animation_destroy(ribbon_animation);ribbon_animation=NULL;
    ribbon_length=ribbon_target;layer_mark_dirty(ribbon_layer);
  }
}

static void move_to(int offset) {
  if (!scroll || !body_layer || page_step <= 0) return;
  if (offset < 0) offset = 0;
  if (offset > max_offset) offset = max_offset;
  scroll_layer_set_content_offset(scroll, GPoint(0, end_card ? 0 : -offset), false);
  layer_mark_dirty(body_layer);
  ribbon_update(offset);
  if (end_card) return;
  int pages = 1 + (max_offset + page_step - 1) / page_step;
  int page = offset == max_offset ? pages : 1 + offset / page_step;
  APP_LOG(APP_LOG_LEVEL_INFO, "Reader size=%s offset=%d/%d page=%d/%d",
          font_size ? "XL" : "L", offset, max_offset, page, pages);
}

static void layout_reading(void) {
  if (!body_layer || !scroll) return;
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
        GRect(0, 0, reading_width(bounds.size.w), 16000),
        GTextOverflowModeWordWrap, GTextAlignmentLeft);
    paragraphs[i].y = height;
    paragraphs[i].height = size.h;
    height += size.h;
    if (i + 1 < paragraph_count) height += 8;
  }
  layer_set_frame(body_layer, GRect(0, 0, bounds.size.w, height < viewport_height ? viewport_height : height));
  layer_mark_dirty(body_layer);
  if (height < viewport_height) height = viewport_height;
  scroll_layer_set_content_size(scroll, GSize(bounds.size.w, height));
  max_offset = height - viewport_height;
  // Overlap by more than one line so clipped edge text is readable next page.
  page_step = viewport_height - (font_size ? 36 : 32);
  int offset = previous_max ? previous_offset * max_offset / previous_max : 0;
  move_to(offset == max_offset ? offset : (offset / page_step) * page_step);
}

static void select_adjacent(int delta) {
  selected+=delta;
  if (menu) menu_layer_set_selected_index(menu,(MenuIndex){0,selected+(evening_offer ? 1 : 0)},MenuRowAlignCenter,false);
  load_reading_part(delta>0 ? 0 : day.readings[selected].parts-1);
  if (delta<0) move_to(max_offset);
}
static void page_up(ClickRecognizerRef recognizer, void *context) {
  if (!scroll || page_step<=0) return;
  if (end_card) {end_card=false;move_to(max_offset);return;}
  int offset=-scroll_layer_get_content_offset(scroll).y;
  if (offset==0 && !details_mode) {
    if (reading_part>0) {load_reading_part(reading_part-1);move_to(max_offset);}
    else if (day_valid && selected>0 && selected<day.count) select_adjacent(-1);
  } else move_to(offset>0 ? ((offset-1)/page_step)*page_step : 0);
}
static void page_down(ClickRecognizerRef recognizer, void *context) {
  if (!scroll || page_step<=0) return;
  if (end_card) {if (selected+1<day.count) select_adjacent(1);return;}
  int offset=-scroll_layer_get_content_offset(scroll).y;
  if (offset==max_offset && !details_mode && day_valid && selected<day.count) {
    if (reading_part+1<day.readings[selected].parts) load_reading_part(reading_part+1);
    else {
      end_card=true;move_to(0);
      APP_LOG(APP_LOG_LEVEL_INFO,"End card reading=%d next=%d",selected,selected+1<day.count);
    }
  } else move_to((offset/page_step+1)*page_step);
}
static void jump_top(ClickRecognizerRef recognizer, void *context) {
  end_card=false;
  if (!details_mode && reading_part) load_reading_part(0);
  move_to(0);
}
static void jump_bottom(ClickRecognizerRef recognizer, void *context) {
  end_card=false;
  if (!details_mode && day_valid && selected<day.count && reading_part+1<day.readings[selected].parts)
    load_reading_part(day.readings[selected].parts-1);
  move_to(max_offset);
}
static void open_options(ClickRecognizerRef recognizer, void *context) {
  window_stack_push(options_window,true);
}

static void reader_click_config(void *context) {
  // Own Up/Down explicitly: native ScrollLayer repeats would conflict with jumps.
  window_single_click_subscribe(BUTTON_ID_UP, page_up);
  window_single_click_subscribe(BUTTON_ID_DOWN, page_down);
  window_long_click_subscribe(BUTTON_ID_UP, 700, jump_top, NULL);
  window_long_click_subscribe(BUTTON_ID_DOWN, 700, jump_bottom, NULL);
  window_single_click_subscribe(BUTTON_ID_SELECT, open_options);
}

static int16_t choice_height(MenuLayer *layer, MenuIndex *index, void *context) {
  return font_size ? 60 : 48;
}
static void draw_choice(GContext *ctx,const Layer *cell,const char *label,const char *subtitle,GColor ink,int top) {
  GRect bounds=layer_get_bounds(cell);
  bool highlighted=menu_cell_layer_is_highlighted(cell);
  graphics_context_set_fill_color(ctx,highlighted ? theme.fill : theme.paper);
  graphics_fill_rect(ctx,bounds,0,GCornerNone);
  if (top) {
    graphics_context_set_fill_color(ctx,theme.paper);
    graphics_fill_rect(ctx,GRect(0,0,bounds.size.w,top),0,GCornerNone);
    graphics_context_set_fill_color(ctx,GColorDarkCandyAppleRed);
    graphics_fill_rect(ctx,GRect(8,4,bounds.size.w-16,1),0,GCornerNone);
  }
  graphics_context_set_text_color(ctx,highlighted ? theme.text : theme.body);
  graphics_draw_text(ctx,label,text_font(),GRect(8,top,bounds.size.w-16,font_size ? 32 : 28),
      GTextOverflowModeTrailingEllipsis,GTextAlignmentLeft,NULL);
  graphics_context_set_text_color(ctx,highlighted ? theme.text : ink);
  graphics_draw_text(ctx,subtitle,small_font(),GRect(8,top+(font_size ? 32 : 26),bounds.size.w-16,font_size ? 28 : 22),
      GTextOverflowModeTrailingEllipsis,GTextAlignmentLeft,NULL);
}
static uint16_t settings_count(MenuLayer *layer,uint16_t section,void *context) {return 2;}
static void settings_draw(GContext *ctx,const Layer *cell,MenuIndex *index,void *context) {
  draw_choice(ctx,cell,index->row ? "Theme" : "Text size",
      index->row ? (dark_mode ? "Church (dark)" : "Light") : (font_size ? "Extra Large" : "Large"),theme.ink,0);
}
static void settings_select(MenuLayer *layer,MenuIndex *index,void *context) {
  if (!index->row) {
    font_size=!font_size;
    if (persist_write_int(FONT_SIZE_KEY,font_size)<0) APP_LOG(APP_LOG_LEVEL_ERROR,"Could not save text size");
  } else {
    dark_mode=!dark_mode;
    if (persist_write_bool(DARK_MODE_KEY,dark_mode)<0) APP_LOG(APP_LOG_LEVEL_ERROR,"Could not save theme");
  }
  refresh_theme();layout_menu();layout_reading();menu_layer_reload_data(options_menu);
  APP_LOG(APP_LOG_LEVEL_INFO,"Settings text=%s theme=%s",font_size ? "XL" : "L",dark_mode ? "dark" : "light");
}
static void options_load(Window *window) {
  GRect bounds=layer_get_bounds(window_get_root_layer(window));
  settings_version=text_layer_create(GRect(0,bounds.size.h-24,bounds.size.w,24));
  if (settings_version) {
    text_layer_set_text(settings_version,"v" MISSAL_VERSION);
    text_layer_set_font(settings_version,fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text_alignment(settings_version,GTextAlignmentCenter);
    text_layer_set_background_color(settings_version,GColorClear);
    text_layer_set_text_color(settings_version,dark_mode ? GColorLightGray : GColorDarkGray);
    layer_add_child(window_get_root_layer(window),text_layer_get_layer(settings_version));
  }
  options_menu=menu_layer_create(GRect(0,0,bounds.size.w,bounds.size.h-24));
  if (!options_menu) return;
  menu_layer_set_callbacks(options_menu,NULL,(MenuLayerCallbacks){.get_num_rows=settings_count,
      .get_cell_height=choice_height,.draw_row=settings_draw,.select_click=settings_select});
  menu_layer_set_click_config_onto_window(options_menu,window);
  style_menu(options_menu);layer_add_child(window_get_root_layer(window),menu_layer_get_layer(options_menu));
}
static void options_unload(Window *window) {
  if (options_menu) menu_layer_destroy(options_menu);
  if (settings_version) text_layer_destroy(settings_version);
  settings_version=NULL;
  options_menu=NULL;
}

static void load_reading_part(uint8_t part) {
  end_card=false;
  uint32_t start_ms = now_ms();
  bool ok = true;
  memset(paragraph_rubric, 0, sizeof(paragraph_rubric));
  reading_part = part;
  if (details_mode) {
    snprintf(reading, READING_CAPACITY, "%s\n\n%s\n\nGeneral Roman Calendar\nEpiphany: January 6\nAscension and Corpus Christi: Thursday\nSunday cycle %c; weekday cycle %s\n\nOffline calendar: 2020-2037\nHold Select on the menu to browse dates.",
        date_text, day_details, day_valid ? 'A'+(day.cycles&3) : '?', day_valid && (day.cycles&4) ? "II" : "I");
  } else {
    uint16_t refs[CAL_MAX_REFS], count=0;
    ok = day_valid && selected < day.count && calendar_part(calendar, &day.readings[selected], part, refs, &count) && ensure_database();
    size_t used=0, paragraph=0;
    reading[0]=0;
    if (ok && citations[selected][0]) {
      used=snprintf(reading,READING_CAPACITY,"%s",citations[selected]);
      paragraph_rubric[paragraph++]=true;
    }
    for (uint16_t i=0;ok && i<count;i++) {
      if (paragraph >= ARRAY_LENGTH(paragraph_rubric) || READING_CAPACITY-used <= 2) {ok=false;break;}
      if (used) {reading[used++]='\n';reading[used++]='\n';}
      uint32_t length;
      ok=pmr_segment(&db,refs[i]&0x7fff,reading+used,READING_CAPACITY-used,&length);
      if (ok) {used+=length;paragraph_rubric[paragraph++]=(refs[i]&0x8000)!=0;}
    }
  }
  if (ok) prepare_display_text(reading);
  else snprintf(reading,READING_CAPACITY,"Reading unavailable.\n\nThe bundled reading could not be loaded. Press Back to return.");
  if (!split_paragraphs()) {
    ok=false;
    snprintf(reading,READING_CAPACITY,"Reading unavailable. Too many paragraphs. Press Back to return.");
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
  viewport_height = bounds.size.h;
  max_offset = 0;
  reading=malloc(READING_CAPACITY);
  scroll = scroll_layer_create(GRect(0, 0, bounds.size.w, viewport_height));
  body_layer = layer_create(GRect(0, 0, bounds.size.w, 16000));
  if (!reading || !scroll || !body_layer) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Reader UI allocation failed");
    return;
  }
  layer_set_update_proc(body_layer, draw_body);
  scroll_layer_add_child(scroll, body_layer);
  layer_add_child(window_get_root_layer(window), scroll_layer_get_layer(scroll));
  scroll_layer_set_shadow_hidden(scroll, true);
  unsigned before=heap_bytes_free();
  ribbon_layer=layer_create(GRect(0,0,bounds.size.w,viewport_height));
  if (ribbon_layer) {
    layer_set_update_proc(ribbon_layer,ribbon_draw);
    layer_add_child(window_get_root_layer(window),ribbon_layer);
  }
  APP_LOG(APP_LOG_LEVEL_INFO,"Ribbon layer heap=%u->%u",before,(unsigned)heap_bytes_free());
  load_reading_part(0);ribbon_open();
}

static void reader_unload(Window *window) {
  ribbon_stop();
  if (ribbon_layer) layer_destroy(ribbon_layer);
  ribbon_layer=NULL;
  free(reading);reading=NULL;
  end_card=false;
  if (body_layer) layer_destroy(body_layer);
  if (scroll) scroll_layer_destroy(scroll);
  body_layer = NULL;
  scroll = NULL;
}

static void select_reading(MenuLayer *layer, MenuIndex *index, void *context) {
  if (evening_offer && !index->row) {open_evening();return;}
  selected = index->row-(evening_offer ? 1 : 0);
  if (selected==day.count+1) {open_options(NULL,NULL);return;}
  details_mode = selected >= day.count;
  window_stack_push(reader_window, true);
}

static uint16_t menu_row_count(MenuLayer *layer, uint16_t section, void *context) {
  return day.count + 2 + (evening_offer ? 1 : 0);
}

static int16_t menu_row_height(MenuLayer *layer, MenuIndex *index, void *context) {
  int row=(int)index->row-(evening_offer ? 1 : 0);
  return (font_size ? 60 : 48)+(day.count && row==day.count ? 9 : 0);
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
  int row=(int)index->row-(evening_offer ? 1 : 0);
  const char *label=row<0 ? "Evening Mass" : row<day.count ? menu_titles[day.readings[row].kind] : row==day.count ? "Day details" : "Settings";
  const char *subtitle=row<0 ? "Tomorrow's readings" : row<day.count ? (citations[row][0] ? citations[row] : "Read text") : row==day.count ? "Full calendar details" : settings_summary;
  // Keep the divider in row drawing; the separator callback faults on PT2 4.38.4.
  draw_choice(ctx,cell,label,subtitle,row>=0 && row<day.count ? theme.rubric : theme.ink,
      day.count && row==day.count ? 9 : 0);
}

static size_t read_calendar_resource(void *context, uint32_t offset, uint8_t *out, size_t size) {
  return resource_load_byte_range(calendar_resource, offset, out, size);
}

static void layout_menu(void) {
  if (!menu || !title || !season || !celebration) return;
  GRect bounds=layer_get_bounds(window_get_root_layer(menu_window));
  int line=font_size ? 28 : 24;
  text_layer_set_font(title,small_font());text_layer_set_font(season,small_font());text_layer_set_font(celebration,small_font());
  layer_set_frame(text_layer_get_layer(title),GRect(0,0,bounds.size.w,line));
  layer_set_frame(text_layer_get_layer(season),GRect(4,line,bounds.size.w-8,line));
  layer_set_frame(text_layer_get_layer(celebration),GRect(4,2*line,bounds.size.w-8,line));
  layer_set_hidden(text_layer_get_layer(celebration),!celebration_text[0]);
  int top=line*(celebration_text[0] ? 3 : 2);
  layer_set_frame(menu_layer_get_layer(menu),GRect(0,top,bounds.size.w,bounds.size.h-top));
  menu_layer_reload_data(menu);
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
  layout_menu();
  if (menu) menu_layer_set_selected_index(menu,(MenuIndex){0,0},MenuRowAlignTop,false);
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

static void choose_prompt(void) {
  if (prompt_mode==PROMPT_EVENING) {
    int32_t target=planner_evening(calendar,time(NULL));
    show_date(target>=0 ? target : -1);
  }
  window_stack_pop(false);
}
static void prompt_move(int offset) {
  if (!prompt_scroll || !prompt_footer) return;
  if (offset<0) offset=0;
  if (offset>prompt_max) offset=prompt_max;
  scroll_layer_set_content_offset(prompt_scroll,GPoint(0,-offset),false);
  text_layer_set_text(prompt_footer,offset<prompt_max ? "Down / Select: more" : prompt_mode==PROMPT_EVENING ? "Select: open tomorrow" : "Select: OK");
}
static void prompt_down(ClickRecognizerRef recognizer,void *context) {
  if (prompt_scroll) prompt_move(-scroll_layer_get_content_offset(prompt_scroll).y+150);
}
static void prompt_up(ClickRecognizerRef recognizer,void *context) {
  if (prompt_scroll) prompt_move(-scroll_layer_get_content_offset(prompt_scroll).y-150);
}
static void prompt_select(ClickRecognizerRef recognizer,void *context) {
  if (!prompt_scroll) return;
  if (-scroll_layer_get_content_offset(prompt_scroll).y<prompt_max) prompt_down(NULL,NULL);
  else choose_prompt();
}
static void prompt_click_config(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP,prompt_up);
  window_single_click_subscribe(BUTTON_ID_DOWN,prompt_down);
  window_single_click_subscribe(BUTTON_ID_SELECT,prompt_select);
}
static void prompt_load(Window *window) {
  Layer *root=window_get_root_layer(window);GRect bounds=layer_get_bounds(root);
  int view=bounds.size.h-FOOTER_HEIGHT;
  GSize size=graphics_text_layout_get_content_size(prompt_message,text_font(),GRect(0,0,bounds.size.w-12,1000),GTextOverflowModeWordWrap,GTextAlignmentLeft);
  int height=size.h+10;prompt_max=height>view ? height-view : 0;
  prompt_scroll=scroll_layer_create(GRect(0,0,bounds.size.w,view));
  prompt_text=text_layer_create(GRect(6,0,bounds.size.w-12,height));
  prompt_footer=text_layer_create(GRect(0,view,bounds.size.w,FOOTER_HEIGHT));
  if (!prompt_scroll || !prompt_text || !prompt_footer) return;
  text_layer_set_font(prompt_text,text_font());text_layer_set_text(prompt_text,prompt_message);
  text_layer_set_text_color(prompt_text,theme.body);text_layer_set_background_color(prompt_text,GColorClear);
  scroll_layer_set_content_size(prompt_scroll,GSize(bounds.size.w,height));
  scroll_layer_add_child(prompt_scroll,text_layer_get_layer(prompt_text));
  layer_add_child(root,scroll_layer_get_layer(prompt_scroll));
  text_layer_set_font(prompt_footer,fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(prompt_footer,GTextAlignmentCenter);
  text_layer_set_text_color(prompt_footer,GColorWhite);text_layer_set_background_color(prompt_footer,GColorBlack);
  layer_add_child(root,text_layer_get_layer(prompt_footer));prompt_move(0);
}
static void prompt_unload(Window *window) {
  if (prompt_footer) text_layer_destroy(prompt_footer);
  if (prompt_text) text_layer_destroy(prompt_text);
  if (prompt_scroll) scroll_layer_destroy(prompt_scroll);
  prompt_footer=NULL;prompt_text=NULL;prompt_scroll=NULL;
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
static const char *date_titles[]={"Today","Previous day","Next day","Next Sunday","Next Holy Day","Evening Mass"};
static uint16_t date_count(MenuLayer *layer,uint16_t section,void *context) {
  return planner_evening(calendar,time(NULL))>=0 ? 6 : 5;
}
static void date_draw(GContext *ctx,const Layer *cell,MenuIndex *index,void *context) {
  draw_choice(ctx,cell,date_titles[index->row],index->row==4 ? "Feast or solemnity" : index->row==5 ? "Tomorrow's readings" : "",theme.ink,0);
}
static void date_select(MenuLayer *layer,MenuIndex *index,void *context) {choose_date(index->row,NULL);}
static void date_load(Window *window) {
  date_menu=menu_layer_create(layer_get_bounds(window_get_root_layer(window)));
  if (!date_menu) return;
  menu_layer_set_callbacks(date_menu,NULL,(MenuLayerCallbacks){.get_num_rows=date_count,.get_cell_height=choice_height,.draw_row=date_draw,.select_click=date_select});
  menu_layer_set_click_config_onto_window(date_menu,window);style_menu(date_menu);
  layer_add_child(window_get_root_layer(window),menu_layer_get_layer(date_menu));
}
static void date_unload(Window *window) {if (date_menu) menu_layer_destroy(date_menu);date_menu=NULL;}
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
    text_layer_set_overflow_mode(season,GTextOverflowModeTrailingEllipsis);
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
  dark_mode=persist_exists(DARK_MODE_KEY) && persist_read_bool(DARK_MODE_KEY);
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
    window_set_click_config_provider(prompt_window,prompt_click_config);
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
