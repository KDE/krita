/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_debug.h"

#include "config-debug.h"

#if HAVE_BACKTRACE
#include <execinfo.h>
#ifdef __GNUC__
#define HAVE_BACKTRACE_DEMANGLE
#include <cxxabi.h>
#endif
#endif

#include <string>


#if HAVE_BACKTRACE
static QString maybeDemangledName(char *name)
{
#ifdef HAVE_BACKTRACE_DEMANGLE
    const int len = strlen(name);
    QByteArray in = QByteArray::fromRawData(name, len);
    const int mangledNameStart = in.indexOf("(_");
    if (mangledNameStart >= 0) {
        const int mangledNameEnd = in.indexOf('+', mangledNameStart + 2);
        if (mangledNameEnd >= 0) {
            int status;
            // if we forget about this line and the one that undoes its effect we don't change the
            // internal data of the QByteArray::fromRawData() ;)
            name[mangledNameEnd] = 0;
            char *demangled = abi::__cxa_demangle(name + mangledNameStart + 1, 0, 0, &status);
            name[mangledNameEnd] = '+';
            if (demangled) {
                QString ret = QString::fromLatin1(name, mangledNameStart + 1) +
                              QString::fromLatin1(demangled) +
                              QString::fromLatin1(name + mangledNameEnd, len - mangledNameEnd);
                free(demangled);
                return ret;
            }
        }
    }
#endif
    return QString::fromLatin1(name);
}
#endif

QString kisBacktrace()
{
    QString s;
#if HAVE_BACKTRACE
    void *trace[256];
    int n = backtrace(trace, 256);
    if (!n) {
        return s;
    }
    char **strings = backtrace_symbols(trace, n);

    s = QLatin1String("[\n");

    for (int i = 0; i < n; ++i)
        s += QLatin1String("\t") + QString::number(i) + QLatin1String(": ") +
             maybeDemangledName(strings[i]) + QLatin1Char('\n');
    s += QLatin1String("]\n");
    free(strings);
#endif
    return s;
}

Q_LOGGING_CATEGORY(_30009, "krita.lib.resources", QtInfoMsg)
Q_LOGGING_CATEGORY(_41000, "krita.general", QtInfoMsg)
Q_LOGGING_CATEGORY(_41001, "krita.core", QtInfoMsg)
Q_LOGGING_CATEGORY(_41002, "krita.registry", QtInfoMsg)
Q_LOGGING_CATEGORY(_41003, "krita.tools", QtInfoMsg)
Q_LOGGING_CATEGORY(_41004, "krita.tiles", QtInfoMsg)
Q_LOGGING_CATEGORY(_41005, "krita.filters", QtInfoMsg)
Q_LOGGING_CATEGORY(_41006, "krita.plugins", QtInfoMsg)
Q_LOGGING_CATEGORY(_41007, "krita.ui", QtInfoMsg)
Q_LOGGING_CATEGORY(_41008, "krita.file", QtInfoMsg)
Q_LOGGING_CATEGORY(_41009, "krita.math", QtInfoMsg)
Q_LOGGING_CATEGORY(_41010, "krita.render", QtInfoMsg)
Q_LOGGING_CATEGORY(_41011, "krita.scripting", QtInfoMsg)
Q_LOGGING_CATEGORY(_41012, "krita.input", QtInfoMsg)
Q_LOGGING_CATEGORY(_41013, "krita.action", QtInfoMsg)
Q_LOGGING_CATEGORY(_41014, "krita.tabletlog", QtDebugMsg)
Q_LOGGING_CATEGORY(_41015, "krita.opengl", QtInfoMsg)
Q_LOGGING_CATEGORY(_41016, "krita.metadata", QtInfoMsg)
Q_LOGGING_CATEGORY(_41017, "krita.android", QtDebugMsg)
Q_LOGGING_CATEGORY(_41018, "krita.locale", QtInfoMsg)

QString __methodName(const char *_prettyFunction)
{
    std::string prettyFunction(_prettyFunction);

    size_t colons = prettyFunction.find("::");
    size_t begin = prettyFunction.substr(0,colons).rfind(" ") + 1;
    size_t end = prettyFunction.rfind("(") - begin;

    return QString(std::string(prettyFunction.substr(begin,end) + "()").c_str());
}

void printBacktrace()
{
    qDebug().noquote() << kisBacktrace();
}
