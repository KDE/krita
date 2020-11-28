/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_paint_device_debug_utils.h"

#include <QRect>
#include <QImage>

#include "kis_paint_device.h"


void kis_debug_save_device_incremental(KisPaintDeviceSP device,
                                       int i,
                                       const QRect &rc,
                                       const QString &suffix, const QString &prefix)
{
    QString filename = QString("%1_%2.png").arg(i).arg(suffix);

    if (!prefix.isEmpty()) {
        filename = QString("%1_%2.png").arg(prefix).arg(filename);
    }

    QRect saveRect(rc);

    if (saveRect.isEmpty()) {
        saveRect = device->exactBounds();
    }

    qDebug() << "Dumping:" << filename;
    device->convertToQImage(0, saveRect).save(filename);
}
