/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_MULTIPLE_PROJECTION_H
#define __KIS_MULTIPLE_PROJECTION_H

#include <QScopedPointer>
#include "kis_types.h"
#include "kritaimage_export.h"

class KisLayerStyleFilterEnvironment;

class KRITAIMAGE_EXPORT KisMultipleProjection
{
public:
    KisMultipleProjection();
    KisMultipleProjection(const KisMultipleProjection &rhs);
    ~KisMultipleProjection();

    static QString defaultProjectionId();

    KisPaintDeviceSP getProjection(const QString &id, const QString &compositeOpId, quint8 opacity, const QBitArray &channelFlags, KisPaintDeviceSP prototype);
    void freeProjection(const QString &id);
    void freeAllProjections();

    void clear(const QRect &rc);

    void apply(KisPaintDeviceSP dstDevice, const QRect &rect, KisLayerStyleFilterEnvironment *env);

    KisPaintDeviceList getLodCapableDevices() const;

    bool isEmpty() const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_MULTIPLE_PROJECTION_H */
