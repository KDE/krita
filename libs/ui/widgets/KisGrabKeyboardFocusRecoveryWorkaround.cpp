/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QApplication>
#include <QWidget>
#include <QDebug>

#include "KisGrabKeyboardFocusRecoveryWorkaround.h"

class KisGrabKeyboardFocusRecoveryWorkaround::Private
{
public:

    QScopedPointer<QWidget> dummyFocusRecoveryWidget {nullptr};

    Private()
    {
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
        dummyFocusRecoveryWidget.reset(new QWidget);
        dummyFocusRecoveryWidget->setObjectName("dummyFocusRecoveryWidget");
#endif
    }
};

KisGrabKeyboardFocusRecoveryWorkaround::KisGrabKeyboardFocusRecoveryWorkaround()
    : m_d(new Private)
{}

KisGrabKeyboardFocusRecoveryWorkaround* KisGrabKeyboardFocusRecoveryWorkaround::instance()
{
    static KisGrabKeyboardFocusRecoveryWorkaround *instance_ {nullptr};
    if (!instance_) {
        instance_ = new KisGrabKeyboardFocusRecoveryWorkaround;
    }
    return instance_;
}

void KisGrabKeyboardFocusRecoveryWorkaround::recoverFocus()
{
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    // Get the top-most window
    QWidget *activeWindow = qApp->activeWindow();
    QWidget *mainWindow = activeWindow;
    // It may be that the active window is some dialog (e.g. the color
    // selector). If the dummy widget is set as a child of that one, it may
    // interfere somehow with the task bar (a new entry may appear on some
    // window managers). So we get the parent window and use that as parent of
    // the dummy widget.
    while (mainWindow->parentWidget()) {
        mainWindow = mainWindow->parentWidget();
    }
    // The window flags have to be set as Qt::Dialog so that the workaround
    // works as expected
    m_d->dummyFocusRecoveryWidget->setParent(mainWindow, Qt::Dialog);
    // Perform the following a couple of times to ensure good results. Doing it
    // only once may fail sometimes for some unknown reason
    for (int i = 0; i < 2; ++i) {
        m_d->dummyFocusRecoveryWidget->show();
        m_d->dummyFocusRecoveryWidget->close();
        // Ensure that the previously activated window is again active. If the
        // active window was a dialog (e.g. the color selector), then the parent
        // window of the dummy widget will be the main window, so the main
        // window may be active at this point
        activeWindow->activateWindow();
    }
#endif
}
