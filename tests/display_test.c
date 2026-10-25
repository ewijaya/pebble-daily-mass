#include "display.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void check(const char *input, bool psalm, const char *expected) {
  char text[8192];
  strcpy(text, input);
  assert(display_prepare(text, sizeof(text), psalm));
  assert(strcmp(text, expected) == 0);
  // Reformatting an already prepared reading must not duplicate markers.
  assert(display_prepare(text, sizeof(text), psalm));
  assert(strcmp(text, expected) == 0);
}

int main(void) {
  check("Psalm 17 (18)\n\n℟. I love you, Lord, my strength.\n\n"
        "I love you, O Lord, my strength, O Lord, my rock.\n"
        "I love you, Lord, my strength.\n\n"
        "The Lord lives and blessed be my rock!\nI love you, Lord, my strength.", true,
        "Psalm 17 (18)\n\nR. I love you, Lord, my strength.\n\n"
        "I love you, O Lord, my strength, O Lord, my rock.\n"
        "R. I love you, Lord, my strength.\n\n"
        "The Lord lives and blessed be my rock!\nR. I love you, Lord, my strength.");
  check("℟. Rejoice!\nOr: Alleluia.\n\nVerse.\nRejoice!\nOr: Alleluia.", true,
        "R. Rejoice!\nOr: Alleluia.\n\nVerse.\nR. Rejoice!\nOr: Alleluia.");
  check("℟. Rejoice!\n\nVerse.\n\nRejoice!\n\n℟. Rejoice!", true,
        "R. Rejoice!\n\nVerse.\n\nR. Rejoice!\n\nR. Rejoice!");
  check("℟. Rejoice!\n\nRejoice! Always.\nWe say Rejoice!\nRejoice!", true,
        "R. Rejoice!\n\nRejoice! Always.\nWe say Rejoice!\nR. Rejoice!");
  check("℟. Rejoice!\n\nVerse.\nRejoice!", false,
        "R. Rejoice!\n\nVerse.\nRejoice!");
  check("Citation\n\nVerse without an explicit response.\n", true,
        "Citation\n\nVerse without an explicit response.\n");
  check("℟. Mercy—forever.\n\nVerse.\nMercy—forever.", true,
        "R. Mercy—forever.\n\nVerse.\nR. Mercy—forever.");
  check("", true, "");
  check("℟. \n\nVerse.", true, "R. \n\nVerse.");

  char exact[14] = "R. Yes\nYes";
  assert(display_prepare(exact, sizeof(exact), true));
  assert(strcmp(exact, "R. Yes\nR. Yes") == 0);
  char short_buffer[13] = "R. Yes\nYes";
  assert(!display_prepare(short_buffer, sizeof(short_buffer), true));
  assert(strcmp(short_buffer, "R. Yes\nYes") == 0);
  char unterminated[] = {'R', '.', ' '};
  assert(!display_prepare(unterminated, sizeof(unterminated), true));
  assert(!display_prepare(unterminated, 0, true));
  puts("Display response markers and buffer boundaries passed");
}
