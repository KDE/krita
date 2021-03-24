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
