/*
 * Copyright (C) 2008-2009 Hyves (Startphone Ltd.)
 * Copyright (c) 2014 Boudewijn Rempt <boud@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  US
 *
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Breakpad integration
 *
 * The Initial Developer of the Original Code is
 * Ted Mielczarek <ted.mielczarek@gmail.com>
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Josh Aas <josh@mozilla.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "kis_crash_handler.h"

#include <QDebug>
#include <QDesktopServices>
#include <QTextStream>

#ifdef Q_WS_MAC
#include <string>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <client/mac/handler/exception_handler.h>
#elif defined Q_OS_MAC
#include <string.h>
#include <windows.h>
#include <iostream>
#include <client/windows/handler/exception_handler.h>
#elif defined HAVE_X11
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <client/linux/handler/exception_handler.h>
#else
#error "Breakpad does not exist for this platform!"
#endif

#include <stdlib.h>
#include <time.h>

#ifdef Q_OS_MAC
#ifndef UNICODE
#define UNICODE
#endif
typedef wchar_t HD_CHAR;
// sort of arbitrary, but MAX_PATH is kinda small
#define HD_MAX_PATH 4096
#define CRASH_REPORTER_BINARY L"crashhandler.exe"
#else
typedef char HD_CHAR;
#define CRASH_REPORTER_BINARY "crashhandler"
#endif

static google_breakpad::ExceptionHandler *exceptionHandler = 0;

// if this is true, we pass the exception on to the OS crash reporter
// static bool showOSCrashReporter = false;

// Note: we've crashed so we cannot use the heap here. So we cannot use any Q* class.
static bool startCrashReporter(const HD_CHAR *dumpPath, const HD_CHAR *minidumpID, void *context,
                               #ifdef Q_OS_MAC
                               EXCEPTION_POINTERS *exceptionInfo,
                               MDRawAssertionInfo *assertion,
                               #endif
                               bool succeeded) {

    if (!succeeded) {
        return false;
    }

#ifdef Q_OS_MAC

    wchar_t command[(HD_MAX_PATH * 2) + 6];
    wcscpy(command, CRASH_REPORTER_BINARY L" \"");
    wcscat(command, dumpPath);
    wcscat(command, L"\" \"");
    wcscat(command, minidumpID);
    wcscat(command, L"\"");

    STARTUPINFOW si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOWNORMAL;
    ZeroMemory(&pi, sizeof(pi));

    if (CreateProcessW(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle( pi.hProcess );
        CloseHandle( pi.hThread );
        TerminateProcess(GetCurrentProcess(), 1);
    }

    return false;
#else
    pid_t pid = fork();

    if (pid == -1) {
        return false;
    } else if (pid == 0) {
        execl(CRASH_REPORTER_BINARY, CRASH_REPORTER_BINARY, dumpPath, minidumpID, (char*)0);
    }
#ifdef HAVE_X11
    abort();
#endif
    return true;
#endif
}

// Copied from Qt's QString class because of weird linking errors
int toWCharArray(const QString str, wchar_t *array)
{
    if (sizeof(wchar_t) == sizeof(QChar)) {
        memcpy(array, str.utf16(), sizeof(wchar_t)*str.length());
        return str.length();
    } else {
        wchar_t *a = array;
        const unsigned short *uc = str.utf16();
        for (int i = 0; i < str.length(); ++i) {
            uint u = uc[i];
            if (u >= 0xd800 && u < 0xdc00 && i < str.length()-1) {
                ushort low = uc[i+1];
                if (low >= 0xdc00 && low < 0xe000) {
                    ++i;
                    u = (u - 0xd800)*0x400 + (low - 0xdc00) + 0x10000;
                }
            }
            *a = wchar_t(u);
            ++a;
        }
        return a - array;
    }
}

KisCrashHandler::KisCrashHandler()
{

    QString tempPath = QDesktopServices::storageLocation(QDesktopServices::TempLocation);

#ifdef Q_OS_MAC
    qDebug() << "Installing CrashHandler" << tempPath;
    typedef std::basic_string<wchar_t> wstring;
    wstring str;
    str.resize(tempPath.length());
    str.resize(toWCharArray(tempPath, &(*str.begin())));
    exceptionHandler = new google_breakpad::ExceptionHandler(str, 0,
                                                             startCrashReporter, 0,
                                                             google_breakpad::ExceptionHandler::HANDLER_ALL);

#else
    qDebug() << "Installing CrashHandler"; // do not remove this line; it is needed to make it work on linux.

    exceptionHandler = new google_breakpad::ExceptionHandler(tempPath.toStdString(), 0,
                                                             startCrashReporter, 0,
                                                             true);
#endif
    Q_CHECK_PTR(exceptionHandler);
}

KisCrashHandler::~KisCrashHandler()
{
    delete exceptionHandler;
}
