/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisBezierTransformMesh.h"

#include "kis_grid_interpolation_tools.h"
#include <KisBezierPatchParamSpaceUtils.h>
#include <KisSampleRectIterator.h>
#include <KisBezierPatchParamToSourceSampler.h>
#include "kis_debug.h"

KisBezierTransformMesh::patch_const_iterator
KisBezierTransformMesh::hitTestPatchImpl(const QPointF &pt, QPointF *localPointResult) const
{
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

    return result;
}

KisBezierTransformMesh::PatchIndex KisBezierTransformMesh::hitTestPatch(const QPointF &pt, QPointF *localPointResult) const
{
    return hitTestPatchImpl(pt, localPointResult).patchIndex();
}

QRect KisBezierTransformMesh::hitTestPatchInSourceSpace(const QRectF &rect) const
{
    const QRectF searchRect = rect & m_originalRect;

    if (searchRect.isEmpty()) return QRect();

    const QPointF proportionalTL = KisAlgebra2D::absoluteToRelative(searchRect.topLeft(), m_originalRect);
    const QPointF proportionalBR = KisAlgebra2D::absoluteToRelative(searchRect.bottomRight(), m_originalRect);

    const auto topItY = prev(upper_bound(m_rows.begin(), prev(m_rows.end()), proportionalTL.y()));
    const int topRow = distance(m_rows.begin(), topItY);

    const auto leftItX = prev(upper_bound(m_columns.begin(), prev(m_columns.end()), proportionalTL.x()));
    const int leftColumn = distance(m_columns.begin(), leftItX);

    const auto bottomItY = prev(upper_bound(m_rows.begin(), prev(m_rows.end()), proportionalBR.y()));
    const int bottomRow = distance(m_rows.begin(), bottomItY);

    const auto rightItX = prev(upper_bound(m_columns.begin(), prev(m_columns.end()), proportionalBR.x()));
    const int rightColumn = distance(m_columns.begin(), rightItX);

    return QRect(leftColumn, topRow,
                 rightColumn - leftColumn + 1,
                 bottomRow - topRow + 1);
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
    QRect result;

    const QRect sampleRect = rc & dstBoundingRect().toAlignedRect();
    if (sampleRect.isEmpty()) return result;

    const QRectF unitRect(0, 0, 1, 1);
    const int samplesLimit = sampleRect.width() * sampleRect.height() / 2;

    QRectF stepRect;

    {
        /**
         * First, try to approximate the bounding need rect by sampling
         * control points. That is the main property of bezier curves:
         * the resulting curve is **always** contained inside the control
         * polygon.
         *
         * TODO: sample the whole wrapping polygon in a more uniform way,
         * that is, sample the whole perimeter of the patch.
         */

        const QRectF dstRect = rc;

        auto tryAddHandle = [&dstRect, &stepRect] (const KisBezierPatch &patch, KisBezierPatch::ControlPointType controlType) {

            auto fetchLocalPoint =
                    [] (const KisBezierPatch &patch,
                        KisBezierPatch::ControlPointType c0,
                        KisBezierPatch::ControlPointType c1,
                        KisBezierPatch::ControlPointType c2,
                        KisBezierPatch::ControlPointType c3) {

                const qreal handleLength = kisDistance(patch.points[c0], patch.points[c1]);
                const qreal totalLength = handleLength +
                        kisDistance(patch.points[c1], patch.points[c2]) +
                        kisDistance(patch.points[c2], patch.points[c3]);

                return KisAlgebra2D::lerp(patch.originalRect.topLeft(), patch.originalRect.topRight(),
                                          handleLength / totalLength);
            };

            if (dstRect.contains(patch.points[controlType])) {
                QPointF localPoint;

                switch (controlType) {
                case KisBezierPatch::TL:
                    localPoint = patch.originalRect.topLeft();
                    break;
                case KisBezierPatch::TL_HC: {
                    localPoint = fetchLocalPoint(patch,
                                                 KisBezierPatch::TL,
                                                 KisBezierPatch::TL_HC,
                                                 KisBezierPatch::TR_HC,
                                                 KisBezierPatch::TR);
                    break;
                }
                case KisBezierPatch::TL_VC:
                    localPoint = fetchLocalPoint(patch,
                                                 KisBezierPatch::TL,
                                                 KisBezierPatch::TL_VC,
                                                 KisBezierPatch::BL_VC,
                                                 KisBezierPatch::BL);
                    break;
                case KisBezierPatch::TR:
                    localPoint = patch.originalRect.topRight();
                    break;
                case KisBezierPatch::TR_HC:
                    localPoint = fetchLocalPoint(patch,
                                                 KisBezierPatch::TR,
                                                 KisBezierPatch::TR_HC,
                                                 KisBezierPatch::TL_HC,
                                                 KisBezierPatch::TL);
                    break;
                case KisBezierPatch::TR_VC:
                    localPoint = fetchLocalPoint(patch,
                                                 KisBezierPatch::TR,
                                                 KisBezierPatch::TR_VC,
                                                 KisBezierPatch::BR_VC,
                                                 KisBezierPatch::BR);

                    break;
                case KisBezierPatch::BL:
                    localPoint = patch.originalRect.bottomLeft();
                    break;
                case KisBezierPatch::BL_HC:
                    localPoint = fetchLocalPoint(patch,
                                                 KisBezierPatch::BL,
                                                 KisBezierPatch::BL_HC,
                                                 KisBezierPatch::BR_HC,
                                                 KisBezierPatch::BR);
                    break;
                case KisBezierPatch::BL_VC:
                    localPoint = fetchLocalPoint(patch,
                                                 KisBezierPatch::BL,
                                                 KisBezierPatch::BL_VC,
                                                 KisBezierPatch::TL_VC,
                                                 KisBezierPatch::TL);
                    break;
                case KisBezierPatch::BR:
                    localPoint = patch.originalRect.bottomRight();
                    break;
                case KisBezierPatch::BR_HC:
                    localPoint = fetchLocalPoint(patch,
                                                 KisBezierPatch::BR,
                                                 KisBezierPatch::BR_HC,
                                                 KisBezierPatch::BL_HC,
                                                 KisBezierPatch::BL);
                    break;
                case KisBezierPatch::BR_VC:
                    localPoint = fetchLocalPoint(patch,
                                                 KisBezierPatch::BR,
                                                 KisBezierPatch::BR_VC,
                                                 KisBezierPatch::TR_VC,
                                                 KisBezierPatch::TR);

                    break;
                }

                KisAlgebra2D::accumulateBounds(localPoint, &stepRect);
            }
        };

        for (auto it = beginPatches(); it != endPatches(); ++it) {
            tryAddHandle(*it, KisBezierPatch::TL);
            tryAddHandle(*it, KisBezierPatch::TL_HC);
            tryAddHandle(*it, KisBezierPatch::TL_VC);

            tryAddHandle(*it, KisBezierPatch::TR);
            tryAddHandle(*it, KisBezierPatch::TR_HC);
            tryAddHandle(*it, KisBezierPatch::TR_VC);

            tryAddHandle(*it, KisBezierPatch::BL);
            tryAddHandle(*it, KisBezierPatch::BL_HC);
            tryAddHandle(*it, KisBezierPatch::BL_VC);

            tryAddHandle(*it, KisBezierPatch::BR);
            tryAddHandle(*it, KisBezierPatch::BR_HC);
            tryAddHandle(*it, KisBezierPatch::BR_VC);
        }
    }

    KisSampleRectIterator dstRectSampler(sampleRect);
    KisBezierPatch patch = *beginPatches();
    KisBezierPatchParamToSourceSampler patchSampler(patch);

    /// the number of points that has actually been
    /// sampled from the destination rect
    int hitPoints = 0;

    while (1) {
        for (int i = 0; i < 10; i++) {
            const QPointF dstPoint = *dstRectSampler++;

            if (patch.dstBoundingRect().contains(dstPoint)) {
                const QPointF localPoint = patch.globalToLocal(dstPoint);
                if (unitRect.contains(localPoint)) {
                    KisAlgebra2D::accumulateBounds(patchSampler.point(localPoint), &stepRect);
                    hitPoints++;
                    continue;
                }
            }

            {
                QPointF localPoint;
                auto it = hitTestPatchImpl(dstPoint, &localPoint);
                if (it != endPatches()) {
                    patch = *it;
                    patchSampler = KisBezierPatchParamToSourceSampler(patch);

                    KisAlgebra2D::accumulateBounds(patchSampler.point(localPoint), &stepRect);
                    hitPoints++;
                }
            }
        }

        QRect alignedRect = stepRect.toAlignedRect();

        if (hitPoints > 20 && !alignedRect.isEmpty() && alignedRect == result) {
            break;
        }

        result = alignedRect;

        if (dstRectSampler.numSamples() > qMin(2000, samplesLimit)) {
            /**
             * We don't warn if the "found" rect is empty, that is a perfectly
             * valid case.
             */
            if (!result.isEmpty()) {
                qWarning() << "KisBezierTransformMesh::approxNeedRect: the algorithm hasn't converged!"
                           << ppVar(hitPoints) << ppVar(stepRect) << ppVar(alignedRect) << ppVar(result);
            }
            break;
        }
    }

    return result;
}

QRect KisBezierTransformMesh::approxChangeRect(const QRect &rc) const
{
    QRect result;

    const QRect affectedPatches = hitTestPatchInSourceSpace(rc);

    for (int row = affectedPatches.top(); row <= affectedPatches.bottom(); row++) {
        for (int column = affectedPatches.left(); column <= affectedPatches.right(); column++) {
            const KisBezierPatch patch = *find(PatchIndex(column, row));
            const QRectF srcRect = QRectF(rc) & patch.srcBoundingRect();
            const QRectF paramRect = calcTightSrcRectRangeInParamSpace(patch, srcRect, 0.1);

            KisSampleRectIterator paramRectSampler(paramRect);
            QRect patchResultRect;
            QRectF stepRect;

            while (1) {
                for (int i = 0; i < 10; i++) {
                    const QPointF sampledParamPoint = *paramRectSampler++;
                    const QPointF globalPoint = patch.localToGlobal(sampledParamPoint);
                    KisAlgebra2D::accumulateBounds(globalPoint, &stepRect);
                }

                const QRect alignedRect = stepRect.toAlignedRect();

                if (!alignedRect.isEmpty() && alignedRect == patchResultRect) {
                    break;
                }

                patchResultRect = alignedRect;

                if (paramRectSampler.numSamples() > 2000) {
                    qWarning() << "KisBezierTransformMesh::approxChangeRect: the algorithm hasn't converged!"
                               << ppVar(result) << ppVar(patchResultRect) << ppVar(stepRect);
                    break;
                }
            }

            result |= patchResultRect;
        }
    }

    return result;
}


/**
 * Approximate the param-space rect that corresponds to \p srcSpaceRect in the source-space.
 * The resulting param-space rect will fully cover the source-space rect (and will be bigger).
 */
QRectF KisBezierTransformMesh::calcTightSrcRectRangeInParamSpace(const KisBezierPatch &patch, const QRectF &srcSpaceRect, qreal srcPrecision)
{
    using KisBezierUtils::Range;
    using KisBezierUtils::calcTightSrcRectRangeInParamSpace1D;

    KIS_ASSERT_RECOVER_NOOP(patch.srcBoundingRect().contains(srcSpaceRect));

    KisBezierPatchParamToSourceSampler sampler(patch);

    auto xSampler = [sampler] (qreal xParam) -> Range {
        return sampler.xRange(xParam);
    };

    auto ySampler = [sampler] (qreal yParam) -> Range {
        return sampler.yRange(yParam);
    };

    Range externalRangeX;
    Range internalRangeX;

    Range externalRangeY;
    Range internalRangeY;

    std::tie(externalRangeX, internalRangeX) =
        calcTightSrcRectRangeInParamSpace1D({0.0, 1.0},
                                            Range::fromRectX(patch.originalRect),
                                            Range::fromRectX(srcSpaceRect),
                                            xSampler, srcPrecision);

    std::tie(externalRangeY, internalRangeY) =
        calcTightSrcRectRangeInParamSpace1D({0.0, 1.0},
                                            Range::fromRectY(patch.originalRect),
                                            Range::fromRectY(srcSpaceRect),
                                            ySampler, srcPrecision);

    return Range::makeRectF(externalRangeX, externalRangeY);
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
