/*
 * SPDX-FileCopyrightText: 2022 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisAndroidCrashHandler.h"

#include <QDateTime>
#include <QMap>
#include <QScopedPointer>
#include <QStandardPaths>
#include <QThread>
#include <android/log.h>
#include <array>
#include <fcntl.h>
#include <signal.h>
#include <sstream>
#include <unistd.h>
#include <unwindstack/Regs.h>
#include <unwindstack/Unwinder.h>

#define CRASH_LOGGER(...) __android_log_print(ANDROID_LOG_WARN, "KisAndroidCrashHandler", __VA_ARGS__)

namespace KisAndroidCrashHandler {

static const std::array<int, 6> signals = {SIGABRT, SIGBUS, SIGFPE, SIGSEGV, SIGSYS, SIGTERM};
static QMap<int, struct sigaction> g_old_actions;

// we need to have keep this object alive
static const std::string path =
    QString(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/kritacrashlog.txt").toStdString();
static const char *crashlog_path = path.c_str();

static bool g_handling_crash = false;

const char *get_signal_name(const int signo)
{
    switch (signo) {
    case SIGABRT:
        return "SIGABRT";
    case SIGBUS:
        return "SIGBUS";
    case SIGFPE:
        return "SIGFPE";
    case SIGSEGV:
        return "SIGSEGV";
    case SIGSYS:
        return "SIGSYS";
    case SIGTERM:
        return "SIGTERM";
    default:
        return "?";
    }
}

void dump_backtrace(siginfo_t *info, void *ucontext)
{
    QScopedPointer<unwindstack::Regs> regs;
    if (ucontext) {
        regs.reset(unwindstack::Regs::CreateFromUcontext(unwindstack::Regs::CurrentArch(), ucontext));
    } else {
        regs.reset(unwindstack::Regs::CreateFromLocal());
    }

    unwindstack::UnwinderFromPid unwinder(256, getpid(), unwindstack::Regs::CurrentArch());
    if (!unwinder.Init()) {
        CRASH_LOGGER("Couldn't initialize the unwinder: %s\n", unwinder.LastErrorCodeString());
        return;
    }

    unwinder.SetRegs(regs.data());
    unwinder.Unwind();

    std::vector<unwindstack::FrameData> frames = unwinder.frames();
    if (frames.size() == 0) {
        CRASH_LOGGER("Couldn't unwind: %s\t code = %d\n", unwinder.LastErrorCodeString(), unwinder.LastErrorCode());
        return;
    }

    const int fd = open(crashlog_path, O_CREAT | O_APPEND | O_WRONLY, S_IRUSR | S_IWUSR);

    std::stringstream header;
    header << "********************** Dumping backtrace **********************\n"
           << "Signal: " << info->si_signo << " (" << get_signal_name(info->si_signo) << ")"
           << " (Code: " << info->si_code << ")"
           << " Time: " << QDateTime::currentDateTimeUtc().toString().toStdString().c_str()
           << "\n";
    write(fd, header.str().c_str(), header.str().size());

    for (size_t i = 0; i < frames.size(); ++i) {
        std::string frame = unwinder.FormatFrame(frames[i]) + "\n";
        write(fd, frame.c_str(), frame.size());
    }
    write(fd, "\n", 1);
    close(fd);
}

void crash_callback(int sig, siginfo_t *info, void *ucontext)
{
    // to prevent second invocation of our signal handler
    if (g_handling_crash) {
        // uninstall the handler for the signal
        sigaction(sig, &g_old_actions[sig], nullptr);
        raise(sig);
        return;
    }

    g_handling_crash = true;
    dump_backtrace(info, ucontext);

    // uninstall the handler for the signal
    sigaction(sig, &g_old_actions[sig], nullptr);

    // invoke the previous handler
    // Some other implementations tend to make call to handler functions
    // directly, seemingly to not make another _slow_ syscall.
    raise(sig);
}

void handler_init()
{
    // create an alternate stack to make sure we can handle overflows
    stack_t alternate_stack;
    alternate_stack.ss_flags = 0;
    alternate_stack.ss_size = SIGSTKSZ;
    if ((alternate_stack.ss_sp = malloc(SIGSTKSZ)) == nullptr) {
        CRASH_LOGGER("Couldn't allocate memory for alternate stack");
        return;
    }

    struct sigaction act = {};
    act.sa_sigaction = crash_callback;
    act.sa_flags = SA_SIGINFO | SA_ONSTACK;
    sigaltstack(&alternate_stack, nullptr);

    for (size_t i = 0; i < signals.size(); ++i) {
        sigaction(signals[i], &act, &g_old_actions[signals[i]]);
    }
}

} // namespace KisAndroidCrashHandler
