/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#include "krita_utils.h"

#include <QRect>

namespace KritaUtils
{

    QVector<QRect> KRITAIMAGE_EXPORT splitRectIntoPatches(const QRect &rc, const QSize &patchSize)
    {
        QVector<QRect> patches;

        qint32 firstCol = rc.x() / patchSize.width();
        qint32 firstRow = rc.y() / patchSize.height();

        qint32 lastCol = (rc.x() + rc.width()) / patchSize.width();
        qint32 lastRow = (rc.y() + rc.height()) / patchSize.height();

        for(qint32 i = firstRow; i <= lastRow; i++) {
            for(qint32 j = firstCol; j <= lastCol; j++) {
                QRect maxPatchRect(j * patchSize.width(), i * patchSize.height(),
                                   patchSize.width(), patchSize.height());
                QRect patchRect = rc & maxPatchRect;

                patches.append(patchRect);
            }
        }

        return patches;
    }

}
