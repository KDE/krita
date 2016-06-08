/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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
