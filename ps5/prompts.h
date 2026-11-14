/* Runtime-only controller prompt adaptation; original ROM assets are untouched. */
#pragma once
#include <stddef.h>
const char *egg_ps5_prompt_text(const char *text, char *buffer, size_t capacity);
