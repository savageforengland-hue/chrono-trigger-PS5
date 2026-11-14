#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

EggPs5Config egg_ps5_config = {2160, 60, 3, "default"};

static char *trim(char *s) {
    while (isspace((unsigned char)*s)) ++s;
    char *end = s + strlen(s);
    while (end > s && isspace((unsigned char)end[-1])) *--end = 0;
    return s;
}

void egg_ps5_config_load(void) {
    FILE *f = fopen("/data/eggoftime/ps5.ini", "r");
    if (!f) {
        f = fopen("/data/eggoftime/ps5.ini", "w");
        if (f) {
            fputs("# Output height: 1080, 1440, 2160. Restart after changes.\n"
                  "output_height=2160\ntarget_fps=60\nrender_scale=3\nactive_pack=default\n", f);
            fclose(f);
        }
        return;
    }
    char line[256];
    while (fgets(line, sizeof line, f)) {
        char *key = trim(line), *equals = strchr(key, '=');
        if (*key == '#' || *key == ';' || !equals) continue;
        *equals = 0;
        key = trim(key);
        char *value = trim(equals + 1);
        if (!strcmp(key, "active_pack")) {
            size_t n = strlen(value);
            int valid = n > 0 && n < sizeof egg_ps5_config.active_pack;
            for (size_t i = 0; i < n; ++i)
                if (!isalnum((unsigned char)value[i]) && value[i] != '-' && value[i] != '_') valid = 0;
            if (valid) snprintf(egg_ps5_config.active_pack, sizeof egg_ps5_config.active_pack, "%s", value);
        } else {
            char *end;
            long v = strtol(value, &end, 10);
            if (*end) continue;
            if (!strcmp(key, "output_height") && (v == 1080 || v == 1440 || v == 2160))
                egg_ps5_config.output_height = (int)v;
            else if (!strcmp(key, "target_fps") && (v == 30 || v == 60))
                egg_ps5_config.target_fps = (int)v;
            else if (!strcmp(key, "render_scale") && v >= 1 && v <= 11)
                egg_ps5_config.render_scale = (int)v;
        }
    }
    fclose(f);
}
