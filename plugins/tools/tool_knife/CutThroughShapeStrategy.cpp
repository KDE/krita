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


CutThroughShapeStrategy::CutThroughShapeStrategy(KoToolBase *tool, KoSelection *selection, QPointF startPoint, qreal width)
    : KoInteractionStrategy(tool)
    , m_startPoint(startPoint)
    , m_width(width)
{
    qCritical() << "Creating a strategy";

    m_selectedShapes = selection->selectedEditableShapes();
    qCritical() << "Amount of selected shapes: " << m_selectedShapes.length();
}

CutThroughShapeStrategy::~CutThroughShapeStrategy()
{

}

KUndo2Command *CutThroughShapeStrategy::createCommand()
{
    // TODO: undoing
    return 0;
}

void CutThroughShapeStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    m_endPoint = mouseLocation;
    //qCritical() << "Now supposedly drawing a line between " << m_d->startPoint << " to " << m_d->endPoint;
    QRectF dirtyRect;
    KisAlgebra2D::accumulateBounds(m_startPoint, &dirtyRect);
    KisAlgebra2D::accumulateBounds(m_endPoint, &dirtyRect);
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


QList<QLineF> getParallelLines(const QLineF& line, const qreal distance) {
    if (line.length() == 0) {
        return QList<QLineF>();
    }
    QPointF lineVector = line.p2() - line.p1();
    // dist(P, l) = |Ax + By + C|/(sqrt(A^2 + B^2))
    // dist(P, l) = |Vy*x - Vx*y + (p2x*p1y - p2y*p1x)|/(sqrt(line.length()))
    // |Vy*x - Vx*y + M| = distance*sqrt(line.length())
    // // that gives us two line equations:
    // Vy*x - Vx*y + M +/- distance*sqrt(line.length()) = 0
    // if Vy != 0:
    // x = (Vx*y + N)/Vy // where N = M +/- distance*sqrt(line.length())
    // else:
    // y = (Vy*x + N)/Vx // where N = M +/- distance*sqrt(line.length())

    qreal M = line.p2().x()*line.p1().y() - line.p2().y()*line.p1().x();
    qreal N1 = M - distance*sqrt(line.length());
    qreal N2 = M + distance*sqrt(line.length());

    qreal Vx = lineVector.x();
    qreal Vy = lineVector.y();


    auto calculateSecondDimension = [](qreal dim1, qreal N, qreal V1, qreal V2) {
        //(Vx*y + N)/Vy
        return (dim1*V2 + N)/V1;
    };

    QList<QLineF> responses;

    if (lineVector.x() != 0) {
        // y = (Vx*y + N)/Vx // where N = M +/- distance*sqrt(line.length())

        qreal x1 = line.p1().x();
        qreal x2 = line.p2().x();
        //qreal y1 = (lineVector.y()*x1 + N1
        responses << QLineF(x1, calculateSecondDimension(x1, N1, Vx, Vy), x2, calculateSecondDimension(x2, N1, Vx, Vy));
        responses << QLineF(x1, calculateSecondDimension(x1, N2, Vx, Vy), x2, calculateSecondDimension(x2, N2, Vx, Vy));

    } else {

        qreal y1 = line.p1().y();
        qreal y2 = line.p2().y();
        //qreal y1 = (lineVector.y()*x1 + N1
        responses << QLineF(calculateSecondDimension(y1, N1, Vy, Vx), y1, calculateSecondDimension(y2, N1, Vy, Vx), y2);
        responses << QLineF(calculateSecondDimension(y1, N2, Vy, Vx), y1, calculateSecondDimension(y2, N2, Vy, Vx), y2);
    }

    return responses;
}



QPainterPath getOnePathFromRectangleCutThrough(const QList<QPointF> &points, const QLineF &line, bool left) {

    qCritical() << "~~~ ONE PATH at a time ~~~" << left;
    qCritical() << "line: " << line << ", left: " << left << ", points:" << points[0] << points[1] << points[2] << points[3];

    auto onTheLine = [](QPointF first, QPointF second) {
        //qCritical() << "Fuzzy compating " << first << " and " << second << "more precisely: " << first.y() << second.y();
        //qCritical() << "X: " << qFuzzyCompare(first.x(), second.x()) << " Y: " << qFuzzyCompare(first.y(), second.y());
        qCritical() << "(inside) on the line? " << first << second << ":" << qAbs(first.x() - second.x()) << qAbs(first.y() - second.y());
        //return qFuzzyCompare(first.x(), second.x()) || qFuzzyCompare(first.y(), second.y());
        //return KisAlgebra2D::fuzzyPointCompare(first, second, 0.5f);
        float delta = 0.1f;
        qCritical() << (qAbs(first.x() - second.x()) < delta || qAbs(first.y() - second.y()) < delta);
        return qAbs(first.x() - second.x()) < delta || qAbs(first.y() - second.y()) < delta;
    };


    bool started = false;
    QPainterPath path;

    path.moveTo(line.p1());
    path.lineTo(line.p2());

    qCritical() << "path after initialization: " << path;





    qCritical() << "Number of points is: " << points.length();

    QList<QPointF> availablePoints;
    int maxRectPointsNumber = 4; // in case the list has five points to count the first one twice
    for(int i = 0; i < maxRectPointsNumber; i++) {
        qreal whichSide = KisAlgebra2D::crossProduct(line.p2() - line.p1(), line.p2() - points[i]);
        qCritical() << "Point " << points[i] << " has which side: " << whichSide;
        if (whichSide*(left ? 1 : -1) >= 0) {
            availablePoints << points[i];
            qCritical() << "Available point:" << points[i];
        }
    }
    qCritical() << "That's the end of the available points.";

    int startValue, increment;
    if (!left) {
        startValue = 2*availablePoints.length();
        increment = -1;
    } else {
        startValue = 0;
        increment = 1;
    }

    auto stopFor = [](int index, int max, bool left) {
        if (!left) {
            return index <= 0;
        } else {
            return index >= max;
        }
    };

    for (int i = startValue; !stopFor(i, 2*availablePoints.length(), left); i += increment) {
        int index = KisAlgebra2D::wrapValue(i, 0, availablePoints.length());
        qCritical() << "Index is: " << index << "started: " << started <<  "on the line: " << onTheLine(availablePoints[index], path.currentPosition());
        qCritical() << "...And previous position was: " << path.currentPosition() << "while the point at index is: " << availablePoints[index];

        if (onTheLine(availablePoints[index], path.currentPosition())) {
            qCritical() << "It is on the line with the previous point.";
            if (!started) {
                qCritical() << "It starts now, at" << availablePoints[index];
                started = true;

                // TODO: if it's not the same point?
                path.lineTo(availablePoints[index]);
                if (onTheLine(availablePoints[index], line.p1())) {
                    qCritical() << "Despite already starting, we're on the line with p1, so ending the path: " << path;
                    break;
                }

            } else {
                // usually would add, unless it's the ending
                qCritical() << "It started already, we're at" << availablePoints[index] << "now checking whether the currently checked point from points it's on the line with the end";
                if (onTheLine(availablePoints[index], line.p1())) {
                    path.lineTo(availablePoints[index]);
                    qCritical() << "We're on the line with p1, so ending the path: " << path;
                    //path.lineTo();
                    break;
                }
                qCritical() << "It wasn't on the line with the end, we're adding the current point: " << availablePoints[index];
                path.lineTo(availablePoints[index]);
                qCritical() << "Path after adding a new point with a line:" << path;
            }
        }
    }


    qCritical() << "Adding the p1 point to the line at the end of the function, path before that: " << path;
    path.lineTo(line.p1());
    qCritical() << "Adding the p1 point to the line at the end of the function, now path: " << path;


    return path;

    for (int i = startValue; !stopFor(i, 2*points.length(), left); i += increment) {
        int index = KisAlgebra2D::wrapValue(i, 0, points.length());
        qCritical() << "Index is: " << index << "started: " << started <<  "on the line: " << onTheLine(points[index], path.currentPosition());
        qCritical() << "...And current position is: " << path.currentPosition() << "while the point at index is: " << points[index];
        qreal whichSide = KisAlgebra2D::dotProduct(line.p2() - line.p1(), line.p2() - points[index]);
        if (whichSide*(left ? 1 : -1) >= 0) {
            continue;
        }

        if (onTheLine(points[index], path.currentPosition())) {
            qCritical() << "it is on the line";
            if (!started) {
                qCritical() << "it Starts now, at" << points[index] << "but without it";
                started = true;

                // TODO: if it's not the same point?
                path.lineTo(points[index]);

            } else {
                // usually would add, unless it's the ending
                qCritical() << "it started already, we're at" << points[index];
                if (onTheLine(points[index], line.p1())) {
                    qCritical() << "We're on the line with p1, so ending the path: " << path;
                    break;
                }
                path.lineTo(points[index]);
                qCritical() << "Path after adding a new point with a line:" << path;
            }
        }
    }

    qCritical() << "~~~ ONE PATH ^^^ ~~~" << left;
    return path;
}

QList<QPainterPath> getPathsFromRectangleCutThrough(QRectF rect, QLineF leftLine, QLineF rightLine) {

    qCritical() << "~~~ BOTH PATHS ~~~";

    QPainterPath left;
    QPainterPath right;

    left.moveTo(leftLine.p1());
    left.lineTo(leftLine.p2());

    right.moveTo(rightLine.p1());
    right.lineTo(rightLine.p2());

    QList<QPointF> rectPoints;
    rectPoints << rect.topLeft() << rect.bottomLeft() << rect.bottomRight() << rect.topRight();

    left = getOnePathFromRectangleCutThrough(rectPoints, leftLine, true);
    right = getOnePathFromRectangleCutThrough(rectPoints, rightLine, false);

    QList<QPainterPath> paths;
    paths << left << right;

     qCritical() << "~~~ BOTH PATHS ^^^ ~~~";

    return paths;
}

void CutThroughShapeStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    tool()->canvas()->updateCanvas(m_previousLineDirtyRect);


    KisCanvas2 *kisCanvas = static_cast<KisCanvas2 *>(tool()->canvas());
    KIS_SAFE_ASSERT_RECOVER_RETURN(kisCanvas);
    const QTransform booleanWorkaroundTransform = KritaUtils::pathShapeBooleanSpaceWorkaround(kisCanvas->image());

    QList<QPainterPath> srcOutlines;
    QRectF outlineRect;

    if (m_selectedShapes.length() == 0) {
        qCritical() << "No shapes are selected";
        return;
    }

    Q_FOREACH (KoShape *shape, m_selectedShapes) {

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

    // TODO: two lines, for both sides of the gap
    QLineF gapLine = QLineF(m_startPoint, m_endPoint);

    QList<QLineF> gapLines = getParallelLines(gapLine, m_width);

    gapLine = booleanWorkaroundTransform.map(gapLine);
    gapLines[0] = booleanWorkaroundTransform.map(gapLines[0]);
    gapLines[1] = booleanWorkaroundTransform.map(gapLines[1]);

    QLineF leftLine = gapLines[0];
    QLineF rightLine = gapLines[1];


    KisAlgebra2D::cropLineToRect(leftLine, outlineRectBiggerInt, true, true);
    KisAlgebra2D::cropLineToRect(rightLine, outlineRectBiggerInt, true, true);


    if (leftLine.length() == 0 || rightLine.length() == 0) {
        qCritical() << "Line is: " << leftLine << rightLine << " and the outline rect is: " << outlineRect << ppVar(outlineRectBigger);
        return; // there would be no cutting
    }


    QList<QPainterPath> paths = getPathsFromRectangleCutThrough(QRectF(outlineRectBiggerInt), leftLine, rightLine);
    QPainterPath left = paths[0];
    QPainterPath right = paths[1];








    Q_FOREACH(KoShape* shape, m_selectedShapes) {
        KoPathShape* pathShape = dynamic_cast<KoPathShape*>(shape);
        if (pathShape) {
            QPainterPath path = pathShape->outline();
            QRectF rect = pathShape->absoluteOutlineRect();

        }
    }


    //QPainterPath dstOutline;
    //QList<QPainterPath> dstOutlines;

    KUndo2Command *cmd = new KUndo2Command(kundo2_i18n("Knife tool"));
    QList<KoShape*> newSelectedShapes;

    QList<KoShape*> shapesToRemove;

    for (int i = 0; i < srcOutlines.size(); i++) {
        QPainterPath leftPath = srcOutlines[i] & left;
        QPainterPath rightPath = srcOutlines[i] & right;

        // there is a bug in Qt, sometimes it leaves the resulting
        // outline open, so just close it explicitly.

        leftPath.closeSubpath();
        rightPath.closeSubpath();

        leftPath = booleanWorkaroundTransform.inverted().map(leftPath);
        rightPath = booleanWorkaroundTransform.inverted().map(rightPath);

        qCritical() << "Path 1 = " << leftPath << leftPath.length();
        qCritical() << "Path 2 = " << rightPath << rightPath.length();


        KoPathShape* leftShape = KoPathShape::createShapeFromPainterPath(leftPath);
        KoPathShape* rightShape = KoPathShape::createShapeFromPainterPath(rightPath);

        if (!leftPath.isEmpty() || !rightPath.isEmpty()) {
            shapesToRemove << m_selectedShapes[i];
        }

        QList<KoPathShape*> shapes;
        shapes << leftShape << rightShape;
        KoShape* referenceShape = m_selectedShapes[i];
        Q_FOREACH(KoShape* shape, shapes) {
            shape->setBackground(referenceShape->background());
            shape->setStroke(referenceShape->stroke());
            shape->setZIndex(referenceShape->zIndex());

            KoShapeContainer *parent = referenceShape->parent();
            tool()->canvas()->shapeController()->addShapeDirect(shape, parent, cmd);

            newSelectedShapes << shape;

        }

    }

    tool()->canvas()->shapeController()->removeShapes(shapesToRemove, cmd);
    new KoKeepShapesSelectedCommand({}, newSelectedShapes, tool()->canvas()->selectedShapesProxy(), true, cmd);


    tool()->canvas()->addCommand(cmd);









}

void CutThroughShapeStrategy::paint(QPainter &painter, const KoViewConverter &converter)
{
    qCritical() << "Cut shape strategy :: paint";
    painter.save();

    painter.setBrush(Qt::darkGray);
    painter.drawLine(converter.documentToView(m_startPoint), converter.documentToView(m_endPoint));
    //painter.drawEllipse(converter.documentToView(m_d->mousePoint), 4, 4);
    //QPointF topLeft = QPointF(min(m_d->startPoint.x(), m_d->endPoint.x()), min(min(m_d->startPoint.x(), m_d->endPoint.x())));
    //QRectF dirtyRect;
    //KisAlgebra2D::accumulateBounds(m_startPoint, &dirtyRect);
    //KisAlgebra2D::accumulateBounds(m_endPoint, &dirtyRect);
    //KisAlgebra2D::accumulateBounds(m_mousePoint + QPointF(-2, -2), &dirtyRect);
    //KisAlgebra2D::accumulateBounds(m_mousePoint + QPointF(2, 2), &dirtyRect);
    //dirtyRect = converter.viewToDocument(dirtyRect);

    //drawCoordsStart(painter, converter);



    painter.restore();
}
