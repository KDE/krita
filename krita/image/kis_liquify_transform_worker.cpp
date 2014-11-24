/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_liquify_transform_worker.h"

#include "kis_grid_interpolation_tools.h"
#include "kis_dom_utils.h"


struct KisLiquifyTransformWorker::Private
{
    Private(const QRect &_srcBounds,
            KoUpdater *_progress,
            int _pixelPrecision)
        : srcBounds(_srcBounds),
          progress(_progress),
          pixelPrecision(_pixelPrecision)
    {
    }

    const QRect srcBounds;

    QVector<QPointF> originalPoints;
    QVector<QPointF> transformedPoints;

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
    return
        m_d->srcBounds == other.m_d->srcBounds &&
        m_d->originalPoints == other.m_d->originalPoints &&
        m_d->transformedPoints == other.m_d->transformedPoints &&
        m_d->pixelPrecision == other.m_d->pixelPrecision &&
        m_d->gridSize == other.m_d->gridSize;
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
}

void KisLiquifyTransformWorker::undoPoints(const QPointF &base,
                                           qreal amount,
                                           qreal sigma)
{
    const qreal maxDistCoeff = 3.0;
    const qreal maxDist = maxDistCoeff * sigma;
    QRectF clipRect(base.x() - maxDist, base.y() - maxDist,
                    2 * maxDist, 2 * maxDist);

    QVector<QPointF>::iterator it = m_d->transformedPoints.begin();
    QVector<QPointF>::iterator end = m_d->transformedPoints.end();

    QVector<QPointF>::iterator refIt = m_d->originalPoints.begin();
    KIS_ASSERT_RECOVER_RETURN(m_d->originalPoints.size() ==
                              m_d->transformedPoints.size());

    for (; it != end; ++it, ++refIt) {
        if (!clipRect.contains(*it)) continue;

        QPointF diff = *it - base;
        qreal dist = KisAlgebra2D::norm(diff);
        if (dist > maxDist) continue;

        qreal lambda = exp(-0.5 * pow2(dist / sigma));
        lambda *= amount;
        *it = *refIt * lambda + *it * (1.0 - lambda);
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

    QVector<QPointF>::iterator it = transformedPoints.begin();
    QVector<QPointF>::iterator end = transformedPoints.end();

    for (; it != end; ++it) {
        if (!clipRect.contains(*it)) continue;

        QPointF diff = *it - base;
        qreal dist = KisAlgebra2D::norm(diff);
        if (dist > maxDist) continue;

        const qreal lambda = exp(-0.5 * pow2(dist / sigma));
        *it = op(*it, base, diff, lambda);
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

    QVector<QPointF>::iterator it = transformedPoints.begin();
    QVector<QPointF>::iterator end = transformedPoints.end();

    QVector<QPointF>::iterator refIt = originalPoints.begin();
    KIS_ASSERT_RECOVER_RETURN(originalPoints.size() ==
                              transformedPoints.size());

    for (; it != end; ++it, ++refIt) {
        if (!clipRect.contains(*it)) continue;

        QPointF diff = *refIt - base;
        qreal dist = KisAlgebra2D::norm(diff);
        if (dist > maxDist) continue;

        const qreal lambda = exp(-0.5 * pow2(dist / sigma));
        QPointF dstPt = op(*refIt, base, diff, lambda);

        if (kisDistance(dstPt, *refIt) > kisDistance(*it, *refIt)) {
            *it = (1.0 - flow) * (*it) + flow * dstPt;
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

struct KisLiquifyTransformWorker::Private::MapIndexesOp {

    MapIndexesOp(KisLiquifyTransformWorker::Private *d)
        : m_d(d)
    {
    }

    inline QVector<int> calculateMappedIndexes(int col, int row,
                                               int *numExistingPoints) const {

        *numExistingPoints = 4;
        QVector<int> cellIndexes =
            GridIterationTools::calculateCellIndexes(col, row, m_d->gridSize);

        return cellIndexes;
    }

    inline int tryGetValidIndex(const QPoint &cellPt) const {
        Q_UNUSED(cellPt);

        KIS_ASSERT_RECOVER_NOOP(0 && "Not applicable");
        return -1;
    }

    inline QPointF getSrcPointForce(const QPoint &cellPt) const {
        Q_UNUSED(cellPt);

        KIS_ASSERT_RECOVER_NOOP(0 && "Not applicable");
        return QPointF();
    }

    inline const QPolygonF srcCropPolygon() const {
        KIS_ASSERT_RECOVER_NOOP(0 && "Not applicable");
        return QPolygonF();
    }

    KisLiquifyTransformWorker::Private *m_d;
};


void KisLiquifyTransformWorker::run(KisPaintDeviceSP device)
{
    KisPaintDeviceSP srcDev = new KisPaintDevice(*device.data());
    device->clear();

    using namespace GridIterationTools;

    PaintDevicePolygonOp polygonOp(srcDev, device);
    Private::MapIndexesOp indexesOp(m_d.data());
    iterateThroughGrid<AlwaysCompletePolygonPolicy>(polygonOp, indexesOp,
                                                    m_d->gridSize,
                                                    m_d->originalPoints,
                                                    m_d->transformedPoints);
}

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <QTransform>

typedef boost::function<QPointF (const QPointF&)> PointMapFunction;

PointMapFunction bindPointMapTransform(const QTransform &transform) {
    typedef QPointF (QTransform::*MapFuncType)(const QPointF&) const;
    return boost::bind(static_cast<MapFuncType>(&QTransform::map), &transform, _1);
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
    foreach (const QPointF &pt, transformedPointsLocal) {
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
    Private::MapIndexesOp indexesOp(m_d.data());
    GridIterationTools::iterateThroughGrid
        <GridIterationTools::AlwaysCompletePolygonPolicy>(polygonOp, indexesOp,
                                                          m_d->gridSize,
                                                          originalPointsLocal,
                                                          transformedPointsLocal);
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
        qWarning() << "WARNING: Failed to load liquify worker from XML";
        return new KisLiquifyTransformWorker(QRect(0,0,1024, 1024), 0, 8);
    }

    KisLiquifyTransformWorker *worker =
        new KisLiquifyTransformWorker(srcBounds, 0, pixelPrecision);

    const int numPoints = originalPoints.size();

    if (numPoints != transformedPoints.size() ||
        numPoints != worker->m_d->originalPoints.size() ||
        gridSize != worker->m_d->gridSize) {
        qWarning() << "WARNING: Inconsistent number of points!";
        qWarning() << ppVar(originalPoints.size());
        qWarning() << ppVar(transformedPoints.size());
        qWarning() << ppVar(gridSize);
        qWarning() << ppVar(worker->m_d->originalPoints.size());
        qWarning() << ppVar(worker->m_d->transformedPoints.size());
        qWarning() << ppVar(worker->m_d->gridSize);

        return worker;
    }

    for (int i = 0; i < numPoints; i++) {
        worker->m_d->originalPoints[i] = originalPoints[i];
        worker->m_d->transformedPoints[i] = transformedPoints[i];
    }


    return worker;
}
