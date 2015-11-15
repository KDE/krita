/*
 *  Copyright (c) 2013 Digia Plc and/or its subsidiary(-ies).
 *  Copyright (c) 2013 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KIS_TABLET_SUPPORT_WIN_H
#define KIS_TABLET_SUPPORT_WIN_H

#include <Qt>
#include <kritaui_export.h>

#include <QAbstractNativeEventFilter>

#ifndef _WINDEF_
typedef unsigned long DWORD;
#endif

class KRITAUI_EXPORT KisTabletSupportWin : public QAbstractNativeEventFilter
{
public:
    struct KRITAUI_EXPORT ButtonsConverter {
        virtual ~ButtonsConverter() {}
        virtual void convert(DWORD btnOld, DWORD btnNew,
                             Qt::MouseButton *button,
                             Qt::MouseButtons *buttons) = 0;
    };

public:
    static void init();
    static void setButtonsConverter(ButtonsConverter *buttonsConverter);
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);

};


struct QWindowsTabletDeviceData
{
	QWindowsTabletDeviceData() : minPressure(0), maxPressure(0), minTanPressure(0),
		maxTanPressure(0), minX(0), maxX(0), minY(0), maxY(0), minZ(0), maxZ(0),
    uniqueId(0), currentDevice(0), currentPointerType(0) {}


	QPointF scaleCoordinates(int coordX, int coordY, const QRect &targetArea) const;
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

};



#endif // KIS_TABLET_SUPPORT_WIN_H
