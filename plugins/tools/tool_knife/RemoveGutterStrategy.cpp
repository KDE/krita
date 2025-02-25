/*
 *  SPDX-FileCopyrightText: 2025 Agata Cacko
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "RemoveGutterStrategy.h"


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




RemoveGutterStrategy::RemoveGutterStrategy(KoToolBase *tool, KoSelection *selection, QPointF startPoint)
    : KoInteractionStrategy(tool)
    , m_startPoint(startPoint)
    , m_endPoint(startPoint)
{
    m_selectedShapes = selection->selectedEditableShapes();
}

RemoveGutterStrategy::~RemoveGutterStrategy()
{

}

KUndo2Command *RemoveGutterStrategy::createCommand()
{
    return 0;
}

void RemoveGutterStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    m_endPoint = mouseLocation;
    QRectF dirtyRect;
    KisAlgebra2D::accumulateBounds(m_startPoint, &dirtyRect);
    KisAlgebra2D::accumulateBounds(m_endPoint, &dirtyRect);
    dirtyRect = kisGrowRect(dirtyRect, 10); // twice as much as it should need to account for lines showing the effect

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


void convertShapeToDebugArray(const QPainterPath& shape) {
    // useful to use with Geogebra
    // TODO: to remove later
    for (int i = 0; i < shape.elementCount(); i++) {
        QPainterPath::Element el = shape.elementAt(i);
        qCritical() << el.x << "\t" << el.y << "\t" << el.type;
    }

}

void convertShapeToDebugArray(const QRectF& rect) {
    // useful to use with Geogebra
    // TODO: to remove later
    QPolygonF poly = QPolygonF(rect);
    for (int i = 0; i < poly.length(); i++) {
        QPointF p = poly[i];
        qCritical() << p.x() << "\t" << p.y();
    }

}

void convertShapeToDebugArray(const QLineF& line) {
    // useful to use with Geogebra
    // TODO: to remove later
    qCritical() << line.p1().x() << "\t" << line.p1().y();
    qCritical() << line.p2().x() << "\t" << line.p2().y();

}

void RemoveGutterStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{

    // so, first find all the shapes whose bounding rect is nonempty when intersected with the line bounding rect
    // then for every one of them find a path that's crossing the mouse line
    // save info for later
    // ensure you have only two paths belonging to two different shapes
    // WAIT
    // what about the situation o T
    // There are three (four?) cases. Gotta look a the picture to understand it!
    // write it down later, after implementing


    // also: when moving the gutter or gutter end, remember about keeping the gutter of the same


    qCritical() << "vvvvvvvvvvvvvvvvvvvvvvvvv REMOVING A GUTTER vvvvvvvvvvvvvvvvvvvvvvvvv";


    KisCanvas2 *kisCanvas = static_cast<KisCanvas2 *>(tool()->canvas());
    KIS_SAFE_ASSERT_RECOVER_RETURN(kisCanvas);
    const QTransform booleanWorkaroundTransform = KritaUtils::pathShapeBooleanSpaceWorkaround(kisCanvas->image());

    QList<QPainterPath> srcOutlines;
    QList<QPainterPath> srcOutlinesOutside;

    //QRectF outlineRect;

    if (m_selectedShapes.length() == 0) {
        qCritical() << "No shapes are selected";
        return;
    }

    QLineF mouseLine = QLineF(m_startPoint, m_endPoint);
    QRectF lineRect = KisAlgebra2D::createRectFromCorners(m_startPoint, m_endPoint);

    qCritical() << "Line and line rect before boolean trans: " << mouseLine << lineRect;
    mouseLine = booleanWorkaroundTransform.map(mouseLine);
    lineRect = KisAlgebra2D::createRectFromCorners(mouseLine.p1(), mouseLine.p2());


    qCritical() << "Line and line rect after boolean trans: " << mouseLine << lineRect;
    qCritical() << "Line:";
    convertShapeToDebugArray(mouseLine);
    qCritical() << "Rect:";
    convertShapeToDebugArray(lineRect);
    qCritical() << "end.";


    QList<int> indexes;
    QList<int> indexesOutside;

    for (int i = 0; i < m_selectedShapes.length(); i++) {
        KoShape* shape = m_selectedShapes[i];
        QPainterPath outlineHere =
            booleanWorkaroundTransform.map(
            shape->absoluteTransformation().map(
                shape->outline()));

        qCritical() << "> Selected path (in boolean): ";
        convertShapeToDebugArray(outlineHere);

        if (outlineHere.boundingRect().intersects(lineRect)) {
            srcOutlines << outlineHere;
            indexes << i;
            //outlineRect |= outlineHere.boundingRect();
        } else {
            srcOutlinesOutside << outlineHere;
            indexesOutside << i;
        }
    }

    if (srcOutlines.isEmpty()) {
        qCritical() << "The list of possible shapes is empty";
        return;
    }


    QList<int> shapeNewIndexes;
    QList<int> shapeOrigIndexes;
    QList<int> lineSegmentIndexes;


    for (int i = 0; i < srcOutlines.length(); i++) {
        QList<int> lineIndexes = KisAlgebra2D::getLineSegmentCrossingLineIndexes(mouseLine, srcOutlines[i]);
        int shapeOrigIndex = indexes[i];

        if (lineIndexes.length() > 0) {
            Q_FOREACH(int lineIndex, lineIndexes) {
                shapeNewIndexes << i;
                lineSegmentIndexes << lineIndex;
                shapeOrigIndexes << shapeOrigIndex;
            }
        } else {
            srcOutlinesOutside << srcOutlines[i];
            indexesOutside << shapeOrigIndex;
        }
    }

    if (shapeNewIndexes.length() != 2) {

        qCritical() << "Shape indexes count isn't correct: " << ppVar(shapeNewIndexes.length()) << ppVar(lineSegmentIndexes.length());
        qCritical() << "Mouse line in used coordinates: " << mouseLine;
        qCritical() << "Number of shapes even considered: " << srcOutlines.length();
        Q_FOREACH(QPainterPath path, srcOutlines) {
            qCritical() << "> A path: ";
            convertShapeToDebugArray(path);
        }
        qCritical() << "That's the end of shapes considered.";
        qCritical() << "Shapes not considered: " << srcOutlinesOutside.length();
        Q_FOREACH(QPainterPath path, srcOutlinesOutside) {
            qCritical() << "> A path: ";
            convertShapeToDebugArray(path);
        }
        qCritical() << "That's the end of shapes not considered.";
        for(int i = 0; i < shapeNewIndexes.length(); i++) {
            int index = shapeNewIndexes[i];
            qCritical() << "Shape: ";
            convertShapeToDebugArray(srcOutlines[index]);
            qCritical() << "Line index: " << lineSegmentIndexes[i] << " meaning it's: " << srcOutlines[index].elementAt(KisAlgebra2D::wrapValue(lineSegmentIndexes[i], 0, srcOutlines[index].elementCount()))
                        << srcOutlines[index].elementAt(KisAlgebra2D::wrapValue(lineSegmentIndexes[i] + 1, 0, srcOutlines[index].elementCount()));
        }

        qCritical() << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF REMOVING A GUTTER, UNSUCCESSFUL";

        // TODO: this if should end here; code below adds a debug line showing the mouse line

        QPainterPath newLineShape = QPainterPath();
        newLineShape.moveTo(mouseLine.p1());
        newLineShape.lineTo(mouseLine.p2());

        newLineShape = booleanWorkaroundTransform.inverted().map(newLineShape);
        KoPathShape* newLineShapeToAdd = KoPathShape::createShapeFromPainterPath(newLineShape);

        newLineShapeToAdd->setBackground(m_selectedShapes[0]->background());
        newLineShapeToAdd->setStroke(m_selectedShapes[0]->stroke());
        newLineShapeToAdd->setZIndex(m_selectedShapes[0]->zIndex() + 100);



        KUndo2Command *cmd = new KUndo2Command(kundo2_i18n("Knife tool: cut through shapes"));
        tool()->canvas()->shapeController()->addShapeDirect(newLineShapeToAdd, m_selectedShapes[0]->parent(), cmd);
        tool()->canvas()->addCommand(cmd);

        return;
    }

    // KisAlgebra2D::getLineSegmentCrossingLineIndexes(QLineF const&, QPainterPath const&)

    qCritical() << "Found two shapes.";
    qCritical() << "Shape 1:";
    convertShapeToDebugArray(srcOutlines[shapeNewIndexes[0]]);
    qCritical() << "Shape 2:";
    convertShapeToDebugArray(srcOutlines[shapeNewIndexes[1]]);



    QPainterPath result = KisAlgebra2D::removeGutterSmart(srcOutlines[shapeNewIndexes[0]], lineSegmentIndexes[0], srcOutlines[shapeNewIndexes[1]], lineSegmentIndexes[1]);

    QList<KoShape*> resultShapes;

    qCritical() << "Indexes that are supposed to be left:";
    Q_FOREACH(int index, indexesOutside) {
        qCritical() << index;
    }
    qCritical() << "End.";
    qCritical() << "Indexes that compose the result shape: " << shapeOrigIndexes[0] << shapeOrigIndexes[1];
    qCritical() << "All indexes are up to: " << m_selectedShapes.length();


    Q_FOREACH(int index, indexesOutside) {
        resultShapes << m_selectedShapes[index];
    }

    // since we can't really decide which style to use, we're gonna use the style of the first found shape.
    // if the user doesn't like it, they can change it.


    result = booleanWorkaroundTransform.inverted().map(result);
    KoPathShape* resultShape = KoPathShape::createShapeFromPainterPath(result);

    if (resultShape->boundingRect().isEmpty()) {
        return;
    }

    KUndo2Command *cmd = new KUndo2Command(kundo2_i18n("Knife tool: remove a gutter"));



    new KoKeepShapesSelectedCommand(m_selectedShapes, {}, tool()->canvas()->selectedShapesProxy(), false, cmd);


    KoShape* referenceShape = m_selectedShapes[shapeOrigIndexes[0]];
    KoPathShape* koPathReferenceShape = dynamic_cast<KoPathShape*>(referenceShape);
    resultShape->setBackground(referenceShape->background());
    resultShape->setStroke(referenceShape->stroke());
    resultShape->setZIndex(referenceShape->zIndex());
    if (koPathReferenceShape) {
        resultShape->setFillRule(koPathReferenceShape->fillRule());
    }


    KoShapeContainer *parent = referenceShape->parent();
    tool()->canvas()->shapeController()->addShapeDirect(resultShape, parent, cmd);

    resultShapes << resultShape;

    QList<KoShape*> shapesToRemove;
    shapesToRemove << m_selectedShapes[shapeOrigIndexes[0]];
    if (shapeOrigIndexes[0] != shapeOrigIndexes[1]) { // there is a good reason in the workflow to allow doing this operation on the same shape
        shapesToRemove << m_selectedShapes[shapeOrigIndexes[1]];
    }




    tool()->canvas()->shapeController()->removeShapes(shapesToRemove, cmd);

    new KoKeepShapesSelectedCommand({}, resultShapes, tool()->canvas()->selectedShapesProxy(), true, cmd);


    // this line sometimes causes a segfault for some reason???
    tool()->canvas()->addCommand(cmd);



    qCritical() << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF REMOVING A GUTTER, SUCCESSFUL, yay! :) ";




}

void RemoveGutterStrategy::paint(QPainter &painter, const KoViewConverter &converter)
{
    painter.save();
    painter.setPen(QPen(QBrush(Qt::black), 5));
    painter.drawLine(converter.documentToView().map(QLineF(m_startPoint, m_endPoint)));
    painter.restore();

}
