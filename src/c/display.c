#include "display.h"
#include <string.h>

bool display_prepare(char *text, size_t capacity, bool psalm) {
  size_t length = 0;
  while (length < capacity && text[length]) ++length;
  if (length == capacity) return false;

  // System Gothic fonts lack U+211F. Keep the following source period.
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
  if (!psalm) return true;
  length = dest - text;

  // The source marks the opening response, then repeats its words without a
  // marker after each stanza (or in a separate paragraph). Match whole lines
  // so verse text containing the same words is not mistaken for a response.
  char *response = text;
  while (strncmp(response, "R. ", 3) != 0) {
    response = strchr(response, '\n');
    if (!response) return true;
    ++response;
  }
  response += 3;
  size_t response_length = strcspn(response, "\n");
  if (!response_length) return true;
  char *first = response + response_length;
  size_t matches = 0;
  for (char *line = first; (line = strchr(line, '\n'));) {
    ++line;
    if (strncmp(line, response, response_length) == 0 &&
        (line[response_length] == '\n' || line[response_length] == 0)) ++matches;
  }
  if (matches > (capacity - length - 1) / 3) return false;
  for (char *line = first; (line = strchr(line, '\n'));) {
    ++line;
    if (strncmp(line, response, response_length) != 0 ||
        (line[response_length] != '\n' && line[response_length] != 0)) continue;
    memmove(line + 3, line, length - (size_t)(line - text) + 1);
    memcpy(line, "R. ", 3);
    length += 3;
    line += 3;
  }
  return true;
}
