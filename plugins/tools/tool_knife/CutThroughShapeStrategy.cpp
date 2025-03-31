/*
 *  SPDX-FileCopyrightText: 2025 Agata Cacko
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "CutThroughShapeStrategy.h"

#include <QDebug>
#include <QPainter>

#include <kis_algebra_2d.h>
#include <KoToolBase.h>
#include <KoCanvasBase.h>
#include <KoViewConverter.h>
#include <KoSelection.h>
#include <kis_global.h>
#include "kis_debug.h"
#include <KoPathShape.h>
#include <krita_utils.h>
#include <kis_canvas2.h>
#include <QPainterPath>
#include <KoShapeController.h>
#include <kundo2command.h>
#include <KoKeepShapesSelectedCommand.h>
#include <QtMath>


CutThroughShapeStrategy::CutThroughShapeStrategy(KoToolBase *tool, KoSelection *selection, const QList<KoShape *> &shapes, QPointF startPoint, const GutterWidthsConfig &width)
    : KoInteractionStrategy(tool)
    , m_startPoint(startPoint)
    , m_endPoint(startPoint)
    , m_width(width)
{
    qCritical() << "Creating a strategy";

    m_selectedShapes = selection->selectedEditableShapes();
    qCritical() << "Amount of selected shapes: " << m_selectedShapes.length();
    m_allShapes = shapes;
    qCritical() << "Amount of all shapes: " << m_allShapes.length();
}

CutThroughShapeStrategy::~CutThroughShapeStrategy()
{

}

KUndo2Command *CutThroughShapeStrategy::createCommand()
{
    // TODO: undoing
    return 0;
}

QPointF snapEndPoint(const QPointF &startPoint, const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers) {

    QPointF nicePoint = snapToClosestNiceAngle(mouseLocation, startPoint); // by default the function gives you 15 degrees increments

    if (modifiers & Qt::KeyboardModifier::ShiftModifier) {
        return nicePoint;
        if (qAbs(mouseLocation.x() - startPoint.x()) >= qAbs(mouseLocation.y() - startPoint.y())) {
            // do horizontal line
            return QPointF(mouseLocation.x(), startPoint.y());
        } else {
            return QPointF(startPoint.x(), mouseLocation.y());
        }
    }
    QLineF line = QLineF(startPoint, mouseLocation);
    qreal angle = line.angleTo(QLineF(startPoint, nicePoint));
    qreal eps = kisDegreesToRadians(2.0f);
    if (angle < eps) {
        return nicePoint;
    }
    return mouseLocation;
}

void CutThroughShapeStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    m_endPoint = snapEndPoint(m_startPoint, mouseLocation, modifiers);
    //qCritical() << "Now supposedly drawing a line between " << m_d->startPoint << " to " << m_d->endPoint;
    QRectF dirtyRect;
    KisAlgebra2D::accumulateBounds(m_startPoint, &dirtyRect);
    KisAlgebra2D::accumulateBounds(m_endPoint, &dirtyRect);
    dirtyRect = kisGrowRect(dirtyRect, gutterWidthInDocumentCoordinates(calculateLineAngle(m_startPoint, m_endPoint))); // twice as much as it should need to account for lines showing the effect

    KisCanvas2* kisCanvas = dynamic_cast<KisCanvas2*>(tool()->canvas());
    //dirtyRect = kisCanvas->viewManager()->
    //dirtyRect = canvas()->viewConverter()->viewToDocument(dirtyRect);
    //qCritical() << "And the dirty rect is: " << dirtyRect;

    QRectF accumulatedWithPrevious = m_previousLineDirtyRect | dirtyRect;

    //KisAlgebra2D::accumulateBounds(dirtyRect, &accumulatedWithPrevious);
    //KisAlgebra2D::accumulateBounds(dirtyRect.topLeft(), &accumulatedWithPrevious);
    //KisAlgebra2D::accumulateBounds(dirtyRect.bottomRight(), &accumulatedWithPrevious);

    //QRectF rect = dirtyRect + m_d->previousLineDirtyRect;

    tool()->canvas()->updateCanvas(accumulatedWithPrevious);
    m_previousLineDirtyRect = dirtyRect;

}

void CutThroughShapeStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    tool()->canvas()->updateCanvas(m_previousLineDirtyRect);


    KisCanvas2 *kisCanvas = static_cast<KisCanvas2 *>(tool()->canvas());
    KIS_SAFE_ASSERT_RECOVER_RETURN(kisCanvas);
    const QTransform booleanWorkaroundTransform = KritaUtils::pathShapeBooleanSpaceWorkaround(kisCanvas->image());

    QList<QPainterPath> srcOutlines;
    QRectF outlineRect;

    if (m_allShapes.length() == 0) {
        qCritical() << "No shapes are available";
        return;
    }

    Q_FOREACH (KoShape *shape, m_allShapes) {

        QPainterPath outlineHere =
            booleanWorkaroundTransform.map(
            shape->absoluteTransformation().map(
                shape->outline()));

        srcOutlines << outlineHere;
        outlineRect |= outlineHere.boundingRect();//booleanWorkaroundTransform.map(shape->absoluteOutlineRect()).boundingRect();
    }

    if (outlineRect.isEmpty()) {

        qCritical() << "The outline rect is empty";
        return;
    }

    QRectF outlineRectBigger = kisGrowRect(outlineRect, 10);
    QRect outlineRectBiggerInt = outlineRectBigger.toRect();

    QLineF gapLine = QLineF(m_startPoint, m_endPoint);
    qreal eps = 0.0000001;
    if (gapLine.length() < eps) {
        return;
    }

    qCritical() << "line angle = " << calculateLineAngle(m_startPoint, m_endPoint);
    qreal gutterWidth = gutterWidthInDocumentCoordinates(calculateLineAngle(m_startPoint, m_endPoint));


    QList<QLineF> gapLines = KisAlgebra2D::getParallelLines(gapLine, gutterWidth/2);

    qCritical() << "############ Are those lines correct? " << KisAlgebra2D::pointToLineDistSquared(gapLines[0].p1(), gapLines[1])
            << KisAlgebra2D::pointToLineDistSquared(gapLines[1].p1(), gapLines[0]) << KisAlgebra2D::pointToLineDistSquared(gapLines[0].p1(), gapLine);
    //qCritical() << "Should be: " << pow2(gapWidth/2) << "or" << pow2(gapWidth);



    //KIS_ASSERT(qFuzzyCompare(KisAlgebra2D::pointToLineDistSquared(gapLines[0].p1(), gapLines[1]), pow2(m_width)) && "Distance doesn't match");

    gapLine = booleanWorkaroundTransform.map(gapLine);
    gapLines[0] = booleanWorkaroundTransform.map(gapLines[0]);
    gapLines[1] = booleanWorkaroundTransform.map(gapLines[1]);

    QLineF leftLine = gapLines[0];
    QLineF rightLine = gapLines[1];

    QLineF leftLineLong = leftLine;
    QLineF rightLineLong = rightLine;



    KisAlgebra2D::cropLineToRect(leftLineLong, outlineRectBiggerInt, true, true);
    KisAlgebra2D::cropLineToRect(rightLineLong, outlineRectBiggerInt, true, true);

    KUndo2Command *cmd = new KUndo2Command(kundo2_i18n("Knife tool: cut through shapes"));


    new KoKeepShapesSelectedCommand(m_selectedShapes, {}, kisCanvas->selectedShapesProxy(), false, cmd);


    if (leftLine.length() == 0 || rightLine.length() == 0) {
        qCritical() << "No cutting at all, because one of the lines has length 0. Line is: " << leftLine << rightLine << " and the outline rect is: " << outlineRect << ppVar(outlineRectBigger);
        qCritical() << "^ No cutting at all... line was: " << gapLine << "and the other were: " << gapLines[0] << gapLines[1];

        KIS_SAFE_ASSERT_RECOVER_RETURN(gapLine.length() != 0 && gapLines[0].length() != 0 && gapLines[1].length() != 0 && "Original gap lines shouldn't be empty at this point");
        // looks like *all* shapes need to be cut out

        tool()->canvas()->shapeController()->removeShapes(m_allShapes, cmd);
        tool()->canvas()->addCommand(cmd);
        return;
    }


    QList<QPainterPath> paths = KisAlgebra2D::getPathsFromRectangleCutThrough(QRectF(outlineRectBiggerInt), leftLineLong, rightLineLong);
    QPainterPath left = paths[0];
    QPainterPath right = paths[1];

    QList<QPainterPath> pathsOpposite = KisAlgebra2D::getPathsFromRectangleCutThrough(QRectF(outlineRectBiggerInt), rightLineLong, leftLineLong);
    QPainterPath leftOpposite = pathsOpposite[0];
    QPainterPath rightOpposite = pathsOpposite[1];



    //QPainterPath dstOutline;
    //QList<QPainterPath> dstOutlines;

    QList<KoShape*> newSelectedShapes;

    QList<KoShape*> shapesToRemove;

    QTransform booleanWorkaroundTransformInverted = booleanWorkaroundTransform.inverted();

    QRectF gapLineLeftRect = KisAlgebra2D::createRectFromCorners(leftLine);
    QRectF gapLineRightRect = KisAlgebra2D::createRectFromCorners(rightLine);


    for (int i = 0; i < srcOutlines.size(); i++) {

        KoShape* referenceShape = m_allShapes[i];
        bool wasSelected = m_selectedShapes.contains(referenceShape);

        if ((srcOutlines[i].boundingRect() & leftOpposite.boundingRect()).isEmpty()
                || (srcOutlines[i].boundingRect() & rightOpposite.boundingRect()).isEmpty()) {
            // there is nothing on one side
            // everything is on the other, far away from the gap line
            // it just makes it a bit faster when there is a whole lot of shapes

            if (wasSelected) {
                newSelectedShapes << referenceShape;
            }
            continue;
        }

        if ((srcOutlines[i].boundingRect() & gapLineLeftRect).isEmpty()
                || (srcOutlines[i].boundingRect() & gapLineRightRect).isEmpty()) {
            // the gap lines can't cross the shape since their bounding rects don't cross
            if (wasSelected) {
                newSelectedShapes << referenceShape;
            }
            continue;
        }

        // either one of the points is inside the path, or the line crosses one of the segments
        bool containsGapLinePoint = srcOutlines[i].contains(leftLine.p1()) || srcOutlines[i].contains(leftLine.p2())
                || srcOutlines[i].contains(rightLine.p1()) || srcOutlines[i].contains(rightLine.p2());
        bool crossesGapLine = KisAlgebra2D::getLineSegmentCrossingLineIndexes(leftLine, srcOutlines[i]).count() > 0
                || KisAlgebra2D::getLineSegmentCrossingLineIndexes(rightLine, srcOutlines[i]).count() > 0;
        if (!containsGapLinePoint && !crossesGapLine) {
            //
            qCritical() << "it doesn't cross the line!";
            if (wasSelected) {
                newSelectedShapes << referenceShape;
            }
            continue;
        }




        QPainterPath leftPath = srcOutlines[i] & left;
        QPainterPath rightPath = srcOutlines[i] & right;

        QList<QPainterPath> bothSides;
        bothSides << leftPath << rightPath;


        Q_FOREACH(QPainterPath path, bothSides) {
            if (path.isEmpty()) {
                continue;
            }

            // comment copied from another place:
            // there is a bug in Qt, sometimes it leaves the resulting
            // outline open, so just close it explicitly.
            path.closeSubpath();
            // this is needed because Qt linearize curves; this allows for a
            // "sane" linearization instead of a very blocky appearance
            path = booleanWorkaroundTransformInverted.map(path);
            KoPathShape* shape = KoPathShape::createShapeFromPainterPath(path);

            if (shape->boundingRect().isEmpty()) {
                continue;
            }

            shape->setBackground(referenceShape->background());
            shape->setStroke(referenceShape->stroke());
            shape->setZIndex(referenceShape->zIndex());

            KoShapeContainer *parent = referenceShape->parent();
            tool()->canvas()->shapeController()->addShapeDirect(shape, parent, cmd);

            if (wasSelected) {
                newSelectedShapes << shape;
            }


        }

        // that happens no matter if there was any non-empty shape
        // because if there is none, maybe they just were underneath the gap
        shapesToRemove << m_allShapes[i];

    }

    tool()->canvas()->shapeController()->removeShapes(shapesToRemove, cmd);
    new KoKeepShapesSelectedCommand({}, newSelectedShapes, tool()->canvas()->selectedShapesProxy(), true, cmd);


    tool()->canvas()->addCommand(cmd);



}

void CutThroughShapeStrategy::paint(QPainter &painter, const KoViewConverter &converter)
{
    painter.save();

    QColor semitransparentGray = QColor(Qt::darkGray);
    semitransparentGray.setAlphaF(0.6);
    QPen pen = QPen(QBrush(semitransparentGray), 2);
    painter.setPen(pen);

    painter.setRenderHint(QPainter::RenderHint::Antialiasing, true);

    qreal gutterWidth = gutterWidthInDocumentCoordinates(calculateLineAngle(m_startPoint, m_endPoint));

    QLineF gutterCenterLine = QLineF(m_startPoint, m_endPoint);
    gutterCenterLine = converter.documentToView().map(gutterCenterLine);
    QLineF gutterWidthHelperLine = QLineF(QPointF(0, 0), QPointF(gutterWidth, 0));
    gutterWidthHelperLine = converter.documentToView().map(gutterWidthHelperLine);

    gutterWidth = gutterWidthHelperLine.length();

    QList<QLineF> gutterLines = KisAlgebra2D::getParallelLines(gutterCenterLine, gutterWidth/2);

    QLineF gutterLine1 = gutterLines.length() > 0 ? gutterLines[0] : gutterCenterLine;
    QLineF gutterLine2 = gutterLines.length() > 1 ? gutterLines[1] : gutterCenterLine;


    painter.drawLine(gutterLine1);
    painter.drawLine(gutterLine2);

    QRectF arcRect1 = QRectF(gutterCenterLine.p1() - QPointF(gutterWidth/2, gutterWidth/2), gutterCenterLine.p1() + QPointF(gutterWidth/2, gutterWidth/2));
    QRectF arcRect2 = QRectF(gutterCenterLine.p2() - QPointF(gutterWidth/2, gutterWidth/2), gutterCenterLine.p2() + QPointF(gutterWidth/2, gutterWidth/2));

    int qtAngleFactor = 16;
    int qtHalfCircle = qtAngleFactor*180;

    painter.drawArc(arcRect1, -qtAngleFactor*kisRadiansToDegrees(KisAlgebra2D::directionBetweenPoints(gutterCenterLine.p1(), gutterLine1.p1(), 0)), qtHalfCircle);
    painter.drawArc(arcRect2, -qtAngleFactor*kisRadiansToDegrees(KisAlgebra2D::directionBetweenPoints(gutterCenterLine.p2(), gutterLine1.p2(), 0)), -qtHalfCircle);


    int xLength = 3;
    qreal xLengthEllipse = 2*qSqrt(2);

    if (false) { // drawing X
    painter.drawLine({QLineF(gutterCenterLine.p1() - QPointF(xLength, xLength), gutterCenterLine.p1() + QPointF(xLength, xLength))});
    painter.drawLine({QLineF(gutterCenterLine.p2() - QPointF(xLength, xLength), gutterCenterLine.p2() + QPointF(xLength, xLength))});

    painter.drawLine({QLineF(gutterCenterLine.p1() - QPointF(xLength, -xLength), gutterCenterLine.p1() + QPointF(xLength, -xLength))});
    painter.drawLine({QLineF(gutterCenterLine.p2() - QPointF(xLength, -xLength), gutterCenterLine.p2() + QPointF(xLength, -xLength))});
    }

    // ellipse at the both ends of the gutter center line
    painter.drawEllipse(gutterCenterLine.p1(), xLengthEllipse, xLengthEllipse);
    painter.drawEllipse(gutterCenterLine.p2(), xLengthEllipse, xLengthEllipse);



    pen.setWidth(1);
    semitransparentGray.setAlphaF(0.2);
    pen.setColor(semitransparentGray);

    painter.setPen(pen);

    //QLineF gutterLineExtended1 = gutterLine1;
    //QLineF gutterLineExtended2 = gutterLine2;
    //KisAlgebra2D::cropLineToRect(gutterLineExtended1, painter.viewport(), true, true);
    //KisAlgebra2D::cropLineToRect(gutterLineExtended2, painter.viewport(), true, true);

    painter.drawLine(gutterCenterLine);

    painter.restore();
}

qreal CutThroughShapeStrategy::gutterWidthInDocumentCoordinates(qreal lineAngle)
{
    KisCanvas2 *kisCanvas = static_cast<KisCanvas2 *>(tool()->canvas());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(kisCanvas, m_width.widthForAngleInPixels(lineAngle));
    QLineF helperGapWidthLine = QLineF(QPointF(0, 0), QPointF(0, m_width.widthForAngleInPixels(lineAngle)));
    QLineF helperGapWidthLineTransformed = kisCanvas->coordinatesConverter()->imageToDocument(helperGapWidthLine);
    return helperGapWidthLineTransformed.length();
}

qreal CutThroughShapeStrategy::calculateLineAngle(QPointF start, QPointF end)
{
    QPointF vec = end - start;
    qreal angleDegrees = KisAlgebra2D::wrapValue(kisRadiansToDegrees(std::atan2(vec.y(), vec.x())), 0.0, 360.0);
    return angleDegrees;
}
