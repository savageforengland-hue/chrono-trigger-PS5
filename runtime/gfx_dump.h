/* Graphics dump (gfx_dump.c): every sprite sheet, sprite frame and background of the ROM as PNG files in
 * graphics-export, written by a thread of its own while the game runs. */
#pragma once

void gfxdump_start(void); /* does nothing while a dump runs */
int gfxdump_running(void);
/* Files done and to do (0, 0 while the ROM's files are being listed), and the share of the work done (0-1, by
 * the files' sizes: the pace to time the rest by); any thread. */
void gfxdump_progress(int *done, int *total, float *share);
/* After a dump: the images written (and 1), once; else 0. Host thread. */
int gfxdump_finished(int *images);

/* Inspect after gfxdump_finished consumes the completion notification. */
int gfxdump_errors(void);
const char *gfxdump_error(void);

/* Explicit legacy repair, never silently skipped. */
void gfxdump_repair_start(void);
int gfxdump_repairing(void);

/* Startup: synchronous one-sprite encode/decode check, before the game thread. */
void gfxdump_check_start(void);
int gfxdump_checking(void);
const char *gfxdump_output_path(void);
