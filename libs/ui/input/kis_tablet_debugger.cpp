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

#include "kis_tablet_debugger.h"

#include <QEvent>
#include <QMessageBox>

#include <kis_debug.h>
#include <kis_config.h>

#include <QGlobalStatic>

#include <klocalizedstring.h>

Q_GLOBAL_STATIC(KisTabletDebugger, s_instance)


inline QString button(const QWheelEvent &ev) {
    Q_UNUSED(ev);
    return "-";
}

template <class T>
QString button(const T &ev) {
    return QString::number(ev.button());
}

template <class T>
QString buttons(const T &ev) {
    return QString::number(ev.buttons());
}

template <class Event>
    void dumpBaseParams(QTextStream &s, const Event &ev, const QString &prefix)
{
    s << qSetFieldWidth(5)  << left << prefix << reset << " ";
    s << qSetFieldWidth(17) << left << KisTabletDebugger::exTypeToString(ev.type()) << reset;
}

template <class Event>
    void dumpMouseRelatedParams(QTextStream &s, const Event &ev)
{
    s << "btn: " << button(ev) << " ";
    s << "btns: " << buttons(ev) << " ";
    s << "pos: " << qSetFieldWidth(4) << ev.x() << qSetFieldWidth(0) << "," << qSetFieldWidth(4) << ev.y() << qSetFieldWidth(0) << " ";
    s << "gpos: "  << qSetFieldWidth(4) << ev.globalX() << qSetFieldWidth(0) << "," << qSetFieldWidth(4) << ev.globalY() << qSetFieldWidth(0) << " ";
}

QString KisTabletDebugger::exTypeToString(QEvent::Type type) {
    return
        type == QEvent::TabletEnterProximity ? "TabletEnterProximity" :
        type == QEvent::TabletLeaveProximity ? "TabletLeaveProximity" :
        type == QEvent::Enter ? "Enter" :
        type == QEvent::Leave ? "Leave" :
        type == QEvent::FocusIn ? "FocusIn" :
        type == QEvent::FocusOut ? "FocusOut" :
        type == QEvent::Wheel ? "Wheel" :
        type == QEvent::KeyPress ? "KeyPress" :
        type == QEvent::KeyRelease ? "KeyRelease" :
        type == QEvent::ShortcutOverride ? "ShortcutOverride" :
        type == QMouseEvent::MouseButtonPress ? "MouseButtonPress" :
        type == QMouseEvent::MouseButtonRelease ? "MouseButtonRelease" :
        type == QMouseEvent::MouseButtonDblClick ? "MouseButtonDblClick" :
        type == QMouseEvent::MouseMove ? "MouseMove" :
        type == QTabletEvent::TabletMove ? "TabletMove" :
        type == QTabletEvent::TabletPress ? "TabletPress" :
        type == QTabletEvent::TabletRelease ? "TabletRelease" :
        "unknown";
}


KisTabletDebugger::KisTabletDebugger()
    : m_debugEnabled(false)
{
    KisConfig cfg(true);
    m_shouldEatDriverShortcuts = cfg.shouldEatDriverShortcuts();
}

KisTabletDebugger* KisTabletDebugger::instance()
{
    return s_instance;
}

void KisTabletDebugger::toggleDebugging()
{
    m_debugEnabled = !m_debugEnabled;
    QMessageBox::information(0, i18nc("@title:window", "Krita"), m_debugEnabled ?
                             i18n("Tablet Event Logging Enabled") :
                             i18n("Tablet Event Logging Disabled"));
    if (m_debugEnabled) {
        dbgTablet << "vvvvvvvvvvvvvvvvvvvvvvv START TABLET EVENT LOG vvvvvvvvvvvvvvvvvvvvvvv";
    }
    else {
        dbgTablet << "^^^^^^^^^^^^^^^^^^^^^^^ END TABLET EVENT LOG ^^^^^^^^^^^^^^^^^^^^^^^";
    }
}

bool KisTabletDebugger::debugEnabled() const
{
    return m_debugEnabled;
}

bool KisTabletDebugger::initializationDebugEnabled() const
{
    // FIXME: make configurable!
    return true;
}

bool KisTabletDebugger::debugRawTabletValues() const
{
    // FIXME: make configurable!
    return m_debugEnabled;
}

bool KisTabletDebugger::shouldEatDriverShortcuts() const
{
    return m_shouldEatDriverShortcuts;
}

QString KisTabletDebugger::eventToString(const QMouseEvent &ev, const QString &prefix)
{
    QString string;
    QTextStream s(&string);

    dumpBaseParams(s, ev, prefix);
    dumpMouseRelatedParams(s, ev);
    s << "hires: " << qSetFieldWidth(8) << ev.screenPos().x() << qSetFieldWidth(0) << "," << qSetFieldWidth(8) << ev.screenPos().y() << qSetFieldWidth(0) << " ";
    s << "Source:" << ev.source();

    return string;
}

QString KisTabletDebugger::eventToString(const QKeyEvent &ev, const QString &prefix)
{
    QString string;
    QTextStream s(&string);

    dumpBaseParams(s, ev, prefix);

    s << "key: 0x" << hex << ev.key() << reset << " ";
    s << "mod: 0x" << hex << ev.modifiers() << reset << " ";
    s << "text: " << (ev.text().isEmpty() ? "none" : ev.text());

    return string;
}

QString KisTabletDebugger::eventToString(const QWheelEvent &ev, const QString &prefix)
{
    QString string;
    QTextStream s(&string);

    dumpBaseParams(s, ev, prefix);
    dumpMouseRelatedParams(s, ev);

    s << "delta: " << ev.delta() << " ";
    s << "orientation: " << (ev.orientation() == Qt::Horizontal ? "H" : "V") << " ";

    return string;
}

QString KisTabletDebugger::eventToString(const QEvent &ev, const QString &prefix)
{
    QString string;
    QTextStream s(&string);

    dumpBaseParams(s, ev, prefix);

    return string;
}

template <class Event>
    QString tabletEventToString(const Event &ev, const QString &prefix)
{
    QString string;
    QTextStream s(&string);

    dumpBaseParams(s, ev, prefix);
    dumpMouseRelatedParams(s, ev);

    s << "hires: " << qSetFieldWidth(8) << ev.hiResGlobalX() << qSetFieldWidth(0) << "," << qSetFieldWidth(8) << ev.hiResGlobalY() << qSetFieldWidth(0) << " ";
    s << "prs: " << qSetFieldWidth(4) << fixed << ev.pressure() << reset << " ";

    s << KisTabletDebugger::tabletDeviceToString((QTabletEvent::TabletDevice) ev.device()) << " ";
    s << KisTabletDebugger::pointerTypeToString((QTabletEvent::PointerType) ev.pointerType()) << " ";
    s << "id: " << ev.uniqueId() << " ";

    s << "xTilt: " << ev.xTilt() << " ";
    s << "yTilt: " << ev.yTilt() << " ";
    s << "rot: " << ev.rotation() << " ";
    s << "z: " << ev.z() << " ";
    s << "tp: " << ev.tangentialPressure() << " ";

    return string;
}

QString KisTabletDebugger::eventToString(const QTabletEvent &ev, const QString &prefix)
{
    return tabletEventToString(ev, prefix);
}

QString KisTabletDebugger::tabletDeviceToString(QTabletEvent::TabletDevice device)
{
    return
        device == QTabletEvent::NoDevice ? "NoDevice" :
        device == QTabletEvent::Puck ? "Puck" :
        device == QTabletEvent::Stylus ? "Stylus" :
        device == QTabletEvent::Airbrush ? "Airbrush" :
        device == QTabletEvent::FourDMouse ? "FourDMouse" :
        device == QTabletEvent::XFreeEraser ? "XFreeEraser" :
        device == QTabletEvent::RotationStylus ? "RotationStylus" :
        "unknown";
}

QString KisTabletDebugger::pointerTypeToString(QTabletEvent::PointerType pointer) {
    return
        pointer == QTabletEvent::UnknownPointer ? "UnknownPointer" :
        pointer == QTabletEvent::Pen ? "Pen" :
        pointer == QTabletEvent::Cursor ? "Cursor" :
        pointer == QTabletEvent::Eraser ? "Eraser" :
        "unknown";
}

