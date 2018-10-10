/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_TABLET_DEBUGGER_H
#define __KIS_TABLET_DEBUGGER_H

#include <QTextStream>
#include <QTabletEvent>
#include <QLoggingCategory>

class KisTabletDebugger
{
public:
    KisTabletDebugger();
    static KisTabletDebugger* instance();

    void toggleDebugging();
    bool debugEnabled() const;
    bool initializationDebugEnabled() const;
    bool debugRawTabletValues() const;

    bool shouldEatDriverShortcuts() const;

    QString eventToString(const QMouseEvent &ev, const QString &prefix);
    QString eventToString(const QKeyEvent &ev, const QString &prefix);
    QString eventToString(const QWheelEvent &ev, const QString &prefix);
    QString eventToString(const QTabletEvent &ev, const QString &prefix);
    QString eventToString(const QEvent &ev, const QString &prefix);

    static QString tabletDeviceToString(QTabletEvent::TabletDevice device);
    static QString pointerTypeToString(QTabletEvent::PointerType pointer);
    static QString exTypeToString(QEvent::Type type);


private:
    bool m_debugEnabled;
    bool m_shouldEatDriverShortcuts;
};

#endif /* __KIS_TABLET_DEBUGGER_H */
