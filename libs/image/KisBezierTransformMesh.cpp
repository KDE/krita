/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisBezierTransformMesh.h"

#include "kis_grid_interpolation_tools.h"
#include "kis_debug.h"

KisBezierTransformMesh::PatchIndex KisBezierTransformMesh::hitTestPatch(const QPointF &pt, QPointF *localPointResult) const {
    auto result = endPatches();

    const QRectF unitRect(0, 0, 1, 1);

    for (auto it = beginPatches(); it != endPatches(); ++it) {
        Patch patch = *it;

        if (patch.dstBoundingRect().contains(pt)) {
            const QPointF localPos = KisBezierUtils::calculateLocalPos(patch.points, pt);

            if (unitRect.contains(localPos)) {

                if (localPointResult) {
                    *localPointResult = localPos;
                }

                result = it;
                break;
            }
        }
    }

    return result.patchIndex();
}

void KisBezierTransformMesh::transformPatch(const KisBezierPatch &patch, const QPoint &srcQImageOffset, const QImage &srcImage, const QPoint &dstQImageOffset, QImage *dstImage)
{
    QVector<QPointF> originalPointsLocal;
    QVector<QPointF> transformedPointsLocal;
    QSize gridSize;

    patch.sampleRegularGrid(gridSize, originalPointsLocal, transformedPointsLocal, QPointF(8,8));

    const QRect dstBoundsI = patch.dstBoundingRect().toAlignedRect();
    const QRect imageSize = QRect(dstQImageOffset, dstImage->size());
    KIS_SAFE_ASSERT_RECOVER_NOOP(imageSize.contains(dstBoundsI));

    {
        GridIterationTools::QImagePolygonOp polygonOp(srcImage, *dstImage, srcQImageOffset, dstQImageOffset);

        GridIterationTools::RegularGridIndexesOp indexesOp(gridSize);
        GridIterationTools::iterateThroughGrid
                <GridIterationTools::AlwaysCompletePolygonPolicy>(polygonOp, indexesOp,
                                                                  gridSize,
                                                                  originalPointsLocal,
                                                                  transformedPointsLocal);
    }
}

void KisBezierTransformMesh::transformPatch(const KisBezierPatch &patch, KisPaintDeviceSP srcDevice, KisPaintDeviceSP dstDevice)
{
    QVector<QPointF> originalPointsLocal;
    QVector<QPointF> transformedPointsLocal;
    QSize gridSize;

    patch.sampleRegularGrid(gridSize, originalPointsLocal, transformedPointsLocal, QPointF(8,8));

    {
        GridIterationTools::PaintDevicePolygonOp polygonOp(srcDevice, dstDevice);

        GridIterationTools::RegularGridIndexesOp indexesOp(gridSize);
        GridIterationTools::iterateThroughGrid
                <GridIterationTools::AlwaysCompletePolygonPolicy>(polygonOp, indexesOp,
                                                                  gridSize,
                                                                  originalPointsLocal,
                                                                  transformedPointsLocal);
    }
}

void KisBezierTransformMesh::transformMesh(const QPoint &srcQImageOffset, const QImage &srcImage, const QPoint &dstQImageOffset, QImage *dstImage) const
{
    for (auto it = beginPatches(); it != endPatches(); ++it) {
        transformPatch(*it, srcQImageOffset, srcImage, dstQImageOffset, dstImage);
    }
}

void KisBezierTransformMesh::transformMesh(KisPaintDeviceSP srcDevice, KisPaintDeviceSP dstDevice) const
{
    for (auto it = beginPatches(); it != endPatches(); ++it) {
        transformPatch(*it, srcDevice, dstDevice);
    }
}

QRect KisBezierTransformMesh::approxNeedRect(const QRect &rc) const
{
    QRect result = rc;

    for (auto it = beginPatches(); it != endPatches(); ++it) {
        KisBezierPatch patch = *it;

        if (patch.dstBoundingRect().intersects(rc)) {
            result |= patch.srcBoundingRect().toAlignedRect();
        }
    }

    return result;
}

QRect KisBezierTransformMesh::approxChangeRect(const QRect &rc) const
{
    QRect result = rc;

    for (auto it = beginPatches(); it != endPatches(); ++it) {
        const KisBezierPatch patch = *it;

        if (patch.srcBoundingRect().intersects(rc)) {
            result |= patch.dstBoundingRect().toAlignedRect();
        }
    }

    return result;
}

#include <kis_dom_utils.h>

void KisBezierTransformMeshDetail::saveValue(QDomElement *parent, const QString &tag, const KisBezierTransformMesh &mesh)
{
    QDomDocument doc = parent->ownerDocument();
    QDomElement e = doc.createElement(tag);
    parent->appendChild(e);

    e.setAttribute("type", "transform-mesh");

    KisDomUtils::saveValue(&e, "size", mesh.m_size);
    KisDomUtils::saveValue(&e, "srcRect", mesh.m_originalRect);
    KisDomUtils::saveValue(&e, "columns", mesh.m_columns);
    KisDomUtils::saveValue(&e, "rows", mesh.m_rows);
    KisDomUtils::saveValue(&e, "nodes", mesh.m_nodes);
}

bool KisBezierTransformMeshDetail::loadValue(const QDomElement &e, KisBezierTransformMesh *mesh)
{
    if (!KisDomUtils::Private::checkType(e, "transform-mesh")) return false;

    mesh->m_columns.clear();
    mesh->m_rows.clear();
    mesh->m_nodes.clear();

    KisDomUtils::loadValue(e, "size", &mesh->m_size);
    KisDomUtils::loadValue(e, "srcRect", &mesh->m_originalRect);
    KisDomUtils::loadValue(e, "columns", &mesh->m_columns);
    KisDomUtils::loadValue(e, "rows", &mesh->m_rows);
    KisDomUtils::loadValue(e, "nodes", &mesh->m_nodes);

    return true;
}
