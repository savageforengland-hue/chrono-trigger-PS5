/* Downstream PS5 startup diagnostics. No game data is logged. */
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
void egg_ps5_diag_early(void);
void egg_ps5_diag_ready(void);
void egg_ps5_diag_stage(int stage, const char *message);
void egg_ps5_diag_message(const char *message);
void egg_ps5_diag_failure(const char *where, const char *detail);
void egg_ps5_diag_exit(int status);
void egg_ps5_diag_game_thread(void);
const char *egg_ps5_diag_path(void);
#ifdef __cplusplus
}
#endif
