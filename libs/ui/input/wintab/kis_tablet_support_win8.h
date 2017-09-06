/*
 * Copyright (c) 2017 Alvin Wong <alvinhochun@gmail.com>
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

#ifndef KIS_TABLET_SUPPORT_WIN8_H
#define KIS_TABLET_SUPPORT_WIN8_H

#include <QAbstractNativeEventFilter>

#include <kritaui_export.h>

class KRITAUI_EXPORT KisTabletSupportWin8 : public QAbstractNativeEventFilter
{
    Q_DISABLE_COPY(KisTabletSupportWin8)

public:
    static bool isAvailable();

    KisTabletSupportWin8() = default;
    ~KisTabletSupportWin8() = default;

    bool init();
    // void registerPointerDeviceNotifications();
    virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
};

#endif // KIS_TABLET_SUPPORT_WIN8_H
