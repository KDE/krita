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

#include "kis_perspective_transform_strategy.h"

#include <QPointF>
#include <QPainter>
#include <QMatrix4x4>
#include <QVector2D>

#include <Eigen/Dense>

#include "kis_coordinates_converter.h"
#include "tool_transform_args.h"
#include "transform_transaction_properties.h"
#include "krita_utils.h"
#include "kis_cursor.h"
#include "kis_transform_utils.h"
#include "kis_free_transform_strategy_gsl_helpers.h"


enum StrokeFunction {
    DRAG_HANDLE = 0,
    DRAG_X_VANISHING_POINT,
    DRAG_Y_VANISHING_POINT,
    MOVE,
    NONE
};

struct KisPerspectiveTransformStrategy::Private
{
    Private(KisPerspectiveTransformStrategy *_q,
            const KisCoordinatesConverter *_converter,
            ToolTransformArgs &_currentArgs,
            TransformTransactionProperties &_transaction)
        : q(_q),
          converter(_converter),
          currentArgs(_currentArgs),
          transaction(_transaction),
          imageTooBig(false),
          isTransforming(false)
    {
    }

    KisPerspectiveTransformStrategy *q;

    /// standard members ///

    const KisCoordinatesConverter *converter;

    //////
    ToolTransformArgs &currentArgs;
    //////
    TransformTransactionProperties &transaction;


    QTransform thumbToImageTransform;
    QImage originalImage;

    QTransform paintingTransform;
    QPointF paintingOffset;

    QTransform handlesTransform;

    /// custom members ///

    StrokeFunction function;

    struct HandlePoints {
        bool xVanishingExists;
        bool yVanishingExists;

        QPointF xVanishing;
        QPointF yVanishing;
    };
    HandlePoints transformedHandles;

    QTransform transform;

    QVector<QPointF> srcCornerPoints;
    QVector<QPointF> dstCornerPoints;
    int currentDraggingCornerPoint;

    bool imageTooBig;

    QPointF clickPos;
    ToolTransformArgs clickArgs;
    bool isTransforming;

    QCursor getScaleCursor(const QPointF &handlePt);
    QCursor getShearCursor(const QPointF &start, const QPointF &end);
    void recalculateTransformations();
    void recalculateTransformedHandles();

    void transformIntoArgs(const Eigen::Matrix3f &t);
    QTransform transformFromArgs();
};

KisPerspectiveTransformStrategy::KisPerspectiveTransformStrategy(const KisCoordinatesConverter *converter,
                                                                 KoSnapGuide *snapGuide,
                                                   ToolTransformArgs &currentArgs,
                                                   TransformTransactionProperties &transaction)
    : KisSimplifiedActionPolicyStrategy(converter, snapGuide),
      m_d(new Private(this, converter, currentArgs, transaction))
{
}

KisPerspectiveTransformStrategy::~KisPerspectiveTransformStrategy()
{
}

void KisPerspectiveTransformStrategy::Private::recalculateTransformedHandles()
{
    srcCornerPoints.clear();
    srcCornerPoints << transaction.originalTopLeft();
    srcCornerPoints << transaction.originalTopRight();
    srcCornerPoints << transaction.originalBottomLeft();
    srcCornerPoints << transaction.originalBottomRight();

    dstCornerPoints.clear();
    Q_FOREACH (const QPointF &pt, srcCornerPoints) {
        dstCornerPoints << transform.map(pt);
    }

    QMatrix4x4 realMatrix(transform);
    QVector4D v;

    v = QVector4D(1, 0, 0, 0);
    v = realMatrix * v;
    transformedHandles.xVanishingExists = !qFuzzyCompare(v.w(), 0);
    transformedHandles.xVanishing = v.toVector2DAffine().toPointF();

    v = QVector4D(0, 1, 0, 0);
    v = realMatrix * v;
    transformedHandles.yVanishingExists = !qFuzzyCompare(v.w(), 0);
    transformedHandles.yVanishing = v.toVector2DAffine().toPointF();
}

void KisPerspectiveTransformStrategy::setTransformFunction(const QPointF &mousePos, bool perspectiveModifierActive)
{
    Q_UNUSED(perspectiveModifierActive);

    QPolygonF transformedPolygon = m_d->transform.map(QPolygonF(m_d->transaction.originalRect()));
    StrokeFunction defaultFunction = transformedPolygon.containsPoint(mousePos, Qt::OddEvenFill) ? MOVE : NONE;
    KisTransformUtils::HandleChooser<StrokeFunction>
        handleChooser(mousePos, defaultFunction);

    qreal handleRadius = KisTransformUtils::effectiveHandleGrabRadius(m_d->converter);

    if (!m_d->transformedHandles.xVanishing.isNull()) {
        handleChooser.addFunction(m_d->transformedHandles.xVanishing,
                                  handleRadius, DRAG_X_VANISHING_POINT);
    }

    if (!m_d->transformedHandles.yVanishing.isNull()) {
        handleChooser.addFunction(m_d->transformedHandles.yVanishing,
                                  handleRadius, DRAG_Y_VANISHING_POINT);
    }

    m_d->currentDraggingCornerPoint = -1;
    for (int i = 0; i < m_d->dstCornerPoints.size(); i++) {
        if (handleChooser.addFunction(m_d->dstCornerPoints[i],
                                      handleRadius, DRAG_HANDLE)) {

            m_d->currentDraggingCornerPoint = i;
        }
    }

    m_d->function = handleChooser.function();
}

QCursor KisPerspectiveTransformStrategy::getCurrentCursor() const
{
    QCursor cursor;

    switch (m_d->function) {
    case NONE:
        cursor = KisCursor::arrowCursor();
        break;
    case MOVE:
        cursor = KisCursor::moveCursor();
        break;
    case DRAG_HANDLE:
    case DRAG_X_VANISHING_POINT:
    case DRAG_Y_VANISHING_POINT:
        cursor = KisCursor::pointingHandCursor();
        break;
    }

    return cursor;
}

void KisPerspectiveTransformStrategy::paint(QPainter &gc)
{
    gc.save();

    gc.setOpacity(m_d->transaction.basePreviewOpacity());
    gc.setTransform(m_d->paintingTransform, true);
    gc.drawImage(m_d->paintingOffset, originalImage());

    gc.restore();

    // Draw Handles
    QPainterPath handles;

    handles.moveTo(m_d->transaction.originalTopLeft());
    handles.lineTo(m_d->transaction.originalTopRight());
    handles.lineTo(m_d->transaction.originalBottomRight());
    handles.lineTo(m_d->transaction.originalBottomLeft());
    handles.lineTo(m_d->transaction.originalTopLeft());


    auto addHandleRectFunc =
        [&](const QPointF &pt) {
            handles.addRect(
                KisTransformUtils::handleRect(KisTransformUtils::handleVisualRadius,
                                              m_d->handlesTransform,
                                              m_d->transaction.originalRect(), pt)
                .translated(pt));
    };

    addHandleRectFunc(m_d->transaction.originalTopLeft());
    addHandleRectFunc(m_d->transaction.originalTopRight());
    addHandleRectFunc(m_d->transaction.originalBottomLeft());
    addHandleRectFunc(m_d->transaction.originalBottomRight());

    gc.save();

    if (m_d->isTransforming) {
        gc.setOpacity(0.1);
    }

    /**
     * WARNING: we cannot install a transform to paint the handles here!
     *
     * There is a bug in Qt that prevents painting of cosmetic-pen
     * brushes in openGL mode when a TxProject matrix is active on
     * a QPainter. So just convert it manually.
     *
     * https://bugreports.qt-project.org/browse/QTBUG-42658
     */

    //gc.setTransform(m_d->handlesTransform, true); <-- don't do like this!

    QPainterPath mappedHandles = m_d->handlesTransform.map(handles);

    QPen pen[2];
    pen[0].setWidth(1);
    pen[1].setWidth(2);
    pen[1].setColor(Qt::lightGray);

    for (int i = 1; i >= 0; --i) {
        gc.setPen(pen[i]);
        gc.drawPath(mappedHandles);
    }

    gc.restore();

    { // painting perspective handles
        QPainterPath perspectiveHandles;

        QRectF handleRect =
            KisTransformUtils::handleRect(KisTransformUtils::handleVisualRadius,
                                          QTransform(),
                                          m_d->transaction.originalRect(), 0, 0);

        if (m_d->transformedHandles.xVanishingExists) {
            QRectF rc = handleRect.translated(m_d->transformedHandles.xVanishing);
            perspectiveHandles.addEllipse(rc);
        }

        if (m_d->transformedHandles.yVanishingExists) {
            QRectF rc = handleRect.translated(m_d->transformedHandles.yVanishing);
            perspectiveHandles.addEllipse(rc);
        }

        if (!perspectiveHandles.isEmpty()) {
            gc.save();
            gc.setTransform(m_d->converter->imageToWidgetTransform());

            gc.setBrush(Qt::red);

            for (int i = 1; i >= 0; --i) {
                gc.setPen(pen[i]);
                gc.drawPath(perspectiveHandles);
            }

            gc.restore();
        }
    }
}

void KisPerspectiveTransformStrategy::externalConfigChanged()
{
    m_d->recalculateTransformations();
}

bool KisPerspectiveTransformStrategy::beginPrimaryAction(const QPointF &pt)
{
    Q_UNUSED(pt);

    if (m_d->function == NONE) return false;

    m_d->clickPos = pt;
    m_d->clickArgs = m_d->currentArgs;

    return true;
}

Eigen::Matrix3f getTransitionMatrix(const QVector<QPointF> &sp)
{
    Eigen::Matrix3f A;
    Eigen::Vector3f v3;

    A << sp[0].x() , sp[1].x() , sp[2].x()
        ,sp[0].y() , sp[1].y() , sp[2].y()
        ,      1   ,       1   ,       1;

    v3 << sp[3].x() , sp[3].y() , 1;

    Eigen::Vector3f coeffs = A.colPivHouseholderQr().solve(v3);

    A.col(0) *= coeffs(0);
    A.col(1) *= coeffs(1);
    A.col(2) *= coeffs(2);

    return A;
}

QTransform toQTransform(const Eigen::Matrix3f &m)
{
    return QTransform(m(0,0), m(1,0), m(2,0),
                      m(0,1), m(1,1), m(2,1),
                      m(0,2), m(1,2), m(2,2));
}

Eigen::Matrix3f fromQTransform(const QTransform &t)
{
    Eigen::Matrix3f m;

    m << t.m11() , t.m21() , t.m31()
        ,t.m12() , t.m22() , t.m32()
        ,t.m13() , t.m23() , t.m33();

    return m;
}

Eigen::Matrix3f fromTranslate(const QPointF &pt)
{
    Eigen::Matrix3f m;

    m << 1 , 0 , pt.x()
        ,0 , 1 , pt.y()
        ,0 , 0 , 1;

    return m;
}

Eigen::Matrix3f fromScale(qreal sx, qreal sy)
{
    Eigen::Matrix3f m;

    m << sx , 0 , 0
        ,0 , sy , 0
        ,0 , 0 , 1;

    return m;
}

Eigen::Matrix3f fromShear(qreal sx, qreal sy)
{
    Eigen::Matrix3f m;

    m << 1 , sx , 0
        ,sy , sx*sy + 1, 0
        ,0 , 0 , 1;

    return m;
}

void KisPerspectiveTransformStrategy::Private::transformIntoArgs(const Eigen::Matrix3f &t)
{
    Eigen::Matrix3f TS = fromTranslate(-currentArgs.originalCenter());

    Eigen::Matrix3f m = t * TS.inverse();

    qreal tX = m(0,2) / m(2,2);
    qreal tY = m(1,2) / m(2,2);

    Eigen::Matrix3f T = fromTranslate(QPointF(tX, tY));

    m = T.inverse() * m;

    // TODO: implement matrix decomposition as described here
    // https://www.w3.org/TR/css-transforms-1/#decomposing-a-3d-matrix

    // For now use an extremely hackish approximation
    if (m(0,1) != 0.0 && m(0,0) != 0.0 && m(2,2) != 0.0) {

        const qreal factor = (m(1,1) / m(0,1) - m(1,0) / m(0,0));

        qreal scaleX = m(0,0) / m(2,2);
        qreal scaleY = m(0,1) / m(2,2) * factor;

        Eigen::Matrix3f SC = fromScale(scaleX, scaleY);

        qreal shearX = 1.0 / factor;
        qreal shearY = m(1,0) / m(0,0);

        Eigen::Matrix3f S = fromShear(shearX, shearY);

        currentArgs.setScaleX(scaleX);
        currentArgs.setScaleY(scaleY);

        currentArgs.setShearX(shearX);
        currentArgs.setShearY(shearY);

        m = m * SC.inverse();
        m = m * S.inverse();
        m /= m(2,2);
    } else {
        currentArgs.setScaleX(1.0);
        currentArgs.setScaleY(1.0);

        currentArgs.setShearX(0.0);
        currentArgs.setShearY(0.0);
    }

    currentArgs.setTransformedCenter(QPointF(tX, tY));
    currentArgs.setFlattenedPerspectiveTransform(toQTransform(m));
}

QTransform KisPerspectiveTransformStrategy::Private::transformFromArgs()
{
    KisTransformUtils::MatricesPack m(currentArgs);
    return m.finalTransform();
}

QVector4D fromQPointF(const QPointF &pt) {
    return QVector4D(pt.x(), pt.y(), 0, 1.0);
}

QPointF toQPointF(const QVector4D &v) {
    return v.toVector2DAffine().toPointF();
}

void KisPerspectiveTransformStrategy::continuePrimaryAction(const QPointF &mousePos, bool shiftModifierActve, bool altModifierActive)
{
    Q_UNUSED(shiftModifierActve);
    Q_UNUSED(altModifierActive);

    m_d->isTransforming = true;

    switch (m_d->function) {
    case NONE:
        break;
    case MOVE: {
        QPointF diff = mousePos - m_d->clickPos;
        m_d->currentArgs.setTransformedCenter(
            m_d->clickArgs.transformedCenter() + diff);
        break;
    }
    case DRAG_HANDLE: {
        KIS_ASSERT_RECOVER_RETURN(m_d->currentDraggingCornerPoint >=0);
        m_d->dstCornerPoints[m_d->currentDraggingCornerPoint] = mousePos;

        Eigen::Matrix3f A = getTransitionMatrix(m_d->srcCornerPoints);
        Eigen::Matrix3f B = getTransitionMatrix(m_d->dstCornerPoints);
        Eigen::Matrix3f result = B * A.inverse();

        m_d->transformIntoArgs(result);

        break;
    }
    case DRAG_X_VANISHING_POINT:
    case DRAG_Y_VANISHING_POINT: {

        QMatrix4x4 m(m_d->transform);

        QPointF tl = m_d->transaction.originalTopLeft();
        QPointF tr = m_d->transaction.originalTopRight();
        QPointF bl = m_d->transaction.originalBottomLeft();
        QPointF br = m_d->transaction.originalBottomRight();

        QVector4D v(1,0,0,0);
        QVector4D otherV(0,1,0,0);

        if (m_d->function == DRAG_X_VANISHING_POINT) {
            v = QVector4D(1,0,0,0);
            otherV = QVector4D(0,1,0,0);
        } else {
            v = QVector4D(0,1,0,0);
            otherV = QVector4D(1,0,0,0);
        }

        QPointF tl_dst = toQPointF(m * fromQPointF(tl));
        QPointF tr_dst = toQPointF(m * fromQPointF(tr));
        QPointF bl_dst = toQPointF(m * fromQPointF(bl));
        QPointF br_dst = toQPointF(m * fromQPointF(br));
        QPointF v_dst = toQPointF(m * v);
        QPointF otherV_dst = toQPointF(m * otherV);

        QVector<QPointF> srcPoints;
        QVector<QPointF> dstPoints;

        QPointF far1_src;
        QPointF far2_src;
        QPointF near1_src;
        QPointF near2_src;

        QPointF far1_dst;
        QPointF far2_dst;
        QPointF near1_dst;
        QPointF near2_dst;

        if (m_d->function == DRAG_X_VANISHING_POINT) {

            // topLeft (far) --- topRight (near) --- vanishing
            if (kisSquareDistance(v_dst, tl_dst) > kisSquareDistance(v_dst, tr_dst)) {
                far1_src = tl;
                far2_src = bl;
                near1_src = tr;
                near2_src = br;

                far1_dst = tl_dst;
                far2_dst = bl_dst;
                near1_dst = tr_dst;
                near2_dst = br_dst;

                // topRight (far) --- topLeft (near) --- vanishing
            } else {
                far1_src = tr;
                far2_src = br;
                near1_src = tl;
                near2_src = bl;

                far1_dst = tr_dst;
                far2_dst = br_dst;
                near1_dst = tl_dst;
                near2_dst = bl_dst;
            }

        } else /* if (m_d->function == DRAG_Y_VANISHING_POINT) */{
            // topLeft (far) --- bottomLeft (near) --- vanishing
            if (kisSquareDistance(v_dst, tl_dst) > kisSquareDistance(v_dst, bl_dst)) {
                far1_src = tl;
                far2_src = tr;
                near1_src = bl;
                near2_src = br;

                far1_dst = tl_dst;
                far2_dst = tr_dst;
                near1_dst = bl_dst;
                near2_dst = br_dst;

                // bottomLeft (far) --- topLeft (near) --- vanishing
            } else {
                far1_src = bl;
                far2_src = br;
                near1_src = tl;
                near2_src = tr;

                far1_dst = bl_dst;
                far2_dst = br_dst;
                near1_dst = tl_dst;
                near2_dst = tr_dst;
            }
        }

        QLineF l0(far1_dst, mousePos);
        QLineF l1(far2_dst, mousePos);
        QLineF l2(otherV_dst, near1_dst);
        l0.intersect(l2, &near1_dst);
        l1.intersect(l2, &near2_dst);

        srcPoints << far1_src;
        srcPoints << far2_src;
        srcPoints << near1_src;
        srcPoints << near2_src;

        dstPoints << far1_dst;
        dstPoints << far2_dst;
        dstPoints << near1_dst;
        dstPoints << near2_dst;

        Eigen::Matrix3f A = getTransitionMatrix(srcPoints);
        Eigen::Matrix3f B = getTransitionMatrix(dstPoints);
        Eigen::Matrix3f result = B * A.inverse();

        m_d->transformIntoArgs(result);
        break;
    }
    }

    m_d->recalculateTransformations();
}

bool KisPerspectiveTransformStrategy::endPrimaryAction()
{
    bool shouldSave = !m_d->imageTooBig;
    m_d->isTransforming = false;

    if (m_d->imageTooBig) {
        m_d->currentArgs = m_d->clickArgs;
        m_d->recalculateTransformations();
    }

    return shouldSave;
}

void KisPerspectiveTransformStrategy::Private::recalculateTransformations()
{
    transform = transformFromArgs();

    QTransform viewScaleTransform = converter->imageToDocumentTransform() * converter->documentToFlakeTransform();
    handlesTransform = transform * viewScaleTransform;

    QTransform tl = QTransform::fromTranslate(transaction.originalTopLeft().x(), transaction.originalTopLeft().y());
    paintingTransform = tl.inverted() * q->thumbToImageTransform() * tl * transform * viewScaleTransform;
    paintingOffset = transaction.originalTopLeft();

    // check whether image is too big to be displayed or not
    const qreal maxScale = 20.0;

    imageTooBig = false;

    if (qAbs(currentArgs.scaleX()) > maxScale ||
        qAbs(currentArgs.scaleY()) > maxScale) {

        imageTooBig = true;

    } else {
        QVector<QPointF> points;
        points << transaction.originalRect().topLeft();
        points << transaction.originalRect().topRight();
        points << transaction.originalRect().bottomRight();
        points << transaction.originalRect().bottomLeft();

        for (int i = 0; i < points.size(); i++) {
            points[i] = transform.map(points[i]);
        }

        for (int i = 0; i < points.size(); i++) {
            const QPointF &pt = points[i];
            const QPointF &prev = points[(i - 1 + 4) % 4];
            const QPointF &next = points[(i + 1) % 4];
            const QPointF &other = points[(i + 2) % 4];

            QLineF l1(pt, other);
            QLineF l2(prev, next);

            QPointF intersection;
            l1.intersect(l2, &intersection);

            qreal maxDistance = kisSquareDistance(pt, other);

            if (kisSquareDistance(pt, intersection) > maxDistance ||
                kisSquareDistance(other, intersection) > maxDistance) {

                imageTooBig = true;
                break;
            }

            const qreal thresholdDistance = 0.02 * l2.length();

            if (kisDistanceToLine(pt, l2) < thresholdDistance) {
                imageTooBig = true;
                break;
            }
        }
    }

    // recalculate cached handles position
    recalculateTransformedHandles();

    emit q->requestShowImageTooBig(imageTooBig);
}
