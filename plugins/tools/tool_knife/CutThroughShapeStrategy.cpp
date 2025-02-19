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

    QList<QLineF> gapLines = KisAlgebra2D::getParallelLines(gapLine, 2*m_width);

    qCritical() << "############ Are those lines correct? " << KisAlgebra2D::pointToLineDistSquared(gapLines[0].p1(), gapLines[1]) << KisAlgebra2D::pointToLineDistSquared(gapLines[1].p1(), gapLines[0]) << KisAlgebra2D::pointToLineDistSquared(gapLines[0].p1(), gapLine);

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


    QList<QPainterPath> paths = KisAlgebra2D::getPathsFromRectangleCutThrough(QRectF(outlineRectBiggerInt), leftLine, rightLine);
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

        //qCritical() << "Path 1 = " << leftPath << leftPath.length();
        //qCritical() << "Path 2 = " << rightPath << rightPath.length();


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
