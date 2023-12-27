/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TIFF_LOGGER_H
#define KIS_TIFF_LOGGER_H

#include <QString>

#include <cstdio>

#include <kis_debug.h>

QString formatVarArgs(const char *fmt, va_list args)
{
    int size = 4096;
    QByteArray buf(size, 0);
#ifdef _WIN32
    int n = vsnprintf_s(buf.data(), size, size - 1, fmt, args);
#else
    int n = vsnprintf(buf.data(), size, fmt, args);
#endif
    while (n >= size || buf.at(size - 2)) {
        size *= 2;
        buf.resize(size);
        buf[size - 1] = 0;
        buf[size - 2] = 0;
#ifdef _WIN32
        n = vsnprintf_s(buf.data(), size, size - 1, fmt, args);
#else
        n = vsnprintf(buf.data(), size, fmt, args);
#endif
    }

    if (n) {
        return {buf};
    } else {
        return {};
    }
}

void KisTiffErrorHandler(const char *module, const char *fmt, va_list args)
{
    QString msg("%1: %2");

    errFile << msg.arg(module, formatVarArgs(fmt, args));
}

void KisTiffWarningHandler(const char *module, const char *fmt, va_list args)
{
    QString msg("%1: %2");

    warnFile << msg.arg(module, formatVarArgs(fmt, args));
}

#endif
