/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "prompts.h"
#include "../runtime/controls.h"
#include <string.h>

static int word(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '_';
}

const char *egg_ps5_prompt_text(const char *text, char *buffer, size_t capacity) {
    size_t used = 0;
    if (!capacity || !text) return text;
    for (size_t i = 0; text[i];) {
        const char *replacement = NULL;
        size_t count = 1;
        int boundary = i == 0 || !word(text[i - 1]);
        if (boundary && text[i + 1] == ':') {
            const char *buttons = "ABXY";
            for (int b = 0; b < 4; ++b)
                if (text[i] == buttons[b]) replacement = controls_pad_label(CTL_A + b);
        }
        if (boundary && !strncmp(text + i, "SELECT", 6) && !word(text[i + 6])) {
            replacement = controls_pad_label(CTL_SELECT); count = 6;
        } else if (boundary && !strncmp(text + i, "START", 5) && !word(text[i + 5])) {
            replacement = controls_pad_label(CTL_START); count = 5;
        }
        size_t length = replacement ? strlen(replacement) : 1;
        if (length >= capacity - used) return text;
        memcpy(buffer + used, replacement ? replacement : text + i, length);
        used += length; i += count;
    }
    buffer[used] = 0;
    return buffer;
}
