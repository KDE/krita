/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISINTERSTROKEDATA_H
#define KISINTERSTROKEDATA_H

#include <kritaimage_export.h>
#include <QSharedPointer>
#include <QPoint>
#include <kis_types.h>

class KUndo2Command;
class KoColorSpace;

/**
 * A special base class for storing temporary data inside a paint
 * device between brush strokes. That might be used by the brushes to
 * store data that needs to be passes between different strokes of the
 * same brush, e.g. paint drying drying or heightmap information.
 *
 * The data is stored inside device->interstrokeData() and added
 * via passing a factory to the transaction.
 *
 * The data is automatically removed when an incompatible change
 * happens to a device, e.g. colorspace change or painting with
 * incompatible brush.
 */
class KRITAIMAGE_EXPORT KisInterstrokeData
{
public:
    KisInterstrokeData(KisPaintDeviceSP device);
    virtual ~KisInterstrokeData();

    virtual void beginTransaction() = 0;
    virtual KUndo2Command* endTransaction() = 0;

    bool isStillCompatible() const;

private:
    QPoint m_linkedDeviceOffset;
    const KoColorSpace *m_linkedColorSpace = 0;
    KisPaintDeviceWSP m_linkedPaintDevice;
};

using KisInterstrokeDataSP = QSharedPointer<KisInterstrokeData>;


#endif // KISINTERSTROKEDATA_H
