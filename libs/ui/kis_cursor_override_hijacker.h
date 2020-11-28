/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_CURSOR_OVERRIDE_HIJACKER_H
#define __KIS_CURSOR_OVERRIDE_HIJACKER_H

#include <QCursor>
#include <QStack>
#include <QApplication>


/**
 * @brief The KisCursorOverrideHijacker class stores all
 * override cursors in a stack, and resets them back after
 * the object is deleted. This is useful when you need to
 * show a dialog when a busy cursor is shown.
 */
class KisCursorOverrideHijacker
{
public:
    KisCursorOverrideHijacker() {
        while (qApp->overrideCursor()) {
            m_cursorStack.push(*qApp->overrideCursor());
            qApp->restoreOverrideCursor();
        }
    }

    ~KisCursorOverrideHijacker() {
        while (!m_cursorStack.isEmpty()) {
            qApp->setOverrideCursor(m_cursorStack.pop());
        }
    }

private:
    QStack<QCursor> m_cursorStack;
};

#endif /* __KIS_CURSOR_OVERRIDE_HIJACKER_H */
