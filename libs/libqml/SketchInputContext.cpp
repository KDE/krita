/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "SketchInputContext.h"
#include "VirtualKeyboardController.h"

#ifdef Q_OS_WIN
    #include <windows.h>
    #include <QProcess>
#endif

SketchInputContext::SketchInputContext(QObject* parent)
    : QInputContext(parent)
{
}

SketchInputContext::~SketchInputContext()
{

}

bool SketchInputContext::isComposing() const
{
    return true;
}

void SketchInputContext::reset()
{

}

QString SketchInputContext::language()
{
    return QString("en_US");
}

QString SketchInputContext::identifierName()
{
    return QString("SketchInputContext");
}

bool SketchInputContext::filterEvent(const QEvent* event)
{
    if (event->type() == QEvent::RequestSoftwareInputPanel) {
        VirtualKeyboardController::instance()->requestShowKeyboard();
#ifdef Q_OS_WIN
        QProcess::execute("cmd /c \"C:\\Program Files\\Common Files\\Microsoft Shared\\Ink\\Tabtip.exe\"");
#endif
        return true;
    } else if (event->type() == QEvent::CloseSoftwareInputPanel) {
        VirtualKeyboardController::instance()->requestHideKeyboard();
#ifdef Q_OS_WIN
        HWND kbd = ::FindWindow(L"IPTip_Main_Window", NULL);
        ::PostMessage(kbd, WM_SYSCOMMAND, SC_CLOSE, 0);
#endif
        return true;
    }
    return false;
}
