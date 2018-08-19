/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
