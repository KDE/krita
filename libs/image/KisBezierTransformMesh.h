/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

    PatchIndex hitTestPatch(const QPointF &pt, QPointF *localPointResult = 0) const;

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

    friend KRITAIMAGE_EXPORT void saveValue(QDomElement *parent, const QString &tag, const KisBezierTransformMesh &mesh);
    friend KRITAIMAGE_EXPORT bool loadValue(const QDomElement &parent, KisBezierTransformMesh *mesh);
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
