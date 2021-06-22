/*
 * SPDX-FileCopyrightText: 2017 Alvin Wong <alvinhochun@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
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
    static bool isPenDeviceAvailable();

    KisTabletSupportWin8() = default;
    ~KisTabletSupportWin8() = default;

    bool init();
    // void registerPointerDeviceNotifications();
    virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
};

#endif // KIS_TABLET_SUPPORT_WIN8_H
