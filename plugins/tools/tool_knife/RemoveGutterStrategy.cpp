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




RemoveGutterStrategy::RemoveGutterStrategy(KoToolBase *tool, KoSelection *selection, const QList<KoShape *> &shapes, QPointF startPoint)
    : KoInteractionStrategy(tool)
    , m_startPoint(startPoint)
    , m_endPoint(startPoint)
{
    m_selectedShapes = selection->selectedEditableShapes();
    m_allShapes = shapes;
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
    QLineF l = QLine(QPoint(), QPoint(50, 50));

    l = tool()->canvas()->viewConverter()->viewToDocument().map(l);
    dirtyRect = kisGrowRect(dirtyRect, l.length()); // twice as much as it should need to account for lines showing the effect

    QRectF accumulatedWithPrevious = m_previousLineDirtyRect | dirtyRect;

    tool()->canvas()->updateCanvas(accumulatedWithPrevious);
    m_previousLineDirtyRect = dirtyRect;
}

#ifdef KNIFE_DEBUG
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
#endif

void RemoveGutterStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    tool()->canvas()->updateCanvas(m_previousLineDirtyRect);


    KisCanvas2 *kisCanvas = static_cast<KisCanvas2 *>(tool()->canvas());
    KIS_SAFE_ASSERT_RECOVER_RETURN(kisCanvas);
    const QTransform booleanWorkaroundTransform = KritaUtils::pathShapeBooleanSpaceWorkaround(kisCanvas->image());

    QList<QPainterPath> srcOutlines;
    QList<QPainterPath> srcOutlinesOutside;

    if (m_allShapes.length() == 0) {
        qCritical() << "No shapes on the layer";
        return;
    }

    QList<bool> isSelected = QList<bool>();
    isSelected.reserve(m_allShapes.length());
    for (int i = 0; i < m_allShapes.length(); i++) {
        isSelected.append(m_selectedShapes.contains(m_allShapes[i]));
    }

    QLineF mouseLine = QLineF(m_startPoint, m_endPoint);
    QRectF lineRect = KisAlgebra2D::createRectFromCorners(m_startPoint, m_endPoint);

    mouseLine = booleanWorkaroundTransform.map(mouseLine);
    lineRect = KisAlgebra2D::createRectFromCorners(mouseLine);

#ifdef KNIFE_DEBUG
    convertShapeToDebugArray(mouseLine);
    convertShapeToDebugArray(lineRect);
#endif


    QList<int> indexes;
    QList<int> indexesOutside;

    for (int i = 0; i < m_allShapes.length(); i++) {
        KoShape* shape = m_allShapes[i];
        QPainterPath outlineHere =
            booleanWorkaroundTransform.map(
            shape->absoluteTransformation().map(
                shape->outline()));
#ifdef KNIFE_DEBUG
        convertShapeToDebugArray(outlineHere);
#endif
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
#ifdef KNIFE_DEBUG
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
        // TODO: this if should end here; code below adds a debug line showing the mouse line

        QPainterPath newLineShape = QPainterPath();
        newLineShape.moveTo(mouseLine.p1());
        newLineShape.lineTo(mouseLine.p2());

        newLineShape = booleanWorkaroundTransform.inverted().map(newLineShape);
        KoPathShape* newLineShapeToAdd = KoPathShape::createShapeFromPainterPath(newLineShape);

        newLineShapeToAdd->setBackground(m_allShapes[0]->background());
        newLineShapeToAdd->setStroke(m_allShapes[0]->stroke());
        newLineShapeToAdd->setZIndex(m_allShapes[0]->zIndex() + 100);



        KUndo2Command *cmd = new KUndo2Command(kundo2_i18n("Knife tool: cut through shapes"));
        tool()->canvas()->shapeController()->addShapeDirect(newLineShapeToAdd, m_allShapes[0]->parent(), cmd);
        tool()->canvas()->addCommand(cmd);

#endif

        return;
    }


#ifdef KNIFE_DEBUG
    qCritical() << "Found two shapes.";
    qCritical() << "Shape 1:";
    convertShapeToDebugArray(srcOutlines[shapeNewIndexes[0]]);
    qCritical() << ppVar(srcOutlines[shapeNewIndexes[0]].toFillPolygon());

    qCritical() << "Shape 2:";
    convertShapeToDebugArray(srcOutlines[shapeNewIndexes[1]]);
    qCritical() << ppVar(srcOutlines[shapeNewIndexes[1]].toFillPolygon());
#endif

    if (shapeNewIndexes[0] == shapeNewIndexes[1]) {
        // the same shape
        // gotta ensure the mouseline starts and ends inside
        bool insideP1 = KisAlgebra2D::isInsideShape(srcOutlines[shapeNewIndexes[0]], mouseLine.p1());
        bool insideP2 = KisAlgebra2D::isInsideShape(srcOutlines[shapeNewIndexes[0]], mouseLine.p2());
        if (!insideP1 || !insideP2) {
            // it's from outside, it doesn't go over a gutter, then
            return;
        }
    }



    QPainterPath result = KisAlgebra2D::removeGutterSmart(srcOutlines[shapeNewIndexes[0]], lineSegmentIndexes[0], srcOutlines[shapeNewIndexes[1]], lineSegmentIndexes[1], shapeNewIndexes[0]==shapeNewIndexes[1]);

#ifdef KNIFE_DEBUG
    qCritical() << "Finally got a result:";
    convertShapeToDebugArray(result);
    qCritical() << ppVar(result.toFillPolygon());
#endif

    QList<KoShape*> resultSelectedShapes;

    Q_FOREACH(int index, indexesOutside) {
        if (isSelected[index]) {
            resultSelectedShapes << m_allShapes[index];
        }
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


    KoShape* referenceShape = m_allShapes[shapeOrigIndexes[0]];
    KoPathShape* koPathReferenceShape = dynamic_cast<KoPathShape*>(referenceShape);
    resultShape->setBackground(referenceShape->background());
    resultShape->setStroke(referenceShape->stroke());
    resultShape->setZIndex(referenceShape->zIndex());
    if (koPathReferenceShape) {
        resultShape->setFillRule(koPathReferenceShape->fillRule());
    }


    KoShapeContainer *parent = referenceShape->parent();
    tool()->canvas()->shapeController()->addShapeDirect(resultShape, parent, cmd);

    if (isSelected[shapeOrigIndexes[0]] || isSelected[shapeOrigIndexes[1]]) { // if either is selected
        resultSelectedShapes << resultShape;
    }

    QList<KoShape*> shapesToRemove;
    shapesToRemove << m_allShapes[shapeOrigIndexes[0]];
    if (shapeOrigIndexes[0] != shapeOrigIndexes[1]) { // there is a good reason in the workflow to allow doing this operation on the same shape
        shapesToRemove << m_allShapes[shapeOrigIndexes[1]];
    }


    tool()->canvas()->shapeController()->removeShapes(shapesToRemove, cmd);
    new KoKeepShapesSelectedCommand({}, resultSelectedShapes, tool()->canvas()->selectedShapesProxy(), true, cmd);
    tool()->canvas()->addCommand(cmd);


}

void RemoveGutterStrategy::paint(QPainter &painter, const KoViewConverter &converter)
{
    painter.save();
    painter.setPen(QPen(QBrush(Qt::darkGray), 2));

    QLineF line = converter.documentToView().map(QLineF(m_startPoint, m_endPoint));
    if (line.length() > 0) {
        QPointF vector = line.p2() - line.p1();
        vector = vector/line.length();
        int arrowLength = 15;
        int arrowThickness = 5;

        QPointF before = line.p1() - vector*arrowLength;
        QPointF after = line.p2() + vector*arrowLength;

        QPointF perpendicular = QPointF(vector.y(), -vector.x());

        painter.drawLine(QPointF(before + arrowThickness*perpendicular), line.p1());
        painter.drawLine(QPointF(before - arrowThickness*perpendicular), line.p1());

        painter.drawLine(QPointF(after + arrowThickness*perpendicular), line.p2());
        painter.drawLine(QPointF(after - arrowThickness*perpendicular), line.p2());


    }
    painter.drawLine(line);

    painter.restore();

}
