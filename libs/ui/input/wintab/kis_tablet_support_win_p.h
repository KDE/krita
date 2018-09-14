/*
 *  Copyright (C) 2015 The Qt Company Ltd.
 *  Contact: http://www.qt.io/licensing/
 *  Copyright (C) 2015 Michael Abrahms <miabraha@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
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

#ifndef KIS_TABLET_SUPPORT_WIN_P_H
#define KIS_TABLET_SUPPORT_WIN_P_H


#include <QtCore/QVector>
#include <QtCore/QPointF>
#include <QMap>
#include <QRect>

#include "wintab.h"

QT_BEGIN_NAMESPACE

class QDebug;
class QWindow;
class QRect;
class QWidget;

struct QWindowsWinTab32DLL
{
    QWindowsWinTab32DLL() : wTOpen(0), wTClose(0), wTInfo(0), wTEnable(0), wTOverlap(0), wTPacketsGet(0), wTGet(0),
        wTQueueSizeGet(0), wTQueueSizeSet(0) {}

    bool init();

    typedef HCTX (API *PtrWTOpen)(HWND, LPLOGCONTEXT, BOOL);
    typedef BOOL (API *PtrWTClose)(HCTX);
    typedef UINT (API *PtrWTInfo)(UINT, UINT, LPVOID);
    typedef BOOL (API *PtrWTEnable)(HCTX, BOOL);
    typedef BOOL (API *PtrWTOverlap)(HCTX, BOOL);
    typedef int  (API *PtrWTPacketsGet)(HCTX, int, LPVOID);
    typedef BOOL (API *PtrWTGet)(HCTX, LPLOGCONTEXT);
    typedef int  (API *PtrWTQueueSizeGet)(HCTX);
    typedef BOOL (API *PtrWTQueueSizeSet)(HCTX, int);

    PtrWTOpen wTOpen;
    PtrWTClose wTClose;
    PtrWTInfo wTInfo;
    PtrWTEnable wTEnable;
    PtrWTOverlap wTOverlap;
    PtrWTPacketsGet wTPacketsGet;
    PtrWTGet wTGet;
    PtrWTQueueSizeGet wTQueueSizeGet;
    PtrWTQueueSizeSet wTQueueSizeSet;
};

struct QWindowsTabletDeviceData
{
    QWindowsTabletDeviceData() : minPressure(0), maxPressure(0), minTanPressure(0),
        maxTanPressure(0), minX(0), maxX(0), minY(0), maxY(0), minZ(0), maxZ(0),
        uniqueId(0), currentDevice(0), currentPointerType(0) {}

    QPointF scaleCoordinates(int coordX, int coordY,const QRect &targetArea) const;
    qreal scalePressure(qreal p) const { return p / qreal(maxPressure - minPressure); }
    qreal scaleTangentialPressure(qreal p) const { return p / qreal(maxTanPressure - minTanPressure); }

    int minPressure;
    int maxPressure;
    int minTanPressure;
    int maxTanPressure;
    int minX, maxX, minY, maxY, minZ, maxZ;
    qint64 uniqueId;
    int currentDevice;
    int currentPointerType;
    QRect virtualDesktopArea;

    // Added by Krita
    QMap<quint8, quint8> buttonsMap;
};

QDebug operator<<(QDebug d, const QWindowsTabletDeviceData &t);

class QWindowsTabletSupport
{
    Q_DISABLE_COPY(QWindowsTabletSupport)

    explicit QWindowsTabletSupport(HWND window, HCTX context);

public:
    ~QWindowsTabletSupport();

    static QWindowsTabletSupport *create();

    void notifyActivate();
    QString description() const;

    bool translateTabletProximityEvent(WPARAM wParam, LPARAM lParam);
    bool translateTabletPacketEvent();

    int absoluteRange() const { return m_absoluteRange; }
    void setAbsoluteRange(int a) { m_absoluteRange = a; }


    void tabletUpdateCursor(const int pkCursor);
    static QWindowsWinTab32DLL m_winTab32DLL;

private:
    unsigned options() const;
    QWindowsTabletDeviceData tabletInit(const quint64 uniqueId, const UINT cursorType) const;

    const HWND m_window;
    const HCTX m_context;
    int m_absoluteRange;
    bool m_tiltSupport;
    QVector<QWindowsTabletDeviceData> m_devices;
    int m_currentDevice;


    QWidget *targetWidget{0};
/**
 * This is an inelegant solution to record pen / eraser switches.
 * On the Surface Pro 3 we are only notified of cursor changes at the last minute.
 * The recommended way to handle switches is WT_CSRCHANGE, but that doesn't work
 * unless we save packet ID information, and we cannot change the structure of the
 * PACKETDATA due to Qt restrictions.
 *
 * Furthermore, WT_CSRCHANGE only ever appears *after* we receive the packet.
 */
    UINT currentPkCursor{0};
    bool isSurfacePro3{false};  //< Only enable this on SP3 or other devices with the same issue.

};

QT_END_NAMESPACE

#endif // KIS_TABLET_SUPPORT_WIN_P_H
