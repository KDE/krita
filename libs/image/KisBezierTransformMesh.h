/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISBEZIERTRANSFORMMESH_H
#define KISBEZIERTRANSFORMMESH_H

#include "kritaimage_export.h"
#include "KisBezierMesh.h"

#include "kis_types.h"

namespace KisBezierTransformMeshDetail {

class KRITAIMAGE_EXPORT KisBezierTransformMesh : public KisBezierMesh
{
public:
    KisBezierTransformMesh()
    {
    }
    KisBezierTransformMesh(const QRectF &srcRect, const QSize &size = QSize(2,2))
        : KisBezierMesh(srcRect, size)
    {
    }

    static void transformPatch(const KisBezierPatch &patch,
                               const QPoint &srcQImageOffset,
                               const QImage &srcImage,
                               const QPoint &dstQImageOffset,
                               QImage *dstImage);

    static void transformPatch(const KisBezierPatch &patch,
                               KisPaintDeviceSP srcDevice,
                               KisPaintDeviceSP dstDevice);


    void transformMesh(const QPoint &srcQImageOffset,
                       const QImage &srcImage,
                       const QPoint &dstQImageOffset,
                       QImage *dstImage) const;

    void transformMesh(KisPaintDeviceSP srcDevice,
                       KisPaintDeviceSP dstDevice) const;

    QRect approxNeedRect(const QRect &rc) const;
    QRect approxChangeRect(const QRect &rc) const;

    friend void saveValue(QDomElement *parent, const QString &tag, const KisBezierTransformMesh &mesh);
    friend bool loadValue(const QDomElement &parent, KisBezierTransformMesh *mesh);
};

KRITAIMAGE_EXPORT
void saveValue(QDomElement *parent, const QString &tag, const KisBezierTransformMesh &mesh);

KRITAIMAGE_EXPORT
bool loadValue(const QDomElement &parent, KisBezierTransformMesh *mesh);

}

namespace KisDomUtils {
using KisBezierTransformMeshDetail::loadValue;
using KisBezierTransformMeshDetail::saveValue;
}

using KisBezierTransformMesh = KisBezierTransformMeshDetail::KisBezierTransformMesh;

#endif // KISBEZIERTRANSFORMMESH_H
