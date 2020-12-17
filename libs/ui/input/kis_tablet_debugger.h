/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    QString eventToString(const QTouchEvent &ev, const QString &prefix);
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
