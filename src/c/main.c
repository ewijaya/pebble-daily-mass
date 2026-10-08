#include <pebble.h>
#include "pmr.h"

static Window *menu_window, *reader_window, *options_window;
static MenuLayer *menu;
static SimpleMenuLayer *options_menu;
static TextLayer *title, *season, *footer;
static Layer *body_layer;
static ScrollLayer *scroll;
static Pmr db;
static ResHandle resource;
// The audited sample readings fit here. Overflow is an error, never truncation.
static char reading[8192];
// Separate drawing boxes allow a real paragraph gap smaller than one text line.
static struct { char *text; int16_t y, height; } paragraphs[64];
static size_t paragraph_count;
static GFont body_font;
static GFont rubric_font;
static bool styled_reading;
static int strip_height;
static int selected;
enum { FONT_SIZE_KEY = 1, FOOTER_HEIGHT = 22 };
static int font_size;
static int viewport_height, max_offset, page_step;
static char footer_text[48];
static uint32_t startup_ms;
static bool first_menu_draw;

static uint32_t now_ms(void) {
  time_t seconds;
  uint16_t milliseconds;
  time_ms(&seconds, &milliseconds);
  return (uint32_t)seconds * 1000 + milliseconds;
}
// Manually verified October 8, 2026 preview (Thursday, Ordinary Week 27, II).
// This is a pinned source selection, not an automatic calendar resolver.
static const uint8_t starts[] = {3, 6, 11, 13};
static const uint8_t counts[] = {3, 5, 2, 5};
static const char *reading_titles[] = {"FIRST READING", "PSALM", "ACCLAMATION", "GOSPEL"};

static bool is_rubric(size_t index) {
  // Roles are audited for this pinned page only. Do not infer scripture roles
  // from arbitrary source text: Psalm responses and Gospel incipits stay black.
  return styled_reading && (index == 0 || (index == 1 && (selected == 0 || selected == 3)));
}

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
    graphics_context_set_fill_color(ctx, GColorDarkCandyAppleRed);
    graphics_fill_rect(ctx, GRect(0, 0, width, strip_height), 0, GCornerNone);
    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_draw_text(ctx, reading_titles[selected], rubric_font,
        GRect(6, 1, width - 12, strip_height),
        GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  }
  for (size_t i = 0; i < paragraph_count; ++i) {
    // Only render paragraphs intersecting the viewport; preserve internal lines.
    if (paragraphs[i].y + paragraphs[i].height < top ||
        paragraphs[i].y > top + viewport_height) continue;
    bool rubric = is_rubric(i);
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
  snprintf(footer_text, sizeof(footer_text), "%d/%d  Select: options", page, pages);
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
    move_to(offset > 0 ? ((offset - 1) / page_step) * page_step : 0);
  }
}

static void page_down(ClickRecognizerRef recognizer, void *context) {
  if (scroll && page_step > 0)
    move_to((-scroll_layer_get_content_offset(scroll).y / page_step + 1) * page_step);
}

static void jump_top(ClickRecognizerRef recognizer, void *context) { move_to(0); }
static void jump_bottom(ClickRecognizerRef recognizer, void *context) { move_to(max_offset); }

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
    move_to(index == 2 ? 0 : max_offset);
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
    menu_layer_set_highlight_colors(simple_menu_layer_get_menu_layer(options_menu), GColorDarkCandyAppleRed, GColorWhite);
    layer_add_child(window_get_root_layer(window), simple_menu_layer_get_layer(options_menu));
    simple_menu_layer_set_selected_index(options_menu, font_size, false);
  }
}

static void options_unload(Window *window) {
  if (options_menu) simple_menu_layer_destroy(options_menu);
  options_menu = NULL;
}

static void reader_load(Window *window) {
  uint32_t start_ms = now_ms();
  uint32_t before = db.inflations;
  bool ok = ensure_database() &&
      pmr_reading(&db, 1549, starts[selected], counts[selected], reading, sizeof(reading));
  if (ok) prepare_display_text(reading);
  if (!ok) snprintf(reading, sizeof(reading), "Reading unavailable.\n\nThe bundled database could not be read. Press Back to return.");
  unsigned text_bytes = strlen(reading);
  if (!split_paragraphs()) {
    ok = false;
    snprintf(reading, sizeof(reading), "Reading unavailable. Too many paragraphs. Press Back to return.");
    split_paragraphs();
  }
  styled_reading = false;
  if (ok && paragraph_count >= 2) {
    char *citation = strchr(paragraphs[0].text, '\n');
    if (citation && citation[1]) {
      // The title moves into the strip; keep the complete source citation.
      paragraphs[0].text = citation + 1;
      styled_reading = true;
    }
  }
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
  layout_reading();
  APP_LOG(APP_LOG_LEVEL_INFO, "Reading %d: ok=%d bytes=%u inflations=%lu heap_free=%u",
          selected, ok, text_bytes,
          (unsigned long)(db.inflations - before), (unsigned)heap_bytes_free());
  APP_LOG(APP_LOG_LEVEL_INFO, "Reading prepared: %lu ms", (unsigned long)(now_ms() - start_ms));
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
  selected = index->row;
  window_stack_push(reader_window, true);
}

static const struct { const char *title, *subtitle; } items[] = {
  {"First Reading", "Galatians 3:1-5"},
  {"Psalm", "Luke 1:69-75"},
  {"Acclamation", "See Acts 16:14b"},
  {"Gospel", "Luke 11:5-13"},
};

static uint16_t menu_row_count(MenuLayer *layer, uint16_t section, void *context) {
  return ARRAY_LENGTH(items);
}

static int16_t menu_row_height(MenuLayer *layer, MenuIndex *index, void *context) {
  return 44; // All four rows fit below the date and season on the PT2.
}

static void main_menu_up(ClickRecognizerRef recognizer, void *context) {
  menu_layer_set_selected_next(menu, true, MenuRowAlignNone, false);
}

static void main_menu_down(ClickRecognizerRef recognizer, void *context) {
  menu_layer_set_selected_next(menu, false, MenuRowAlignNone, false);
}

static void main_menu_select(ClickRecognizerRef recognizer, void *context) {
  MenuIndex index = menu_layer_get_selected_index(menu);
  select_reading(menu, &index, NULL);
}

static void main_menu_click_config(void *context) {
  // This four-row preview fits entirely. Move the highlight without the native
  // scroll margins shifting rows behind the date heading.
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 250, main_menu_up);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 250, main_menu_down);
  window_single_click_subscribe(BUTTON_ID_SELECT, main_menu_select);
}

static void menu_draw_row(GContext *ctx, const Layer *cell, MenuIndex *index, void *context) {
  if (!first_menu_draw) {
    first_menu_draw = true;
    APP_LOG(APP_LOG_LEVEL_INFO, "Startup first menu draw: %lu ms", (unsigned long)(now_ms() - startup_ms));
  }
  GRect bounds = layer_get_bounds(cell);
  bool highlighted = menu_cell_layer_is_highlighted(cell);
  graphics_context_set_fill_color(ctx, highlighted ? GColorDarkCandyAppleRed : GColorWhite);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_text_color(ctx, highlighted ? GColorWhite : GColorBlack);
  graphics_draw_text(ctx, items[index->row].title, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
      GRect(10, 0, bounds.size.w - 20, 24), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
  graphics_context_set_text_color(ctx, highlighted ? GColorWhite : GColorDarkCandyAppleRed);
  graphics_draw_text(ctx, items[index->row].subtitle, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
      GRect(10, 24, bounds.size.w - 20, 20), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
}

static void menu_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);
  title = text_layer_create(GRect(0, 0, bounds.size.w, 24));
  if (title) {
    text_layer_set_font(title, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_background_color(title, GColorDarkCandyAppleRed);
    text_layer_set_text_color(title, GColorWhite);
    text_layer_set_text_alignment(title, GTextAlignmentCenter);
    text_layer_set_text(title, "THU 8 OCT 2026");
    layer_add_child(root, text_layer_get_layer(title));
  }
  season = text_layer_create(GRect(0, 26, bounds.size.w, 24));
  if (season) {
    text_layer_set_font(season, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_color(season, GColorDarkCandyAppleRed);
    text_layer_set_text_alignment(season, GTextAlignmentCenter);
    text_layer_set_text(season, "Ordinary Time - Week 27");
    layer_add_child(root, text_layer_get_layer(season));
  }
  menu = menu_layer_create(GRect(0, 52, bounds.size.w, bounds.size.h - 52));
  if (menu) {
    menu_layer_set_callbacks(menu, NULL, (MenuLayerCallbacks){
        .get_num_rows = menu_row_count, .get_cell_height = menu_row_height,
        .draw_row = menu_draw_row, .select_click = select_reading});
    menu_layer_set_normal_colors(menu, GColorWhite, GColorBlack);
    menu_layer_set_highlight_colors(menu, GColorDarkCandyAppleRed, GColorWhite);
    menu_layer_pad_bottom_enable(menu, false);
    window_set_click_config_provider(window, main_menu_click_config);
    layer_add_child(root, menu_layer_get_layer(menu));
  }
}

static void menu_unload(Window *window) {
  if (menu) menu_layer_destroy(menu);
  if (title) text_layer_destroy(title);
  if (season) text_layer_destroy(season);
  menu = NULL;
  title = season = NULL;
}

int main(void) {
  startup_ms = now_ms();
  font_size = persist_exists(FONT_SIZE_KEY) && persist_read_int(FONT_SIZE_KEY) == 1 ? 1 : 0;
  // Menu data is small and already compiled in. Validate/open the bundled
  // library only when a reading is requested; retain all integrity checks.
  menu_window = window_create();
  reader_window = window_create();
  options_window = window_create();
  if (menu_window && reader_window && options_window) {
    window_set_window_handlers(menu_window, (WindowHandlers){.load = menu_load, .unload = menu_unload});
    window_set_window_handlers(reader_window, (WindowHandlers){.load = reader_load, .unload = reader_unload});
    window_set_window_handlers(options_window, (WindowHandlers){.load = options_load, .unload = options_unload});
    window_set_background_color(reader_window, GColorWhite);
    window_set_click_config_provider(reader_window, reader_click_config);
    window_stack_push(menu_window, true);
    app_event_loop();
  }
  if (options_window) window_destroy(options_window);
  if (reader_window) window_destroy(reader_window);
  if (menu_window) window_destroy(menu_window);
}
