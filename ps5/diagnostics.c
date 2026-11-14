/* Downstream PS5 diagnostics. Compile with -fno-builtin: the entry-point
 * path must not call libc, SDL, a heap allocator, or TLS before _init_env.
 * Fatal-signal handling uses fixed stack buffers and kernel writes. It preserves
 * the default fatal action rather than hiding the crash from system diagnostics.
 */
#include "diagnostics.h"
#include <fcntl.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <ucontext.h>
#include <unistd.h>

extern int sceKernelOpen(const char *, int, unsigned);
extern int sceKernelClose(int);
extern int sceKernelMkdir(const char *, unsigned);
extern ssize_t sceKernelWrite(int, const void *, size_t);
extern int sceKernelFsync(int);
extern int sceKernelSendNotificationRequest(uint32_t, void *, size_t, uint32_t);
extern int sceKernelDebugOutText(int, const char *);

static int log_fd = -1;
static const char *log_path;
static volatile sig_atomic_t current_stage;
static volatile sig_atomic_t handling_signal;
static volatile sig_atomic_t c_runtime_ready;
static unsigned char main_signal_stack[65536] __attribute__((aligned(16)));
static unsigned char game_signal_stack[65536] __attribute__((aligned(16)));
static const char *const paths[] = {
    "/data/eggoftime/eggoftime.log",
    "/data/eggoftime-startup.log",
    "/download0/eggoftime.log",
    "/temp0/eggoftime.log"
};
static int open_results[sizeof paths / sizeof paths[0]];

typedef struct Line { char text[2048]; size_t used; } Line;
static void append(Line *line, const char *text) {
    if (!text) text = "(null)";
    while (*text && line->used < sizeof line->text - 2)
        line->text[line->used++] = *text++;
}
static void number(Line *line, uint64_t value, unsigned base) {
    char reverse[32]; unsigned count = 0;
    do { reverse[count++] = "0123456789abcdef"[value % base]; value /= base; } while (value && count < sizeof reverse);
    while (count && line->used < sizeof line->text - 2) line->text[line->used++] = reverse[--count];
}
static void write_bytes(int fd, const char *data, size_t count) {
    /* Kernel writes can be partial. Bound retries so a signal handler cannot hang. */
    for (unsigned tries = 0; count && tries < 2048; ++tries) {
        ssize_t done = sceKernelWrite(fd, data, count);
        if (done <= 0) break;
        data += done; count -= (size_t)done;
    }
}
static void emit(Line *line, int durable) {
    line->text[line->used++] = '\n';
    line->text[line->used] = 0;
    write_bytes(log_fd >= 0 ? log_fd : STDERR_FILENO, line->text, line->used);
    /* Mirror startup checkpoints to the same kernel log used by ApricotGS.
     * This path works even when every file destination fails. Keep the fatal
     * handler limited to its existing fixed-buffer file/descriptor writes. */
    if (c_runtime_ready && !handling_signal && (durable || log_fd < 0)) {
        Line record; record.used = 0;
        append(&record, "[CTPS5 RC3] "); append(&record, line->text);
        record.text[record.used] = 0;
        (void)sceKernelDebugOutText(0, record.text);
    }
    if (durable && log_fd >= 0) (void)sceKernelFsync(log_fd);
}
const char *egg_ps5_diag_path(void) { return log_path; }
void egg_ps5_diag_message(const char *message) {
    Line line; line.used = 0;
    append(&line, message); emit(&line, 0);
}
void egg_ps5_diag_stage(int stage, const char *message) {
    current_stage = stage;
    Line line; line.used = 0;
    append(&line, "stage="); number(&line, (unsigned)stage, 10);
    append(&line, " "); append(&line, message); emit(&line, 1);
}
void egg_ps5_diag_early(void) {
    /* Executed before even dereferencing the loader's process-parameter block. */
    for (unsigned i = 0; i < sizeof paths / sizeof paths[0]; ++i) {
        open_results[i] = sceKernelOpen(paths[i], O_WRONLY | O_CREAT | O_APPEND, 0666);
        if (open_results[i] >= 0) { log_fd = open_results[i]; log_path = paths[i]; break; }
    }
    egg_ps5_diag_stage(1, "CTPS5 0.1.0-rc3 entry reached");
    Line anchor; anchor.used = 0;
    append(&anchor, "image_anchor egg_ps5_diag_early=0x");
    number(&anchor, (uintptr_t)&egg_ps5_diag_early, 16); emit(&anchor, 1);
    for (unsigned i = 0; i < sizeof paths / sizeof paths[0]; ++i) {
        Line line; line.used = 0;
        append(&line, "log-open "); append(&line, paths[i]);
        append(&line, " result=0x"); number(&line, (uint32_t)open_results[i], 16);
        emit(&line, 1);
        if (open_results[i] >= 0) break;
    }
}

static void notify(const char *prefix, const char *detail) {
    /* Public native boilerplate's notification request layout. Called only
     * after C runtime initialization, never from a fatal-signal handler. */
    struct { unsigned char reserved[45]; char message[3075]; } request = {0};
    Line line; line.used = 0;
    append(&line, prefix); append(&line, detail);
    for (size_t i = 0; i < line.used; ++i) request.message[i] = line.text[i];
    (void)sceKernelSendNotificationRequest(0, &request, sizeof request, 0);
}

static void fatal_signal(int signal_number, siginfo_t *info, void *context) {
    if (handling_signal) _exit(128 + signal_number);
    handling_signal = 1;
    Line line; line.used = 0;
    append(&line, "FATAL signal="); number(&line, (unsigned)signal_number, 10);
    append(&line, " last_stage="); number(&line, (unsigned)current_stage, 10);
    if (info) {
        append(&line, " si_code=0x"); number(&line, (uint32_t)info->si_code, 16);
        append(&line, " fault_address=0x"); number(&line, (uintptr_t)info->si_addr, 16);
    }
    if (context) {
        ucontext_t *uc = context;
#ifdef __linux__
        uintptr_t pc = uc->uc_mcontext.gregs[REG_RIP];
        uintptr_t sp = uc->uc_mcontext.gregs[REG_RSP];
#else
        uintptr_t pc = uc->uc_mcontext.mc_rip;
        uintptr_t sp = uc->uc_mcontext.mc_rsp;
#endif
        append(&line, " native_pc=0x"); number(&line, pc, 16);
        append(&line, " native_sp=0x"); number(&line, sp, 16);
    }
    emit(&line, 1);
    /* SA_RESETHAND has restored the system's default action. Re-deliver the
     * signal; the system can retain its normal crash reporting/core behavior. */
    if (kill(getpid(), signal_number) != 0) _exit(128 + signal_number);
}

static void set_signal_stack(unsigned char *buffer, size_t size) {
    stack_t stack = {0};
    stack.ss_sp = buffer; stack.ss_size = size;
    if (sigaltstack(&stack, NULL) != 0) egg_ps5_diag_message("warning: signal alternate stack unavailable");
}
void egg_ps5_diag_game_thread(void) {
    set_signal_stack(game_signal_stack, sizeof game_signal_stack);
    egg_ps5_diag_message("game thread entered");
}
static void redirect_stream(FILE **destination, const char *name) {
    /* A native title can be denied dup2. Give each FILE its own newly opened
     * descriptor and assign the C stream, as ApricotGS's pinned SDK does. */
    int fd = sceKernelOpen(log_path, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd < 0) { egg_ps5_diag_message(name); return; }
    FILE *stream = fdopen(fd, "a");
    if (!stream) { (void)sceKernelClose(fd); egg_ps5_diag_message(name); return; }
    (void)setvbuf(stream, NULL, _IONBF, 0);
    *destination = stream;
}
void egg_ps5_diag_ready(void) {
    c_runtime_ready = 1;
    set_signal_stack(main_signal_stack, sizeof main_signal_stack);
    struct sigaction action = {0};
    action.sa_sigaction = fatal_signal;
    action.sa_flags = SA_SIGINFO | SA_ONSTACK | SA_RESETHAND;
    (void)sigemptyset(&action.sa_mask);
    const int signals[] = {SIGSEGV, SIGBUS, SIGILL, SIGFPE, SIGABRT};
    for (unsigned i = 0; i < sizeof signals / sizeof signals[0]; ++i)
        if (sigaction(signals[i], &action, NULL) != 0)
            egg_ps5_diag_message("warning: a fatal-signal handler could not be installed");
    if (log_fd >= 0) {
        redirect_stream(&stdout, "warning: stdout stream redirection failed");
        redirect_stream(&stderr, "warning: stderr stream redirection failed");
        /* Normal launches keep their startup path in the file log. */
    } else {
        notify("CTPS5: file logging unavailable. ", "Startup errors will also appear in notifications.");
    }
    egg_ps5_diag_stage(20, "C runtime ready; fatal-signal handlers installed where supported");
}
void egg_ps5_diag_failure(const char *where, const char *detail) {
    Line line; line.used = 0;
    append(&line, "STARTUP FAILURE "); append(&line, where);
    append(&line, ": "); append(&line, detail); emit(&line, 1);
    /* The file path belongs to the title's namespace and might not be exposed
     * by its FTP service. Include the actual failure detail in a notification
     * so diagnosis does not require reading that file. Keep the prefix short.
     */
    Line visible; visible.used = 0;
    append(&visible, where); append(&visible, ": "); append(&visible, detail);
    visible.text[visible.used] = 0;
    notify("CTPS5 RC3 ", visible.text);
}
void egg_ps5_diag_exit(int status) {
    Line line; line.used = 0;
    append(&line, "main returned status=");
    if (status < 0) { append(&line, "-"); number(&line, (uint64_t)-(int64_t)status, 10); }
    else number(&line, (unsigned)status, 10);
    emit(&line, 1);
}
