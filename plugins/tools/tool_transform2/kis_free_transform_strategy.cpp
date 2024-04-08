/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_free_transform_strategy.h"

#include <QPointF>
#include <QPainter>
#include <QPainterPath>
#include <QMatrix4x4>

#include <KoResourcePaths.h>

#include "kis_coordinates_converter.h"
#include "tool_transform_args.h"
#include "transform_transaction_properties.h"
#include "krita_utils.h"
#include "kis_cursor.h"
#include "kis_transform_utils.h"
#include "kis_free_transform_strategy_gsl_helpers.h"
#include "kis_algebra_2d.h"


namespace {
enum StrokeFunction {
    ROTATE = 0,
    MOVE,
    RIGHTSCALE,
    TOPRIGHTSCALE,
    TOPSCALE,
    TOPLEFTSCALE,
    LEFTSCALE,
    BOTTOMLEFTSCALE,
    BOTTOMSCALE,
    BOTTOMRIGHTSCALE,
    BOTTOMSHEAR,
    RIGHTSHEAR,
    TOPSHEAR,
    LEFTSHEAR,
    MOVECENTER,
    PERSPECTIVE,
    ROTATEBOUNDS
};
}

struct KisFreeTransformStrategy::Private
{
    Private(KisFreeTransformStrategy *_q,
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
        scaleCursors[0] = KisCursor::sizeHorCursor();
        scaleCursors[1] = KisCursor::sizeFDiagCursor();
        scaleCursors[2] = KisCursor::sizeVerCursor();
        scaleCursors[3] = KisCursor::sizeBDiagCursor();
        scaleCursors[4] = KisCursor::sizeHorCursor();
        scaleCursors[5] = KisCursor::sizeFDiagCursor();
        scaleCursors[6] = KisCursor::sizeVerCursor();
        scaleCursors[7] = KisCursor::sizeBDiagCursor();

        shearCursorPixmap.load(":/shear_cursor.png");
    }

    KisFreeTransformStrategy *q;

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

    StrokeFunction function {MOVE};

    struct HandlePoints {
        QPointF topLeft;
        QPointF topMiddle;
        QPointF topRight;

        QPointF middleLeft;
        QPointF rotationCenter;
        QPointF middleRight;

        QPointF bottomLeft;
        QPointF bottomMiddle;
        QPointF bottomRight;
    };
    HandlePoints transformedHandles;
    HandlePoints rotatedHandles;

    QTransform transform;

    QCursor scaleCursors[8]; // cursors for the 8 directions
    QPixmap shearCursorPixmap;

    bool imageTooBig {false};

    ToolTransformArgs clickArgs;
    QPointF clickPos;
    QTransform clickTransform;

    bool isTransforming {false};

    QCursor getScaleCursor(const QPointF &handlePt);
    QCursor getShearCursor(const QPointF &start, const QPointF &end);
    void recalculateTransformations();
    void recalculateTransformedHandles();
    void recalculateRotatedHandles();
};

KisFreeTransformStrategy::KisFreeTransformStrategy(const KisCoordinatesConverter *converter,
                                                   KoSnapGuide *snapGuide,
                                                   ToolTransformArgs &currentArgs,
                                                   TransformTransactionProperties &transaction)
    : KisSimplifiedActionPolicyStrategy(converter, snapGuide),
      m_d(new Private(this, converter, currentArgs, transaction))
{
}

KisFreeTransformStrategy::~KisFreeTransformStrategy()
{
}

void KisFreeTransformStrategy::Private::recalculateRotatedHandles()
{
    QTransform BR;
    BR.rotateRadians(currentArgs.boundsRotation());

    QPointF axisX = BR.map(QPointF(1.0, 0.0));
    QPointF axisY = BR.map(QPointF(0.0, 1.0));
    QPointF rotationCenter = currentArgs.originalCenter() + currentArgs.rotationCenterOffset();

    QList<QPointF> corners;
    corners << transaction.originalTopLeft() << transaction.originalTopRight() << transaction.originalBottomLeft() << transaction.originalBottomRight();
    qreal maxX = -std::numeric_limits<qreal>::infinity();
    qreal minX = std::numeric_limits<qreal>::infinity();
    qreal maxY = -std::numeric_limits<qreal>::infinity();
    qreal minY = std::numeric_limits<qreal>::infinity();
    for (const QPointF &p : corners) {
        qreal dx = QPointF::dotProduct(axisX, p);
        qreal dy = QPointF::dotProduct(axisY, p);
        maxX = std::max(maxX, dx);
        minX = std::min(minX, dx);
        maxY = std::max(maxY, dy);
        minY = std::min(minY, dy);
    }

    rotatedHandles.topLeft = BR.map(QPointF(minX, minY));
    rotatedHandles.topRight = BR.map(QPointF(maxX, minY));
    rotatedHandles.bottomLeft = BR.map(QPointF(minX, maxY));
    rotatedHandles.bottomRight = BR.map(QPointF(maxX, maxY));

    rotatedHandles.topMiddle = (rotatedHandles.topLeft + rotatedHandles.topRight) * 0.5;
    rotatedHandles.bottomMiddle = (rotatedHandles.bottomLeft + rotatedHandles.bottomRight) * 0.5;

    rotatedHandles.middleLeft = (rotatedHandles.topLeft + rotatedHandles.bottomLeft) * 0.5;
    rotatedHandles.middleRight = (rotatedHandles.topRight + rotatedHandles.bottomRight) * 0.5;

    rotatedHandles.rotationCenter = rotationCenter;

}

void KisFreeTransformStrategy::Private::recalculateTransformedHandles()
{
    recalculateRotatedHandles();
    transformedHandles.topLeft = transform.map(rotatedHandles.topLeft);
    transformedHandles.topMiddle = transform.map(rotatedHandles.topMiddle);
    transformedHandles.topRight = transform.map(rotatedHandles.topRight);

    transformedHandles.middleLeft = transform.map(rotatedHandles.middleLeft);
    transformedHandles.rotationCenter = transform.map(rotatedHandles.rotationCenter);
    transformedHandles.middleRight = transform.map(rotatedHandles.middleRight);

    transformedHandles.bottomLeft = transform.map(rotatedHandles.bottomLeft);
    transformedHandles.bottomMiddle = transform.map(rotatedHandles.bottomMiddle);
    transformedHandles.bottomRight = transform.map(rotatedHandles.bottomRight);
}

bool overlapsThickLine(QPointF lineStart, QPointF lineEnd, QPointF p, double lineHalfWidth) {
    QPointF lineVector = lineEnd - lineStart;
    double d = QPointF::dotProduct(p - lineStart, lineVector) / QPointF::dotProduct(lineVector, lineVector);
    if (d < 0.0 || d > 1.0) {
        return false;
    }
    QPointF nearestPoint = lineStart + d * lineVector;
    QPointF diff = p - nearestPoint;
    return QPointF::dotProduct(diff, diff) < lineHalfWidth * lineHalfWidth;
}

void KisFreeTransformStrategy::setTransformFunction(const QPointF &mousePos, bool perspectiveModifierActive, bool shiftModifierActive, bool altModifierActive)
{
    // Q_UNUSED(shiftModifierActive);
    Q_UNUSED(altModifierActive);
    //printf("Ctrl: %i, Shift: %i, Alt: %i\n", perspectiveModifierActive, shiftModifierActive, altModifierActive);

    if (perspectiveModifierActive && !m_d->transaction.shouldAvoidPerspectiveTransform()) {
        m_d->function = PERSPECTIVE;
        return;
    }

    QPolygonF transformedPolygon;
    transformedPolygon << m_d->transformedHandles.topLeft << m_d->transformedHandles.topRight
        << m_d->transformedHandles.bottomRight << m_d->transformedHandles.bottomLeft << m_d->transformedHandles.topLeft;
    qreal handleRadius = KisTransformUtils::effectiveHandleGrabRadius(m_d->converter);
    qreal rotationHandleRadius = KisTransformUtils::effectiveHandleGrabRadius(m_d->converter);


    StrokeFunction defaultFunction =
        transformedPolygon.containsPoint(mousePos, Qt::OddEvenFill) ? MOVE : (shiftModifierActive ? ROTATEBOUNDS : ROTATE);
    KisTransformUtils::HandleChooser<StrokeFunction>
        handleChooser(mousePos, defaultFunction);

    handleChooser.addFunction(m_d->transformedHandles.topMiddle,
                              handleRadius, TOPSCALE);
    handleChooser.addFunction(m_d->transformedHandles.topRight,
                              handleRadius, TOPRIGHTSCALE);
    handleChooser.addFunction(m_d->transformedHandles.middleRight,
                              handleRadius, RIGHTSCALE);

    handleChooser.addFunction(m_d->transformedHandles.bottomRight,
                              handleRadius, BOTTOMRIGHTSCALE);
    handleChooser.addFunction(m_d->transformedHandles.bottomMiddle,
                              handleRadius, BOTTOMSCALE);
    handleChooser.addFunction(m_d->transformedHandles.bottomLeft,
                              handleRadius, BOTTOMLEFTSCALE);
    handleChooser.addFunction(m_d->transformedHandles.middleLeft,
                              handleRadius, LEFTSCALE);
    handleChooser.addFunction(m_d->transformedHandles.topLeft,
                              handleRadius, TOPLEFTSCALE);
    handleChooser.addFunction(m_d->transformedHandles.rotationCenter,
                              rotationHandleRadius, MOVECENTER);

    m_d->function = handleChooser.function();

    if (m_d->function == ROTATE || m_d->function == MOVE) {
        QPointF t = m_d->transform.inverted().map(mousePos);

        if (overlapsThickLine(m_d->rotatedHandles.topLeft, m_d->rotatedHandles.topRight, t, handleRadius)) {
            m_d->function = TOPSHEAR;
        } else if (overlapsThickLine(m_d->rotatedHandles.bottomLeft, m_d->rotatedHandles.bottomRight, t, handleRadius)) {
            m_d->function = BOTTOMSHEAR;
        } else if (overlapsThickLine(m_d->rotatedHandles.bottomLeft, m_d->rotatedHandles.topLeft, t, handleRadius)) {
            m_d->function = LEFTSHEAR;
        } else if (overlapsThickLine(m_d->rotatedHandles.bottomRight, m_d->rotatedHandles.topRight, t, handleRadius)) {
            m_d->function = RIGHTSHEAR;
        }
    }
}

bool KisFreeTransformStrategy::shiftModifierIsUsed() const
{
    return true;
}

QCursor KisFreeTransformStrategy::Private::getScaleCursor(const QPointF &handlePt)
{
    QPointF handlePtInWidget = converter->imageToWidget(handlePt);
    QPointF centerPtInWidget = converter->imageToWidget(currentArgs.transformedCenter());

    QPointF direction = handlePtInWidget - centerPtInWidget;
    qreal angle = atan2(direction.y(), direction.x());
    angle = normalizeAngle(angle);

    int octant = qRound(angle * 4. / M_PI) % 8;
    return scaleCursors[octant];
}

QCursor KisFreeTransformStrategy::Private::getShearCursor(const QPointF &start, const QPointF &end)
{
    QPointF startPtInWidget = converter->imageToWidget(start);
    QPointF endPtInWidget = converter->imageToWidget(end);
    QPointF direction = endPtInWidget - startPtInWidget;

    qreal angle = atan2(-direction.y(), direction.x());
    return QCursor(shearCursorPixmap.transformed(QTransform().rotateRadians(-angle)));
}

QCursor KisFreeTransformStrategy::getCurrentCursor() const
{
    QCursor cursor;

    switch (m_d->function) {
    case MOVE:
        cursor = KisCursor::moveCursor();
        break;
    case ROTATEBOUNDS:
        cursor = KisCursor::rotateCursor();
        break;
    case ROTATE:
        cursor = KisCursor::rotateCursor();
        break;
    case PERSPECTIVE:
        //TODO: find another cursor for perspective
        cursor = KisCursor::rotateCursor();
        break;
    case RIGHTSCALE:
        cursor = m_d->getScaleCursor(m_d->transformedHandles.middleRight);
        break;
    case TOPSCALE:
        cursor = m_d->getScaleCursor(m_d->transformedHandles.topMiddle);
        break;
    case LEFTSCALE:
        cursor = m_d->getScaleCursor(m_d->transformedHandles.middleLeft);
        break;
    case BOTTOMSCALE:
        cursor = m_d->getScaleCursor(m_d->transformedHandles.bottomMiddle);
        break;
    case TOPRIGHTSCALE:
        cursor = m_d->getScaleCursor(m_d->transformedHandles.topRight);
        break;
    case BOTTOMLEFTSCALE:
        cursor = m_d->getScaleCursor(m_d->transformedHandles.bottomLeft);
        break;
    case TOPLEFTSCALE:
        cursor = m_d->getScaleCursor(m_d->transformedHandles.topLeft);
        break;
    case BOTTOMRIGHTSCALE:
        cursor = m_d->getScaleCursor(m_d->transformedHandles.bottomRight);
        break;
    case MOVECENTER:
        cursor = KisCursor::handCursor();
        break;
    case BOTTOMSHEAR:
        cursor = m_d->getShearCursor(m_d->transformedHandles.bottomLeft, m_d->transformedHandles.bottomRight);
        break;
    case RIGHTSHEAR:
        cursor = m_d->getShearCursor(m_d->transformedHandles.bottomRight, m_d->transformedHandles.topRight);
        break;
    case TOPSHEAR:
        cursor = m_d->getShearCursor(m_d->transformedHandles.topRight, m_d->transformedHandles.topLeft);
        break;
    case LEFTSHEAR:
        cursor = m_d->getShearCursor(m_d->transformedHandles.topLeft, m_d->transformedHandles.bottomLeft);
        break;
    }

    return cursor;
}

void KisFreeTransformStrategy::paint(QPainter &gc)
{
    gc.save();

    gc.setOpacity(m_d->transaction.basePreviewOpacity());
    gc.setTransform(m_d->paintingTransform, true);
    gc.drawImage(m_d->paintingOffset, originalImage());

    gc.restore();

    // Draw Handles

    QRectF handleRect =
        KisTransformUtils::handleRect(KisTransformUtils::handleVisualRadius,
                                      m_d->handlesTransform,
                                      m_d->transaction.originalRect(), 0, 0);

    qreal rX = 1;
    qreal rY = 1;
    QRectF rotationCenterRect =
        KisTransformUtils::handleRect(KisTransformUtils::rotationHandleVisualRadius,
                                      m_d->handlesTransform,
                                      m_d->transaction.originalRect(),
                                      &rX,
                                      &rY);

    QPainterPath handles;

    handles.moveTo(m_d->rotatedHandles.topLeft);
    handles.lineTo(m_d->rotatedHandles.topRight);
    handles.lineTo(m_d->rotatedHandles.bottomRight);
    handles.lineTo(m_d->rotatedHandles.bottomLeft);
    handles.lineTo(m_d->rotatedHandles.topLeft);

    handles.addRect(handleRect.translated(m_d->rotatedHandles.topLeft));
    handles.addRect(handleRect.translated(m_d->rotatedHandles.topRight));
    handles.addRect(handleRect.translated(m_d->rotatedHandles.bottomLeft));
    handles.addRect(handleRect.translated(m_d->rotatedHandles.bottomRight));
    handles.addRect(handleRect.translated(m_d->rotatedHandles.middleLeft));
    handles.addRect(handleRect.translated(m_d->rotatedHandles.middleRight));
    handles.addRect(handleRect.translated(m_d->rotatedHandles.topMiddle));
    handles.addRect(handleRect.translated(m_d->rotatedHandles.bottomMiddle));

    QPointF rotationCenter = m_d->rotatedHandles.rotationCenter;
    QPointF dx(rX + 3, 0);
    QPointF dy(0, rY + 3);
    handles.addEllipse(rotationCenterRect.translated(rotationCenter));
    handles.moveTo(rotationCenter - dx);
    handles.lineTo(rotationCenter + dx);
    handles.moveTo(rotationCenter - dy);
    handles.lineTo(rotationCenter + dy);

    gc.save();

    if (m_d->isTransforming) {
        gc.setOpacity(0.1);
    }

    //gc.setTransform(m_d->handlesTransform, true); <-- don't do like this!
    QPainterPath mappedHandles = m_d->handlesTransform.map(handles);

    QPen pen[2];
    pen[0].setWidth(decorationThickness());
    pen[0].setCosmetic(true);
    pen[1].setWidth(decorationThickness() * 2);
    pen[1].setCosmetic(true);
    pen[1].setColor(Qt::lightGray);

    for (int i = 1; i >= 0; --i) {
        gc.setPen(pen[i]);
        gc.drawPath(mappedHandles);
    }

    gc.restore();
}

void KisFreeTransformStrategy::externalConfigChanged()
{
    m_d->recalculateTransformations();
}

bool KisFreeTransformStrategy::beginPrimaryAction(const QPointF &pt)
{
    m_d->clickArgs = m_d->currentArgs;
    m_d->clickPos = pt;

    KisTransformUtils::MatricesPack m(m_d->clickArgs);
    m_d->clickTransform = m.finalTransform();

    return true;
}

void KisFreeTransformStrategy::continuePrimaryAction(const QPointF &mousePos,
                                                     bool shiftModifierActive,
                                                     bool altModifierActive)
{
    // Note: "shiftModifierActive" just tells us if the shift key is being pressed
    // Note: "altModifierActive" just tells us if the alt key is being pressed

    m_d->isTransforming = true;
    const QPointF anchorPoint = m_d->clickArgs.originalCenter() + m_d->clickArgs.rotationCenterOffset();

    switch (m_d->function) {
    case MOVE: {
        QPointF diff = mousePos - m_d->clickPos;

        if (shiftModifierActive) {

            KisTransformUtils::MatricesPack m(m_d->clickArgs);
            QTransform t = m.S * m.BR * m.projectedP;
            QPointF originalDiff = t.inverted().map(diff);

            if (qAbs(originalDiff.x()) >= qAbs(originalDiff.y())) {
                originalDiff.setY(0);
            } else {
                originalDiff.setX(0);
            }

            diff = t.map(originalDiff);

        }

        m_d->currentArgs.setTransformedCenter(m_d->clickArgs.transformedCenter() + diff);

        break;
    }
    case ROTATEBOUNDS:
    {
        const KisTransformUtils::MatricesPack clickM(m_d->clickArgs);
        const QTransform clickT = clickM.finalTransform();

        const QPointF rotationCenter = m_d->clickArgs.originalCenter() + m_d->clickArgs.rotationCenterOffset();
        const QPointF clickMouseImagePos = clickT.inverted().map(m_d->clickPos) - rotationCenter;
        const QPointF mouseImagePosClickSpace = clickT.inverted().map(mousePos) - rotationCenter;

        const qreal a1 = atan2(clickMouseImagePos.y(), clickMouseImagePos.x());
        const qreal a2 = atan2(mouseImagePosClickSpace.y(), mouseImagePosClickSpace.x());

        const qreal theta = a2 - a1;
        m_d->currentArgs.setBoundsRotation(m_d->clickArgs.boundsRotation() + theta);
        
        // Rotate scale/shear to compensate
        qreal phi = -m_d->currentArgs.boundsRotation() + m_d->clickArgs.boundsRotation();
        QTransform BR; BR.rotateRadians(phi);
        QTransform BRI; BRI.rotateRadians(-phi);
        QTransform desired = BRI * clickM.SC * clickM.S * BR;
        KisTransformUtils::ScaleShearSolution solution = KisTransformUtils::solveScaleShear(desired);
        if (solution.isValid) {
            m_d->currentArgs.setScaleX(solution.scaleX);
            m_d->currentArgs.setScaleY(solution.scaleY);
            m_d->currentArgs.setShearX(solution.shearX);
            m_d->currentArgs.setShearY(solution.shearY);
        }

        // Snap with shift key
        // if (shiftModifierActive) {
        //     const qreal angle = m_d->currentArgs.boundsRotation();
        //     const qreal snapAngle = M_PI_4 / 6.0; // 7.5 degrees
        //     qint32 angleIndex = static_cast<qint32>((angle / snapAngle) + 0.5);
        //     m_d->currentArgs.setBoundsRotation(angleIndex * snapAngle);
        // }
    }
    break;
    case ROTATE:
    {
        const KisTransformUtils::MatricesPack clickM(m_d->clickArgs);
        const QTransform clickT = clickM.finalTransform();

        const QPointF rotationCenter = m_d->clickArgs.originalCenter() + m_d->clickArgs.rotationCenterOffset();
        const QPointF clickMouseImagePos = clickT.inverted().map(m_d->clickPos) - rotationCenter;
        const QPointF mouseImagePosClickSpace = clickT.inverted().map(mousePos) - rotationCenter;

        const qreal a1 = atan2(clickMouseImagePos.y(), clickMouseImagePos.x());
        const qreal a2 = atan2(mouseImagePosClickSpace.y(), mouseImagePosClickSpace.x());

        /**
         * We use determinant of `clickM.SC` instead of `clickT` to be able to catch
         * the case when the image is flipped by 0x or 0y perspective rotations.
         */
        const qreal theta = KisAlgebra2D::signZZ(clickM.SC.determinant()) * (a2 - a1);

        // Snap with shift key
        if (shiftModifierActive) {
            const qreal snapAngle = M_PI_4 / 6.0; // 7.5 degrees
            qint32 thetaIndex = static_cast<qint32>((theta / snapAngle) + 0.5);
            m_d->currentArgs.setAZ(thetaIndex * snapAngle);
        }
        else {
            const qreal clickAngle = m_d->clickArgs.aZ();
            const qreal targetAngle = m_d->clickArgs.aZ() + theta;
            qreal shortestDistance = shortestAngularDistance(clickAngle, targetAngle);
            const bool clockwise =  (theta <= M_PI && theta >= 0) || (theta < -M_PI);
            shortestDistance = clockwise ? shortestDistance : shortestDistance * -1;

            m_d->currentArgs.setAZ(m_d->clickArgs.aZ() + shortestDistance);
        }

        KisTransformUtils::MatricesPack m(m_d->currentArgs);
        QTransform t = m.finalTransform();
        QPointF newRotationCenter = t.map(m_d->currentArgs.originalCenter() + m_d->currentArgs.rotationCenterOffset());
        QPointF oldRotationCenter = clickT.map(m_d->clickArgs.originalCenter() + m_d->clickArgs.rotationCenterOffset());

        m_d->currentArgs.setTransformedCenter(m_d->currentArgs.transformedCenter() + oldRotationCenter - newRotationCenter);
    }
    break;
    case PERSPECTIVE:
    {
        QPointF diff = mousePos - m_d->clickPos;
        double thetaX = - diff.y() * M_PI / m_d->transaction.originalHalfHeight() / 2 / fabs(m_d->currentArgs.scaleY());
        m_d->currentArgs.setAX(normalizeAngle(m_d->clickArgs.aX() + thetaX));

        qreal sign = qAbs(m_d->currentArgs.aX() - M_PI) < M_PI / 2 ? -1.0 : 1.0;
        double thetaY = sign * diff.x() * M_PI / m_d->transaction.originalHalfWidth() / 2 / fabs(m_d->currentArgs.scaleX());
        m_d->currentArgs.setAY(normalizeAngle(m_d->clickArgs.aY() + thetaY));

        KisTransformUtils::MatricesPack m(m_d->currentArgs);
        QTransform t = m.finalTransform();
        QPointF newRotationCenter = t.map(m_d->currentArgs.originalCenter() + m_d->currentArgs.rotationCenterOffset());

        KisTransformUtils::MatricesPack clickM(m_d->clickArgs);
        QTransform clickT = clickM.finalTransform();
        QPointF oldRotationCenter = clickT.map(m_d->clickArgs.originalCenter() + m_d->clickArgs.rotationCenterOffset());

        m_d->currentArgs.setTransformedCenter(m_d->currentArgs.transformedCenter() + oldRotationCenter - newRotationCenter);
    }
    break;
    case TOPSCALE:
    case BOTTOMSCALE: {
        QPointF staticPoint;
        QPointF movingPoint;

        if (m_d->function == TOPSCALE) {
            staticPoint = m_d->rotatedHandles.bottomMiddle;
            movingPoint = m_d->rotatedHandles.topMiddle;
        } else {
            staticPoint = m_d->rotatedHandles.topMiddle;
            movingPoint = m_d->rotatedHandles.bottomMiddle;
        }

        QPointF staticPointInView = m_d->clickTransform.map(staticPoint);
        const QPointF movingPointInView = m_d->clickTransform.map(movingPoint);

        const QPointF projNormVector =
            KisAlgebra2D::normalize(movingPointInView - staticPointInView);

        const qreal projLength =
            KisAlgebra2D::dotProduct(mousePos - staticPointInView, projNormVector);

        const QPointF targetMovingPointInView = staticPointInView + projNormVector * projLength;

        // override scale static point if it is locked
        if ((m_d->clickArgs.transformAroundRotationCenter() ^ altModifierActive) &&
            !qFuzzyCompare(anchorPoint.y(), movingPoint.y())) {

            staticPoint = anchorPoint;
            staticPointInView = m_d->clickTransform.map(staticPoint);
        }

        GSL::ScaleResult1D result =
            GSL::calculateScaleY(m_d->currentArgs,
                                 staticPoint,
                                 staticPointInView,
                                 movingPoint,
                                 targetMovingPointInView);

        if (!result.isValid) {
            break;
        }

        if (shiftModifierActive ||  m_d->currentArgs.keepAspectRatio()) {
            qreal aspectRatio = m_d->clickArgs.scaleX() / m_d->clickArgs.scaleY();
            m_d->currentArgs.setScaleX(aspectRatio * result.scale);
        }

        m_d->currentArgs.setScaleY(result.scale);
        m_d->currentArgs.setTransformedCenter(result.transformedCenter);
        break;
    }

    case LEFTSCALE:
    case RIGHTSCALE: {
        QPointF staticPoint;
        QPointF movingPoint;

        if (m_d->function == LEFTSCALE) {
            staticPoint = m_d->rotatedHandles.middleRight;
            movingPoint = m_d->rotatedHandles.middleLeft;
        } else {
            staticPoint = m_d->rotatedHandles.middleLeft;
            movingPoint = m_d->rotatedHandles.middleRight;
        }

        QPointF staticPointInView = m_d->clickTransform.map(staticPoint);
        const QPointF movingPointInView = m_d->clickTransform.map(movingPoint);

        const QPointF projNormVector =
            KisAlgebra2D::normalize(movingPointInView - staticPointInView);

        const qreal projLength =
            KisAlgebra2D::dotProduct(mousePos - staticPointInView, projNormVector);

        const QPointF targetMovingPointInView = staticPointInView + projNormVector * projLength;

        // override scale static point if it is locked
        if ((m_d->currentArgs.transformAroundRotationCenter() ^ altModifierActive) &&
            !qFuzzyCompare(anchorPoint.x(), movingPoint.x())) {

            staticPoint = anchorPoint;
            staticPointInView = m_d->clickTransform.map(staticPoint);
        }

        GSL::ScaleResult1D result =
            GSL::calculateScaleX(m_d->currentArgs,
                                 staticPoint,
                                 staticPointInView,
                                 movingPoint,
                                 targetMovingPointInView);

        if (!result.isValid) {
            break;
        }

        if (shiftModifierActive  ||  m_d->currentArgs.keepAspectRatio()) {
            qreal aspectRatio = m_d->clickArgs.scaleY() / m_d->clickArgs.scaleX();
            m_d->currentArgs.setScaleY(aspectRatio * result.scale);
        }

        m_d->currentArgs.setScaleX(result.scale);
        m_d->currentArgs.setTransformedCenter(result.transformedCenter);
        break;
    }
    case TOPRIGHTSCALE:
    case BOTTOMRIGHTSCALE:
    case TOPLEFTSCALE:
    case BOTTOMLEFTSCALE: {
        QPointF staticPoint;
        QPointF movingPoint;

        if (m_d->function == TOPRIGHTSCALE) {
            staticPoint = m_d->rotatedHandles.bottomLeft;
            movingPoint = m_d->rotatedHandles.topRight;
        } else if (m_d->function == BOTTOMRIGHTSCALE) {
            staticPoint = m_d->rotatedHandles.topLeft;
            movingPoint = m_d->rotatedHandles.bottomRight;
        } else if (m_d->function == TOPLEFTSCALE) {
            staticPoint = m_d->rotatedHandles.bottomRight;
            movingPoint = m_d->rotatedHandles.topLeft;
        } else {
            staticPoint = m_d->rotatedHandles.topRight;
            movingPoint = m_d->rotatedHandles.bottomLeft;
        }

        // override scale static point if it is locked
        if ((m_d->currentArgs.transformAroundRotationCenter() ^ altModifierActive) &&
            !(qFuzzyCompare(anchorPoint.x(), movingPoint.x()) ||
              qFuzzyCompare(anchorPoint.y(), movingPoint.y()))) {

            staticPoint = anchorPoint;
        }

        QPointF staticPointInView = m_d->clickTransform.map(staticPoint);
        QPointF movingPointInView = mousePos;

        if (shiftModifierActive  ||  m_d->currentArgs.keepAspectRatio()) {
            QPointF refDiff = m_d->clickTransform.map(movingPoint) - staticPointInView;
            QPointF realDiff = mousePos - staticPointInView;
            realDiff = kisProjectOnVector(refDiff, realDiff);

            movingPointInView = staticPointInView + realDiff;
        }

        const bool isAffine =
            qFuzzyIsNull(m_d->currentArgs.aX()) &&
            qFuzzyIsNull(m_d->currentArgs.aY());

        GSL::ScaleResult2D result =
                !isAffine ?
                    GSL::calculateScale2D(m_d->currentArgs,
                                          staticPoint,
                                          staticPointInView,
                                          movingPoint,
                                          movingPointInView) :
                    GSL::calculateScale2DAffine(m_d->currentArgs,
                                                staticPoint,
                                                staticPointInView,
                                                movingPoint,
                                                movingPointInView);

        if (result.isValid) {
            m_d->currentArgs.setScaleX(result.scaleX);
            m_d->currentArgs.setScaleY(result.scaleY);
            m_d->currentArgs.setTransformedCenter(result.transformedCenter);
        }

        break;
    }
    case MOVECENTER: {
        QPointF pt = m_d->transform.inverted().map(mousePos);
        if (altModifierActive) {
            pt = KisTransformUtils::clipInRect(pt, m_d->transaction.originalRect());
        }

        QPointF newRotationCenterOffset = pt - m_d->currentArgs.originalCenter();

        if (shiftModifierActive) {
            if (qAbs(newRotationCenterOffset.x()) > qAbs(newRotationCenterOffset.y())) {
                newRotationCenterOffset.ry() = 0;
            } else {
                newRotationCenterOffset.rx() = 0;
            }
        }

        m_d->currentArgs.setRotationCenterOffset(newRotationCenterOffset);
        emit requestResetRotationCenterButtons();
    }
        break;
    case TOPSHEAR:
    case BOTTOMSHEAR: {
        KisTransformUtils::MatricesPack m(m_d->clickArgs);

        QPointF oldStaticPoint = m.finalTransform().map(m_d->clickArgs.originalCenter() + m_d->clickArgs.rotationCenterOffset());

        QTransform backwardT = (m.S * m.BR * m.projectedP).inverted();
        QPointF diff = backwardT.map(mousePos - m_d->clickPos);

        qreal sign = m_d->function == BOTTOMSHEAR ? 1.0 : -1.0;

        // get the dx pixels corresponding to the current shearX factor
        qreal dx = sign * m_d->clickArgs.shearX() * m_d->clickArgs.scaleY() * m_d->transaction.originalHalfHeight(); // get the dx pixels corresponding to the current shearX factor
        dx += diff.x();

        // calculate the new shearX factor
        m_d->currentArgs.setShearX(sign * dx / m_d->currentArgs.scaleY() / m_d->transaction.originalHalfHeight()); // calculate the new shearX factor

        KisTransformUtils::MatricesPack currentM(m_d->currentArgs);
        QTransform t = currentM.finalTransform();
        QPointF newStaticPoint = t.map(m_d->clickArgs.originalCenter() + m_d->clickArgs.rotationCenterOffset());
        m_d->currentArgs.setTransformedCenter(m_d->currentArgs.transformedCenter() + oldStaticPoint - newStaticPoint);
        break;
    }

    case LEFTSHEAR:
    case RIGHTSHEAR: {
        KisTransformUtils::MatricesPack m(m_d->clickArgs);

        QPointF oldStaticPoint = m.finalTransform().map(m_d->clickArgs.originalCenter() + m_d->clickArgs.rotationCenterOffset());

        QTransform backwardT = (m.S * m.BR * m.projectedP).inverted();
        QPointF diff = backwardT.map(mousePos - m_d->clickPos);

        qreal sign = m_d->function == RIGHTSHEAR ? 1.0 : -1.0;

        // get the dx pixels corresponding to the current shearX factor
        qreal dy = sign *  m_d->clickArgs.shearY() * m_d->clickArgs.scaleX() * m_d->transaction.originalHalfWidth();
        dy += diff.y();

        // calculate the new shearY factor
        m_d->currentArgs.setShearY(sign * dy / m_d->clickArgs.scaleX() / m_d->transaction.originalHalfWidth());

        KisTransformUtils::MatricesPack currentM(m_d->currentArgs);
        QTransform t = currentM.finalTransform();
        QPointF newStaticPoint = t.map(m_d->clickArgs.originalCenter() + m_d->clickArgs.rotationCenterOffset());
        m_d->currentArgs.setTransformedCenter(m_d->currentArgs.transformedCenter() + oldStaticPoint - newStaticPoint);
        break;
    }
    }

    m_d->recalculateTransformations();
}

bool KisFreeTransformStrategy::endPrimaryAction()
{
    bool shouldSave = !m_d->imageTooBig;
    m_d->isTransforming = false;

    if (m_d->imageTooBig) {
        m_d->currentArgs = m_d->clickArgs;
        m_d->recalculateTransformations();
    }

    return shouldSave;
}

void KisFreeTransformStrategy::Private::recalculateTransformations()
{
    KisTransformUtils::MatricesPack m(currentArgs);
    QTransform sanityCheckMatrix = m.TS * m.SC * m.S * m.projectedP;

    /**
     * The center of the original image should still
     * stay the origin of CS
     */
    KIS_ASSERT_RECOVER_NOOP(sanityCheckMatrix.map(currentArgs.originalCenter()).manhattanLength() < 1e-4);

    transform = m.finalTransform();

    QTransform viewScaleTransform = converter->imageToDocumentTransform() * converter->documentToFlakeTransform();
    handlesTransform = transform * viewScaleTransform;

    QTransform tl = QTransform::fromTranslate(transaction.originalTopLeft().x(), transaction.originalTopLeft().y());
    paintingTransform = tl.inverted() * q->thumbToImageTransform() * tl * transform * viewScaleTransform;
    paintingOffset = transaction.originalTopLeft();

    // check whether image is too big to be displayed or not
    imageTooBig = KisTransformUtils::checkImageTooBig(transaction.originalRect(), m, currentArgs.cameraPos().z());

    // recalculate cached handles position
    recalculateTransformedHandles();

    emit q->requestShowImageTooBig(imageTooBig);
    emit q->requestImageRecalculation();
}
