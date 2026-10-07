#include <pebble.h>
static Window *window;
static TextLayer *label;
static char status[160];
static void load(Window *w) {
  ResHandle db = resource_get_handle(RESOURCE_ID_READINGS_DB);
  uint8_t header[16];
  size_t read = resource_load_byte_range(db, 0, header, sizeof(header));
  bool valid = read == sizeof(header) && memcmp(header, "PMR1", 4) == 0;
  snprintf(status, sizeof(status), "Daily Mass\nStorage probe\n\nDatabase: %lu bytes\nHeader: %s\n\nReader not implemented", (unsigned long)resource_size(db), valid ? "OK" : "FAILED");
  label = text_layer_create(GRect(8, 12, 184, 204));
  text_layer_set_font(label, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text(label, status);
  layer_add_child(window_get_root_layer(w), text_layer_get_layer(label));
}
static void unload(Window *w) { text_layer_destroy(label); }
int main(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers){.load = load, .unload = unload});
  window_stack_push(window, true);
  app_event_loop();
  window_destroy(window);
}
