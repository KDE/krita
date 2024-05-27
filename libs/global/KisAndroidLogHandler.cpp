/*
 * SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisAndroidLogHandler.h"

#include <QtGlobal>
#include <QDebug>

#ifndef Q_OS_ANDROID
#error "KisAndroidLogHandler can only built on Android platform"
#endif

#include <config-android-stdio-forwarding.h>
#ifdef ANDROID_ENABLE_STDIO_FORWARDING

#include <android/log.h>
#include <pthread.h>
#include <unistd.h>

namespace KisAndroidLogHandler {

namespace detail {
const char*const applicationName="krita";


/**
 * TODO: theoretically, Qt should forward its debugging lines itself, but for
 *       some undefined reason it doesn't, so let's just force it with a custom
 *       handler.
 */
void kritaQtMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QString report = msg;
    if (context.file && !QString(context.file).isEmpty()) {
        report+=" in file ";
        report+=QString(context.file);
        report+=" line ";
        report+=QString::number(context.line);
    }
    if (context.function && !QString(context.function).isEmpty()) {
        report+=+" function ";
        report+=QString(context.function);
    }

    const QByteArray local8bit = report.toLocal8Bit();
    const char *const local = local8bit.constData();
    switch (type) {
    case QtDebugMsg:
        __android_log_write(ANDROID_LOG_DEBUG, applicationName, local);
        break;
    case QtInfoMsg:
        __android_log_write(ANDROID_LOG_INFO, applicationName, local);
        break;
    case QtWarningMsg:
        __android_log_write(ANDROID_LOG_WARN, applicationName, local);
        break;
    case QtCriticalMsg:
        __android_log_write(ANDROID_LOG_ERROR, applicationName, local);
        break;
    case QtFatalMsg:
    default:
        __android_log_write(ANDROID_LOG_FATAL, applicationName, local);
        abort();    
    }
}

static int pfd[2];
static pthread_t loggingThread;

static void *loggingFunction(void*) {
    ssize_t readSize;
    char buf[128];

    while((readSize = read(pfd[0], buf, sizeof buf - 1)) > 0) {
        if(buf[readSize - 1] == '\n') {
            --readSize;
        }

        buf[readSize] = 0;  // add null-terminator

        __android_log_write(ANDROID_LOG_DEBUG, applicationName, buf); // Set any log level you want
    }

    return 0;
}

static int runStdioForwardingThread() { // run this function to redirect your output to android log
    setvbuf(stdout, 0, _IOLBF, 0); // make stdout line-buffered
    setvbuf(stderr, 0, _IONBF, 0); // make stderr unbuffered

    /* create the pipe and redirect stdout and stderr */
    pipe(pfd);
    dup2(pfd[1], 1);
    dup2(pfd[1], 2);

    /* spawn the logging thread */
    if(pthread_create(&loggingThread, 0, loggingFunction, 0) == -1) {
        return -1;
    }

    pthread_detach(loggingThread);

    return 0;
}

} // detail

void handler_init()
{
    qInstallMessageHandler(&detail::kritaQtMessageHandler);
    detail::runStdioForwardingThread();
}

}

#else /* ANDROID_ENABLE_STDIO_FORWARDING */

namespace KisAndroidLogHandler {
void handler_init()
{
    // do nothing
}

}

#endif /* ANDROID_ENABLE_STDIO_FORWARDING */