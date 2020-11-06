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

#include "KisBezierTransformMesh.h"

#include "kis_grid_interpolation_tools.h"
#include "kis_debug.h"

void KisBezierTransformMesh::transformPatch(const KisBezierPatch &patch, const QPoint &srcQImageOffset, const QImage &srcImage, const QPoint &dstQImageOffset, QImage *dstImage)
{
    QVector<QPointF> originalPointsLocal;
    QVector<QPointF> transformedPointsLocal;
    QSize gridSize;

    patch.sampleRegularGrid(gridSize, originalPointsLocal, transformedPointsLocal, QPointF(8,8));
    //patch.sampleIrregularGrid(gridSize, originalPointsLocal, transformedPointsLocal);

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
    //patch.sampleIrregularGrid(gridSize, originalPointsLocal, transformedPointsLocal);

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
