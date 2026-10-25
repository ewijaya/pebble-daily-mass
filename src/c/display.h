#pragma once
#include <stdbool.h>
#include <stddef.h>

// Prepare a NUL-terminated reading in place. False means insufficient capacity.
bool display_prepare(char *text, size_t capacity, bool psalm);
