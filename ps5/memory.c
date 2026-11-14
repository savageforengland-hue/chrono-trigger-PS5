/* SPDX-License-Identifier: GPL-3.0-or-later
 * Validate the allocator selected by the final link, then report title-heap
 * usage before SDL and whenever startup fails. Do not depend on SDL here.
 */
#include "memory.h"
#include "diagnostics.h"
#include "ps5platform/heap.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void egg_ps5_memory_report(const char *phase) {
    struct ps5_heap_stats stats;
    ps5_heap_stats(&stats);
    char line[384];
    snprintf(line, sizeof line,
             "memory %s: direct-heap base=0x%llx reserved=%llu mapped=%llu peak=%llu arenas=%u segments=%u libc_fallbacks=%llu",
             phase, (unsigned long long)stats.range_base,
             (unsigned long long)stats.range_bytes, (unsigned long long)stats.mapped_bytes,
             (unsigned long long)stats.peak_bytes, stats.arenas, stats.segments, stats.libc_fallbacks);
    egg_ps5_diag_message(line);
}

static bool failed(const char *operation) {
    char line[160];
    snprintf(line, sizeof line, "%s failed; errno=%d", operation, errno);
    egg_ps5_diag_failure("memory preflight", line);
    egg_ps5_memory_report("preflight failed");
    return false;
}

bool egg_ps5_memory_prepare(void) {
    unsigned char *block = malloc(64);
    if (!block) return failed("malloc(64)");
    const bool direct = ps5_heap_owns(block);
    memset(block, 0x5a, 64);
    unsigned char *grown = realloc(block, 4096);
    if (!grown) { int saved = errno; free(block); errno = saved; return failed("realloc(4096)"); }
    bool intact = true;
    for (unsigned i = 0; i < 64; ++i) intact &= grown[i] == 0x5a;
    free(grown);
    if (!intact) {
        egg_ps5_diag_failure("memory preflight", "realloc did not preserve contents");
        return false;
    }
    unsigned char *zero = calloc(32, 16);
    if (!zero) return failed("calloc(32,16)");
    for (unsigned i = 0; i < 512; ++i) intact &= zero[i] == 0;
    free(zero);
    if (!intact) {
        egg_ps5_diag_failure("memory preflight", "calloc did not clear contents");
        return false;
    }
    egg_ps5_diag_stage(35, direct ? "allocator preflight passed: application direct-memory heap" :
                       "allocator preflight passed using libc fallback; direct heap unavailable");
    egg_ps5_memory_report("before SDL_Init");
    return true;
}
