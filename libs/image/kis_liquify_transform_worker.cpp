/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_liquify_transform_worker.h"

#include <KoColorSpace.h>
#include "kis_grid_interpolation_tools.h"
#include "kis_dom_utils.h"
#include "krita_utils.h"
#include "KisSpatialContainer.h"


struct Q_DECL_HIDDEN KisLiquifyTransformWorker::Private
{
    Private(const QRect &_srcBounds,
            KoUpdater *_progress,
            int _pixelPrecision)
        : srcBounds(_srcBounds)
        , originalPointsContainer(_srcBounds)
        , transformedPointsContainer(_srcBounds)
        , progress(_progress)
        , pixelPrecision(_pixelPrecision)
    {
    }

    QRect srcBounds;

    QVector<QPointF> originalPoints;
    QVector<QPointF> transformedPoints;

    KisSpatialContainer originalPointsContainer;
    KisSpatialContainer transformedPointsContainer;

    QRectF accumulatedBrushStrokes;

    KoUpdater *progress;
    int pixelPrecision;
    QSize gridSize;

    void preparePoints();

    struct MapIndexesOp;

    template <class ProcessOp>
    void processTransformedPixelsBuildUp(ProcessOp op,
                                         const QPointF &base,
                                         qreal sigma);

    template <class ProcessOp>
    void processTransformedPixelsWash(ProcessOp op,
                                      const QPointF &base,
                                      qreal sigma,
                                      qreal flow);

    template <class ProcessOp>
    void processTransformedPixels(ProcessOp op,
                                  const QPointF &base,
                                  qreal sigma,
                                  bool useWashMode,
                                  qreal flow);
};

KisLiquifyTransformWorker::KisLiquifyTransformWorker(const QRect &srcBounds,
                                                     KoUpdater *progress,
                                                     int pixelPrecision)
    : m_d(new Private(srcBounds, progress, pixelPrecision))
{
    KIS_ASSERT_RECOVER_RETURN(!srcBounds.isEmpty());

    // TODO: implement 'progress' stuff
    m_d->preparePoints();
}

KisLiquifyTransformWorker::KisLiquifyTransformWorker(const KisLiquifyTransformWorker &rhs)
    : m_d(new Private(*rhs.m_d.data()))
{
}

KisLiquifyTransformWorker::~KisLiquifyTransformWorker()
{
}

bool KisLiquifyTransformWorker::operator==(const KisLiquifyTransformWorker &other) const
{
    bool result =
            m_d->srcBounds == other.m_d->srcBounds &&
            m_d->pixelPrecision == other.m_d->pixelPrecision &&
            m_d->gridSize == other.m_d->gridSize &&
            m_d->originalPoints.size() == other.m_d->originalPoints.size() &&
            m_d->transformedPoints.size() == other.m_d->transformedPoints.size();

    if (!result) return false;

    const qreal eps = 1e-6;

    result =
        KisAlgebra2D::fuzzyPointCompare(m_d->originalPoints, other.m_d->originalPoints, eps) &&
        KisAlgebra2D::fuzzyPointCompare(m_d->transformedPoints, other.m_d->transformedPoints, eps);

    return result;
}

bool KisLiquifyTransformWorker::isIdentity() const
{
    const qreal eps = 1e-6;
    return KisAlgebra2D::fuzzyPointCompare(m_d->originalPoints, m_d->transformedPoints, eps);
}

int KisLiquifyTransformWorker::pointToIndex(const QPoint &cellPt)
{
    return GridIterationTools::pointToIndex(cellPt, m_d->gridSize);
}

QSize KisLiquifyTransformWorker::gridSize() const
{
    return m_d->gridSize;
}

const QVector<QPointF>& KisLiquifyTransformWorker::originalPoints() const
{
    return m_d->originalPoints;
}

QVector<QPointF>& KisLiquifyTransformWorker::transformedPoints()
{
    return m_d->transformedPoints;
}

struct AllPointsFetcherOp
{
    AllPointsFetcherOp(QRectF srcRect) : m_srcRect(srcRect) {}

    inline void processPoint(int col, int row,
                             int prevCol, int prevRow,
                             int colIndex, int rowIndex) {

        Q_UNUSED(prevCol);
        Q_UNUSED(prevRow);
        Q_UNUSED(colIndex);
        Q_UNUSED(rowIndex);

        QPointF pt(col, row);
        m_points << pt;
    }

    inline void nextLine() {
    }

    QVector<QPointF> m_points;
    QRectF m_srcRect;
};

void KisLiquifyTransformWorker::Private::preparePoints()
{
    gridSize =
        GridIterationTools::calcGridSize(srcBounds, pixelPrecision);

    AllPointsFetcherOp pointsOp(srcBounds);
    GridIterationTools::processGrid(pointsOp, srcBounds, pixelPrecision);

    const int numPoints = pointsOp.m_points.size();

    KIS_ASSERT_RECOVER_RETURN(numPoints == gridSize.width() * gridSize.height());

    originalPoints = pointsOp.m_points;
    transformedPoints = pointsOp.m_points;

    originalPointsContainer.initializeWithGridPoints(srcBounds, pixelPrecision);
    transformedPointsContainer.initializeWithGridPoints(srcBounds, pixelPrecision);

}

void KisLiquifyTransformWorker::translate(const QPointF &offset)
{
    KIS_ASSERT_RECOVER_RETURN(m_d->originalPoints.size() ==
                              m_d->transformedPoints.size());

    // TODO: make it within Spatial Container, either a hidden offset, or just offsetting all points at once
    // and benchmark
    for (int i = 0; i < m_d->transformedPoints.count(); i++) {
        m_d->originalPointsContainer.movePoint(i, m_d->originalPoints[i], m_d->originalPoints[i] + offset);
        m_d->transformedPointsContainer.movePoint(i, m_d->transformedPoints[i], m_d->transformedPoints[i] + offset);

        m_d->originalPoints[i] += offset;
        m_d->transformedPoints[i] += offset;
    }

    m_d->accumulatedBrushStrokes.translate(offset);
}

void KisLiquifyTransformWorker::translateDstSpace(const QPointF &offset)
{
    // TODO: make it within Spatial Container, either a hidden offset, or just offsetting all points at once
    // and benchmark
    for (int i = 0; i < m_d->transformedPoints.count(); i++) {
        m_d->transformedPointsContainer.movePoint(i, m_d->transformedPoints[i], m_d->transformedPoints[i] + offset);
        m_d->transformedPoints[i] += offset;
    }
}

void KisLiquifyTransformWorker::undoPoints(const QPointF &base,
                                           qreal amount,
                                           qreal sigma)
{
    const qreal maxDistCoeff = 3.0;
    const qreal maxDist = maxDistCoeff * sigma;

    KIS_ASSERT_RECOVER_RETURN(m_d->originalPoints.size() ==
                              m_d->transformedPoints.size());

    QVector<int> indexes;
    m_d->transformedPointsContainer.findAllInRange(indexes, base, maxDist);
    for (int i = 0; i < indexes.count(); i++) {

        QPointF diff = m_d->transformedPoints[indexes[i]] - base;
        qreal dist = KisAlgebra2D::norm(diff);
        qreal lambda = exp(-0.5 * pow2(dist / sigma));
        lambda *= amount;

        QPointF oldPosition = m_d->transformedPoints[indexes[i]];
        m_d->transformedPoints[indexes[i]] = m_d->originalPoints[indexes[i]] * lambda + m_d->transformedPoints[indexes[i]] * (1.0 - lambda);

        m_d->transformedPointsContainer.movePoint(indexes[i], oldPosition, m_d->transformedPoints[indexes[i]]);
    }
}

namespace {

struct PointUpdate
{
    int index = -1;
    QPointF oldPosition;
    QPointF newPosition;
};

bool solveLinear3x3(const qreal matrix[3][3], const qreal rhs[3], qreal result[3])
{
    const qreal determinant =
        matrix[0][0] * (matrix[1][1] * matrix[2][2] - matrix[1][2] * matrix[2][1]) -
        matrix[0][1] * (matrix[1][0] * matrix[2][2] - matrix[1][2] * matrix[2][0]) +
        matrix[0][2] * (matrix[1][0] * matrix[2][1] - matrix[1][1] * matrix[2][0]);

    if (qAbs(determinant) < 1e-12) return false;

    const qreal invDeterminant = 1.0 / determinant;

    qreal inverse[3][3];
    inverse[0][0] =  (matrix[1][1] * matrix[2][2] - matrix[1][2] * matrix[2][1]) * invDeterminant;
    inverse[0][1] = -(matrix[0][1] * matrix[2][2] - matrix[0][2] * matrix[2][1]) * invDeterminant;
    inverse[0][2] =  (matrix[0][1] * matrix[1][2] - matrix[0][2] * matrix[1][1]) * invDeterminant;
    inverse[1][0] = -(matrix[1][0] * matrix[2][2] - matrix[1][2] * matrix[2][0]) * invDeterminant;
    inverse[1][1] =  (matrix[0][0] * matrix[2][2] - matrix[0][2] * matrix[2][0]) * invDeterminant;
    inverse[1][2] = -(matrix[0][0] * matrix[1][2] - matrix[0][2] * matrix[1][0]) * invDeterminant;
    inverse[2][0] =  (matrix[1][0] * matrix[2][1] - matrix[1][1] * matrix[2][0]) * invDeterminant;
    inverse[2][1] = -(matrix[0][0] * matrix[2][1] - matrix[0][1] * matrix[2][0]) * invDeterminant;
    inverse[2][2] =  (matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0]) * invDeterminant;

    for (int i = 0; i < 3; i++) {
        result[i] = inverse[i][0] * rhs[0] + inverse[i][1] * rhs[1] + inverse[i][2] * rhs[2];
    }

    return true;
}

}

void KisLiquifyTransformWorker::relaxPoints(const QPointF &base,
                                            qreal amount,
                                            qreal sigma)
{
    const qreal maxDistCoeff = 3.0;
    const qreal maxDist = maxDistCoeff * sigma;

    KIS_ASSERT_RECOVER_RETURN(m_d->originalPoints.size() ==
                              m_d->transformedPoints.size());

    QRectF clipRect(base.x() - maxDist, base.y() - maxDist,
                    2 * maxDist, 2 * maxDist);
    m_d->accumulatedBrushStrokes |= kisGrowRect(clipRect, m_d->pixelPrecision);

    QVector<int> indexes;
    m_d->transformedPointsContainer.findAllInRange(indexes, base, maxDist);

    QVector<PointUpdate> updates;
    updates.reserve(indexes.count());

    /*
     * Blur the displacement field, not the transformed point positions.
     * Repeatedly blurring positions would pull all coordinates in the dab
     * toward their centroid, effectively compressing the grid like a black
     * hole. Blurring displacements keeps the original grid structure and
     * only reduces local disagreement in how neighboring points moved.
     *
     * Store all updates first so every point is relaxed against the same
     * pre-dab state; otherwise the result would depend on iteration order.
     */
    for (int i = 0; i < indexes.count(); i++) {
        const int index = indexes[i];

        QPointF diff = m_d->transformedPoints[index] - base;
        qreal dist = KisAlgebra2D::norm(diff);
        if (dist > maxDist) continue;

        qreal lambda = exp(-0.5 * pow2(dist / sigma));
        lambda *= amount;

        const int col = index % m_d->gridSize.width();
        const int row = index / m_d->gridSize.width();

        QPointF displacementSum;
        int displacementCount = 0;

        for (int neighborRow = qMax(0, row - 1);
             neighborRow <= qMin(m_d->gridSize.height() - 1, row + 1);
             neighborRow++) {
            for (int neighborCol = qMax(0, col - 1);
                 neighborCol <= qMin(m_d->gridSize.width() - 1, col + 1);
                 neighborCol++) {

                if (neighborCol == col && neighborRow == row) continue;

                const int neighborIndex =
                    GridIterationTools::pointToIndex(QPoint(neighborCol, neighborRow),
                                                     m_d->gridSize);

                displacementSum += m_d->transformedPoints[neighborIndex] -
                    m_d->originalPoints[neighborIndex];
                displacementCount++;
            }
        }

        if (!displacementCount) continue;

        const QPointF oldDisplacement =
            m_d->transformedPoints[index] - m_d->originalPoints[index];
        const QPointF relaxedDisplacement = displacementSum / displacementCount;

        const QPointF newPosition =
            m_d->originalPoints[index] +
            oldDisplacement * (1.0 - lambda) +
            relaxedDisplacement * lambda;

        updates << PointUpdate{index, m_d->transformedPoints[index], newPosition};
    }

    for (int i = 0; i < updates.count(); i++) {
        const PointUpdate &update = updates[i];
        m_d->transformedPoints[update.index] = update.newPosition;
        m_d->transformedPointsContainer.movePoint(update.index, update.oldPosition, update.newPosition);
    }
}

void KisLiquifyTransformWorker::affineRelaxPoints(const QPointF &base,
                                                  qreal amount,
                                                  qreal sigma)
{
    const qreal maxDistCoeff = 3.0;
    const qreal maxDist = maxDistCoeff * sigma;
    const int neighborhoodRadius = 2;

    KIS_ASSERT_RECOVER_RETURN(m_d->originalPoints.size() ==
                              m_d->transformedPoints.size());

    QRectF clipRect(base.x() - maxDist, base.y() - maxDist,
                    2 * maxDist, 2 * maxDist);
    m_d->accumulatedBrushStrokes |= kisGrowRect(clipRect, m_d->pixelPrecision);

    QVector<int> indexes;
    m_d->transformedPointsContainer.findAllInRange(indexes, base, maxDist);

    QVector<PointUpdate> updates;
    updates.reserve(indexes.count());

    /*
     * Fit a local affine transform from neighboring original points to their
     * transformed positions. This preserves broad local translation, rotation,
     * scale and shear. A pure displacement blur would slowly damp rotations
     * because a rotated area has different displacement vectors at different
     * points, even when the rotation itself is perfectly smooth.
     *
     * Like simple relax, collect updates first so each fit samples the same
     * pre-dab deformation grid.
     */
    for (int i = 0; i < indexes.count(); i++) {
        const int index = indexes[i];

        QPointF diff = m_d->transformedPoints[index] - base;
        qreal dist = KisAlgebra2D::norm(diff);
        if (dist > maxDist) continue;

        qreal lambda = exp(-0.5 * pow2(dist / sigma));
        lambda *= amount;

        const int col = index % m_d->gridSize.width();
        const int row = index / m_d->gridSize.width();
        const QPointF originalCenter = m_d->originalPoints[index];

        qreal matrix[3][3] = {};
        qreal rhsX[3] = {};
        qreal rhsY[3] = {};
        int sampleCount = 0;

        for (int neighborRow = qMax(0, row - neighborhoodRadius);
             neighborRow <= qMin(m_d->gridSize.height() - 1, row + neighborhoodRadius);
             neighborRow++) {
            for (int neighborCol = qMax(0, col - neighborhoodRadius);
                 neighborCol <= qMin(m_d->gridSize.width() - 1, col + neighborhoodRadius);
                 neighborCol++) {

                if (neighborCol == col && neighborRow == row) continue;

                const int neighborIndex =
                    GridIterationTools::pointToIndex(QPoint(neighborCol, neighborRow),
                                                     m_d->gridSize);
                const QPointF originalOffset =
                    m_d->originalPoints[neighborIndex] - originalCenter;
                const qreal fitPoint[3] = {originalOffset.x(), originalOffset.y(), 1.0};

                const qreal gridDistance = KisAlgebra2D::norm(originalOffset);
                const qreal fitSigma = qMax<qreal>(m_d->pixelPrecision, 1.0);
                const qreal weight = exp(-0.5 * pow2(gridDistance / (neighborhoodRadius * fitSigma)));

                for (int r = 0; r < 3; r++) {
                    for (int c = 0; c < 3; c++) {
                        matrix[r][c] += weight * fitPoint[r] * fitPoint[c];
                    }
                    rhsX[r] += weight * fitPoint[r] * m_d->transformedPoints[neighborIndex].x();
                    rhsY[r] += weight * fitPoint[r] * m_d->transformedPoints[neighborIndex].y();
                }

                sampleCount++;
            }
        }

        qreal coeffX[3];
        qreal coeffY[3];
        if (sampleCount < 3 ||
            !solveLinear3x3(matrix, rhsX, coeffX) ||
            !solveLinear3x3(matrix, rhsY, coeffY)) {

            continue;
        }

        const QPointF predictedPosition(coeffX[2], coeffY[2]);

        const QPointF newPosition =
            m_d->transformedPoints[index] * (1.0 - lambda) +
            predictedPosition * lambda;

        updates << PointUpdate{index, m_d->transformedPoints[index], newPosition};
    }

    for (int i = 0; i < updates.count(); i++) {
        const PointUpdate &update = updates[i];
        m_d->transformedPoints[update.index] = update.newPosition;
        m_d->transformedPointsContainer.movePoint(update.index, update.oldPosition, update.newPosition);
    }
}

template <class ProcessOp>
void KisLiquifyTransformWorker::Private::
processTransformedPixelsBuildUp(ProcessOp op,
                                const QPointF &base,
                                qreal sigma)
{
    const qreal maxDist = ProcessOp::maxDistCoeff * sigma;
    QRectF clipRect(base.x() - maxDist, base.y() - maxDist,
                    2 * maxDist, 2 * maxDist);

    accumulatedBrushStrokes |= kisGrowRect(clipRect, pixelPrecision);

    QVector<int> indexes;
    transformedPointsContainer.findAllInRange(indexes, base, maxDist);

    for (int i = 0; i < indexes.count(); i++) {

        QPointF diff = transformedPoints[indexes[i]] - base;
        qreal dist = KisAlgebra2D::norm(diff);
        if (dist > maxDist) continue;

        const qreal lambda = exp(-0.5 * pow2(dist / sigma));
        QPointF oldPosition = transformedPoints[indexes[i]];
        transformedPoints[indexes[i]] = op(transformedPoints[indexes[i]], base, diff, lambda);


        transformedPointsContainer.movePoint(indexes[i], oldPosition, transformedPoints[indexes[i]]);

    }
}

template <class ProcessOp>
void KisLiquifyTransformWorker::Private::
processTransformedPixelsWash(ProcessOp op,
                             const QPointF &base,
                             qreal sigma,
                             qreal flow)
{
    const qreal maxDist = ProcessOp::maxDistCoeff * sigma;
    QRectF clipRect(base.x() - maxDist, base.y() - maxDist,
                    2 * maxDist, 2 * maxDist);

    accumulatedBrushStrokes |= kisGrowRect(clipRect, pixelPrecision);

    KIS_ASSERT_RECOVER_RETURN(originalPoints.size() ==
                              transformedPoints.size());

    // TODO: remove the originalPointsContainer entirely, and use GridIterationTools to figure out indexes instead
    // and add unit tests for it

    QVector<int> indexes;
    originalPointsContainer.findAllInRange(indexes, base, maxDist);
    for (int i = 0; i < indexes.count(); i++) {

        QPointF diff = originalPoints[indexes[i]] - base;
        qreal dist = KisAlgebra2D::norm(diff);

        const qreal lambda = exp(-0.5 * pow2(dist / sigma));
        QPointF dstPt = op(originalPoints[indexes[i]], base, diff, lambda);

        if (kisDistance(dstPt, originalPoints[indexes[i]]) > kisDistance(transformedPoints[indexes[i]], originalPoints[indexes[i]])) {
            QPointF oldPosition = transformedPoints[indexes[i]];
            transformedPoints[indexes[i]] = (1.0 - flow) * transformedPoints[indexes[i]] + flow * dstPt;

            transformedPointsContainer.movePoint(indexes[i], oldPosition, transformedPoints[indexes[i]]);
        }
    }
}

template <class ProcessOp>
void KisLiquifyTransformWorker::Private::
processTransformedPixels(ProcessOp op,
                         const QPointF &base,
                         qreal sigma,
                         bool useWashMode,
                         qreal flow)
{
    if (useWashMode) {
        processTransformedPixelsWash(op, base, sigma, flow);
    } else {
        processTransformedPixelsBuildUp(op, base, sigma);
    }
}

struct TranslateOp
{
    TranslateOp(const QPointF &offset) : m_offset(offset) {}

    QPointF operator() (const QPointF &pt,
                        const QPointF &base,
                        const QPointF &diff,
                        qreal lambda)
    {
        Q_UNUSED(base);
        Q_UNUSED(diff);
        return pt + lambda * m_offset;
    }

    static const qreal maxDistCoeff;

    QPointF m_offset;
};

const qreal TranslateOp::maxDistCoeff = 3.0;

struct ScaleOp
{
    ScaleOp(qreal scale) : m_scale(scale) {}

    QPointF operator() (const QPointF &pt,
                        const QPointF &base,
                        const QPointF &diff,
                        qreal lambda)
    {
        Q_UNUSED(pt);
        Q_UNUSED(diff);
        return base + (1.0 + m_scale * lambda) * diff;
    }

    static const qreal maxDistCoeff;

    qreal m_scale;
};

const qreal ScaleOp::maxDistCoeff = 3.0;

struct RotateOp
{
    RotateOp(qreal angle) : m_angle(angle) {}

    QPointF operator() (const QPointF &pt,
                        const QPointF &base,
                        const QPointF &diff,
                        qreal lambda)
    {
        Q_UNUSED(pt);

        const qreal angle = m_angle * lambda;
        const qreal sinA = std::sin(angle);
        const qreal cosA = std::cos(angle);

        qreal x =  cosA * diff.x() + sinA * diff.y();
        qreal y = -sinA * diff.x() + cosA * diff.y();

        return base + QPointF(x, y);
    }

    static const qreal maxDistCoeff;

    qreal m_angle;
};

const qreal RotateOp::maxDistCoeff = 3.0;

void KisLiquifyTransformWorker::translatePoints(const QPointF &base,
                                                const QPointF &offset,
                                                qreal sigma,
                                                bool useWashMode,
                                                qreal flow)
{
    TranslateOp op(offset);
    m_d->processTransformedPixels(op, base, sigma, useWashMode, flow);
}

void KisLiquifyTransformWorker::scalePoints(const QPointF &base,
                                            qreal scale,
                                            qreal sigma,
                                            bool useWashMode,
                                            qreal flow)
{
    ScaleOp op(scale);
    m_d->processTransformedPixels(op, base, sigma, useWashMode, flow);
}

void KisLiquifyTransformWorker::rotatePoints(const QPointF &base,
                                             qreal angle,
                                             qreal sigma,
                                             bool useWashMode,
                                             qreal flow)
{
    RotateOp op(angle);
    m_d->processTransformedPixels(op, base, sigma, useWashMode, flow);
}

void KisLiquifyTransformWorker::run(KisPaintDeviceSP srcDevice, KisPaintDeviceSP dstDevice)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(*srcDevice->colorSpace() == *dstDevice->colorSpace());

    dstDevice->clear();

    using namespace GridIterationTools;
    QRect correctSubGrid = calculateCorrectSubGrid(m_d->srcBounds, m_d->pixelPrecision, m_d->accumulatedBrushStrokes, m_d->gridSize);

    PaintDevicePolygonOp polygonOp(srcDevice, dstDevice);
    RegularGridIndexesOp indexesOp(m_d->gridSize);

    bool canMergeRects = GridIterationTools::canProcessRectsInRandomOrder(indexesOp, m_d->transformedPoints, correctSubGrid);
    polygonOp.setCanMergeRects(canMergeRects);

#ifdef DEBUG_PAINTING_POLYGONS
    polygonOp.setDebugColor(Qt::red);
#endif

    iterateThroughGrid<AlwaysCompletePolygonPolicy>(polygonOp, indexesOp,
                                                    m_d->gridSize,
                                                    m_d->originalPoints,
                                                    m_d->transformedPoints,
                                                    correctSubGrid);
    QList<QRectF> areasToCopy = cutOutSubgridFromBounds(correctSubGrid, m_d->srcBounds, m_d->gridSize, m_d->originalPoints);
#ifdef DEBUG_PAINTING_POLYGONS
    QList<QColor> colors = {Qt::blue, Qt::green, Qt::yellow, Qt::black};
#endif
    for (int i = 0; i < areasToCopy.length(); i++) {
#ifdef DEBUG_PAINTING_POLYGONS
        polygonOp.setDebugColor(colors[i]);
#endif
        polygonOp.fastCopyArea(areasToCopy[i].toRect(), false);
    }
}

QRect KisLiquifyTransformWorker::approxChangeRect(const QRect &rc)
{
    const qreal margin = 0.05;
    QRect resultRect = m_d->transformedPointsContainer.exactBounds().toRect();
    return KisAlgebra2D::blowRect(resultRect | rc, margin);
}

QRect KisLiquifyTransformWorker::approxNeedRect(const QRect &rc, const QRect &fullBounds)
{
    Q_UNUSED(rc);
    return fullBounds;
}

QRectF KisLiquifyTransformWorker::accumulatedStrokesBounds() const
{
    return m_d->accumulatedBrushStrokes;
}

void KisLiquifyTransformWorker::transformSrcAndDst(const QTransform &t)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(t.type() <= QTransform::TxScale);

    m_d->srcBounds = t.mapRect(m_d->srcBounds);

    // TODO: do it within Spatial Container
    for (int i = 0; i < m_d->transformedPoints.count(); i++) {
        m_d->originalPointsContainer.movePoint(i, m_d->originalPoints[i], t.map(m_d->originalPoints[i]));
        m_d->transformedPointsContainer.movePoint(i, m_d->transformedPoints[i], t.map(m_d->transformedPoints[i]));

        m_d->originalPoints[i] = t.map(m_d->originalPoints[i]);
        m_d->transformedPoints[i] = t.map(m_d->transformedPoints[i]);
    }
    m_d->accumulatedBrushStrokes = t.map(m_d->accumulatedBrushStrokes).boundingRect();
    if (t == QTransform::fromScale(t.m11(), t.m22()) && t.m11() == t.m22()) {
        m_d->pixelPrecision *= t.m11();
        KIS_SAFE_ASSERT_RECOVER(m_d->pixelPrecision > 0) { m_d->pixelPrecision = 1; }
        KIS_SAFE_ASSERT_RECOVER(QList<int>({1, 2, 4, 8, 16}).contains(m_d->pixelPrecision) || m_d->pixelPrecision%16 == 0) { m_d->pixelPrecision = 1; }
        // should check if pixelPrecision is a power of 2, but that's more complicated
    }
}

#include <functional>
#include <QTransform>

using PointMapFunction = std::function<QPointF (const QPointF&)>;


PointMapFunction bindPointMapTransform(const QTransform &transform) {
    using namespace std::placeholders;

    typedef QPointF (QTransform::*MapFuncType)(const QPointF&) const;
    return std::bind(static_cast<MapFuncType>(&QTransform::map), &transform, _1);
}

QImage KisLiquifyTransformWorker::runOnQImage(const QImage &srcImage,
                                              const QPointF &srcImageOffset,
                                              const QTransform &imageToThumbTransform,
                                              QPointF *newOffset)
{
    KIS_ASSERT_RECOVER(m_d->originalPoints.size() == m_d->transformedPoints.size()) {
        return QImage();
    }

    KIS_ASSERT_RECOVER(!srcImage.isNull()) {
        return QImage();
    }

    KIS_ASSERT_RECOVER(srcImage.format() == QImage::Format_ARGB32) {
        return QImage();
    }

    QVector<QPointF> originalPointsLocal(m_d->originalPoints);
    QVector<QPointF> transformedPointsLocal(m_d->transformedPoints);

    PointMapFunction mapFunc = bindPointMapTransform(imageToThumbTransform);

    std::transform(originalPointsLocal.begin(), originalPointsLocal.end(),
                   originalPointsLocal.begin(), mapFunc);

    std::transform(transformedPointsLocal.begin(), transformedPointsLocal.end(),
                   transformedPointsLocal.begin(), mapFunc);

    QRectF dstBounds;
    Q_FOREACH (const QPointF &pt, transformedPointsLocal) {
        KisAlgebra2D::accumulateBounds(pt, &dstBounds);
    }

    const QRectF srcBounds(srcImageOffset, srcImage.size());
    dstBounds |= srcBounds;

    QPointF dstQImageOffset = dstBounds.topLeft();
    *newOffset = dstQImageOffset;

    QRect dstBoundsI = dstBounds.toAlignedRect();

    QImage dstImage(dstBoundsI.size(), srcImage.format());
    dstImage.fill(0);

    GridIterationTools::QImagePolygonOp polygonOp(srcImage, dstImage, srcImageOffset, dstQImageOffset);
    GridIterationTools::RegularGridIndexesOp indexesOp(m_d->gridSize);


    QRect correctSubGrid = GridIterationTools::calculateCorrectSubGrid(m_d->srcBounds, m_d->pixelPrecision, m_d->accumulatedBrushStrokes, m_d->gridSize);
    bool canMergeRects = GridIterationTools::canProcessRectsInRandomOrder(indexesOp, m_d->transformedPoints, correctSubGrid);
    polygonOp.setCanMergeRects(canMergeRects);


    GridIterationTools::iterateThroughGrid<GridIterationTools::AlwaysCompletePolygonPolicy>(polygonOp, indexesOp,
                                                    m_d->gridSize,
                                                    originalPointsLocal,
                                                    transformedPointsLocal,
                                                    correctSubGrid);


    QList<QRectF> areasToCopy = GridIterationTools::cutOutSubgridFromBounds(correctSubGrid, m_d->srcBounds, m_d->gridSize, m_d->originalPoints);
    polygonOp.setCanMergeRects(false);
    const qreal eps = 0.001;
    for (int i = 0; i < areasToCopy.length(); i++) {
        QPolygonF transformed = imageToThumbTransform.map(QPolygonF(areasToCopy[i]));
        if (KisAlgebra2D::isPolygonPixelAlignedRect(transformed, eps)) {
            polygonOp.fastCopyArea(transformed.boundingRect().toRect());
        } else {
            polygonOp.operator()(transformed, transformed);
        }
    }
    return dstImage;
}


void KisLiquifyTransformWorker::toXML(QDomElement *e) const
{
    QDomDocument doc = e->ownerDocument();
    QDomElement liqEl = doc.createElement("liquify_points");
    e->appendChild(liqEl);

    KisDomUtils::saveValue(&liqEl, "srcBounds", m_d->srcBounds);
    KisDomUtils::saveValue(&liqEl, "originalPoints", m_d->originalPoints);
    KisDomUtils::saveValue(&liqEl, "transformedPoints", m_d->transformedPoints);
    KisDomUtils::saveValue(&liqEl, "pixelPrecision", m_d->pixelPrecision);
    KisDomUtils::saveValue(&liqEl, "gridSize", m_d->gridSize);
}

KisLiquifyTransformWorker* KisLiquifyTransformWorker::fromXML(const QDomElement &e)
{
    QDomElement liquifyEl;

    QRect srcBounds;
    QVector<QPointF> originalPoints;
    QVector<QPointF> transformedPoints;
    int pixelPrecision;
    QSize gridSize;

    bool result = false;


    result =
        KisDomUtils::findOnlyElement(e, "liquify_points", &liquifyEl) &&

        KisDomUtils::loadValue(liquifyEl, "srcBounds", &srcBounds) &&
        KisDomUtils::loadValue(liquifyEl, "originalPoints", &originalPoints) &&
        KisDomUtils::loadValue(liquifyEl, "transformedPoints", &transformedPoints) &&
        KisDomUtils::loadValue(liquifyEl, "pixelPrecision", &pixelPrecision) &&
        KisDomUtils::loadValue(liquifyEl, "gridSize", &gridSize);

    if (!result) {
        warnKrita << "WARNING: Failed to load liquify worker from XML";
        return new KisLiquifyTransformWorker(QRect(0,0,1024, 1024), 0, 8);
    }

    KisLiquifyTransformWorker *worker =
        new KisLiquifyTransformWorker(srcBounds, 0, pixelPrecision);

    const int numPoints = originalPoints.size();

    if (numPoints != transformedPoints.size() ||
        numPoints != worker->m_d->originalPoints.size() ||
        gridSize != worker->m_d->gridSize) {
        warnKrita << "WARNING: Inconsistent number of points!";
        warnKrita << ppVar(originalPoints.size());
        warnKrita << ppVar(transformedPoints.size());
        warnKrita << ppVar(gridSize);
        warnKrita << ppVar(worker->m_d->originalPoints.size());
        warnKrita << ppVar(worker->m_d->transformedPoints.size());
        warnKrita << ppVar(worker->m_d->gridSize);

        return worker;
    }

    QRectF changedRect = QRectF();

    for (int i = 0; i < numPoints; i++) {
        worker->m_d->originalPoints[i] = originalPoints[i];
        worker->m_d->transformedPoints[i] = transformedPoints[i];
        if (!KisAlgebra2D::fuzzyPointCompare(transformedPoints[i], originalPoints[i])) {
            KisAlgebra2D::accumulateBounds(transformedPoints[i], &changedRect);
            KisAlgebra2D::accumulateBounds(originalPoints[i], &changedRect);
        }
    }
    changedRect = kisGrowRect(changedRect, pixelPrecision);

    worker->m_d->transformedPointsContainer.initializeWith(worker->m_d->transformedPoints);
    worker->m_d->originalPointsContainer.initializeWith(worker->m_d->originalPoints);

    worker->m_d->accumulatedBrushStrokes = changedRect;


    return worker;
}
