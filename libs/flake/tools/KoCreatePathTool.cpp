/* This file is part of the KDE project
 *
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008-2010 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoCreatePathTool.h"

#include "KoShapeController.h"
#include "KoPointerEvent.h"
#include "KoPathShape.h"
#include "KoLineBorder.h"
#include "KoSelection.h"
#include "commands/KoPathPointMergeCommand.h"
#include "SnapGuideConfigWidget.h"

#include <KNumInput>

#include <QtGui/QPainter>
#include <QtGui/QLabel>

#include "KoCreatePathTool_p.h"


KoCreatePathTool::KoCreatePathTool(KoCanvasBase *canvas)
        : KoToolBase(*(new KoCreatePathToolPrivate(this, canvas)))
{
}

KoCreatePathTool::~KoCreatePathTool()
{
}

void KoCreatePathTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_D(KoCreatePathTool);
    if (d->shape) {
        painter.save();
        paintPath(*(d->shape), painter, converter);
        painter.restore();

        painter.save();

        painter.setTransform(d->shape->absoluteTransformation(&converter) * painter.transform());

        KoShape::applyConversion(painter, converter);

        painter.setPen(Qt::blue);
        painter.setBrush(Qt::white);   //TODO make configurable

        const bool firstPoint = (d->firstPoint == d->activePoint);
        if (d->pointIsDragged || firstPoint) {
            const bool onlyPaintActivePoints = false;
            KoPathPoint::PointTypes paintFlags = KoPathPoint::ControlPoint2;
            if (d->activePoint->activeControlPoint1())
                paintFlags |= KoPathPoint::ControlPoint1;
            d->activePoint->paint(painter, d->handleRadius, paintFlags, onlyPaintActivePoints);
        }

        // paint the first point

        // check if we have to color the first point
        if (d->mouseOverFirstPoint)
            painter.setBrush(Qt::red);   // //TODO make configurable
        else
            painter.setBrush(Qt::white);   //TODO make configurable

        d->firstPoint->paint(painter, d->handleRadius, KoPathPoint::Node);

        painter.restore();
    }

    if (d->hoveredPoint) {
        painter.save();
        painter.setTransform(d->hoveredPoint->parent()->absoluteTransformation(&converter), true);
        KoShape::applyConversion(painter, converter);
        painter.setPen(Qt::blue);
        painter.setBrush(Qt::white);   //TODO make configurable
        d->hoveredPoint->paint(painter, d->handleRadius, KoPathPoint::Node);
        painter.restore();
    }
    painter.save();
    KoShape::applyConversion(painter, converter);
    canvas()->snapGuide()->paint(painter, converter);
    painter.restore();
}

void KoCreatePathTool::paintPath(KoPathShape& pathShape, QPainter &painter, const KoViewConverter &converter)
{
    Q_D(KoCreatePathTool);
    painter.setTransform(pathShape.absoluteTransformation(&converter) * painter.transform());
    painter.save();
    pathShape.paint(painter, converter);
    painter.restore();
    if (pathShape.border()) {
        painter.save();
        pathShape.border()->paint(d->shape, painter, converter);
        painter.restore();
    }
}

void KoCreatePathTool::mousePressEvent(KoPointerEvent *event)
{
    Q_D(KoCreatePathTool);

    if (event->button() == Qt::RightButton || (event->button() == Qt::LeftButton && event->modifiers() & Qt::ShiftModifier)) {
        if (d->shape) {
            // repaint the shape before removing the last point
            canvas()->updateCanvas(d->shape->boundingRect());
            delete d->shape->removePoint(d->shape->pathPointIndex(d->activePoint));

            d->addPathShape();
        }
        // Return as otherwise a point would be added
        return;
    }

    if (d->shape) {
        // the path shape gets closed by clicking on the first point
        if (handleGrabRect(d->firstPoint->point()).contains(event->point)) {
            d->activePoint->setPoint(d->firstPoint->point());
            d->shape->closeMerge();
            // we are closing the path, so reset the existing start path point
            d->existingStartPoint = 0;
            // finish path
            d->addPathShape();
        } else {
            canvas()->updateCanvas(canvas()->snapGuide()->boundingRect());

            QPointF point = canvas()->snapGuide()->snap(event->point, event->modifiers());

            // check whether we hit an start/end node of an existing path
            d->existingEndPoint = d->endPointAtPosition(point);
            if (d->existingEndPoint.isValid() && d->existingEndPoint != d->existingStartPoint) {
                point = d->existingEndPoint.path->shapeToDocument(d->existingEndPoint.point->point());
                d->activePoint->setPoint(point);
                // finish path
                d->addPathShape();
            } else {
                d->activePoint->setPoint(point);
                canvas()->updateCanvas(d->shape->boundingRect());
                canvas()->updateCanvas(canvas()->snapGuide()->boundingRect());
            }
        }
    } else {
        KoPathShape *pathShape = new KoPathShape();
        d->shape=pathShape;
        pathShape->setShapeId(KoPathShapeId);

        KoLineBorder *border = new KoLineBorder(canvas()->resourceManager()->activeBorder());
        border->setColor(canvas()->resourceManager()->foregroundColor().toQColor());

        pathShape->setBorder(border);
        canvas()->updateCanvas(canvas()->snapGuide()->boundingRect());
        QPointF point = canvas()->snapGuide()->snap(event->point, event->modifiers());

        // check whether we hit an start/end node of an existing path
        d->existingStartPoint = d->endPointAtPosition(point);
        if (d->existingStartPoint.isValid()) {
            point = d->existingStartPoint.path->shapeToDocument(d->existingStartPoint.point->point());
        }
        d->activePoint = pathShape->moveTo(point);
        d->firstPoint = d->activePoint;
        canvas()->updateCanvas(handlePaintRect(point));
        canvas()->updateCanvas(canvas()->snapGuide()->boundingRect());

        canvas()->snapGuide()->setEditedShape(pathShape);

        d->angleSnapStrategy = new AngleSnapStrategy(d->angleSnappingDelta);
        canvas()->snapGuide()->addCustomSnapStrategy(d->angleSnapStrategy);
    }

    if (d->angleSnapStrategy)
        d->angleSnapStrategy->setStartPoint(d->activePoint->point());
}

void KoCreatePathTool::mouseDoubleClickEvent(KoPointerEvent *event)
{
    Q_D(KoCreatePathTool);

    //remove handle
    canvas()->updateCanvas(handlePaintRect(event->point));

    if (d->shape) {
        // the first click of the double click created a new point which has the be removed again
        d->shape->removePoint(d->shape->pathPointIndex(d->activePoint));

        d->addPathShape();
    }
}

void KoCreatePathTool::mouseMoveEvent(KoPointerEvent *event)
{
    Q_D(KoCreatePathTool);

    KoPathPoint *endPoint = d->endPointAtPosition(event->point);
    if (d->hoveredPoint != endPoint) {
        if (d->hoveredPoint) {
            QPointF nodePos = d->hoveredPoint->parent()->shapeToDocument(d->hoveredPoint->point());
            canvas()->updateCanvas(handlePaintRect(nodePos));
        }
        d->hoveredPoint = endPoint;
        if (d->hoveredPoint) {
            QPointF nodePos = d->hoveredPoint->parent()->shapeToDocument(d->hoveredPoint->point());
            canvas()->updateCanvas(handlePaintRect(nodePos));
        }
    }

    if (! d->shape) {
        canvas()->updateCanvas(canvas()->snapGuide()->boundingRect());
        canvas()->snapGuide()->snap(event->point, event->modifiers());
        canvas()->updateCanvas(canvas()->snapGuide()->boundingRect());

        d->mouseOverFirstPoint = false;
        return;
    }

    d->mouseOverFirstPoint = handleGrabRect(d->firstPoint->point()).contains(event->point);

    canvas()->updateCanvas(d->shape->boundingRect());
    canvas()->updateCanvas(canvas()->snapGuide()->boundingRect());
    QPointF snappedPosition = canvas()->snapGuide()->snap(event->point, event->modifiers());

    d->repaintActivePoint();
    if (event->buttons() & Qt::LeftButton) {
        d->pointIsDragged = true;
        QPointF offset = snappedPosition - d->activePoint->point();
        d->activePoint->setControlPoint2(d->activePoint->point() + offset);
        // pressing <alt> stops controls points moving symmetrically
        if ((event->modifiers() & Qt::AltModifier) == 0)
            d->activePoint->setControlPoint1(d->activePoint->point() - offset);
        d->repaintActivePoint();
    } else {
        d->activePoint->setPoint(snappedPosition);
    }

    canvas()->updateCanvas(d->shape->boundingRect());
    canvas()->updateCanvas(canvas()->snapGuide()->boundingRect());
}

void KoCreatePathTool::mouseReleaseEvent(KoPointerEvent *event)
{
    Q_D(KoCreatePathTool);

    if (! d->shape || (event->buttons() & Qt::RightButton))
        return;

    d->repaintActivePoint();
    d->pointIsDragged = false;
    KoPathPoint *lastActivePoint = d->activePoint;
    d->activePoint = d->shape->lineTo(event->point);
    // apply symmetric point property if applicable
    if (lastActivePoint->activeControlPoint1() && lastActivePoint->activeControlPoint2()) {
        QPointF diff1 = lastActivePoint->point() - lastActivePoint->controlPoint1();
        QPointF diff2 = lastActivePoint->controlPoint2() - lastActivePoint->point();
        if (qFuzzyCompare(diff1.x(), diff2.x()) && qFuzzyCompare(diff1.y(), diff2.y()))
            lastActivePoint->setProperty(KoPathPoint::IsSymmetric);
    }
    canvas()->snapGuide()->setIgnoredPathPoints( (QList<KoPathPoint*>()<<d->activePoint) );
    if (d->angleSnapStrategy && lastActivePoint->activeControlPoint2()) {
        d->angleSnapStrategy->deactivate();
    }
}

void KoCreatePathTool::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        emit done();
    else
        event->ignore();
}

void KoCreatePathTool::activate(ToolActivation, const QSet<KoShape*> &)
{
    Q_D(KoCreatePathTool);
    useCursor(Qt::ArrowCursor);

    // retrieve the actual global handle radius
    d->handleRadius = canvas()->resourceManager()->handleRadius();

    // reset snap guide
    canvas()->updateCanvas(canvas()->snapGuide()->boundingRect());
    canvas()->snapGuide()->reset();
}

void KoCreatePathTool::deactivate()
{
    Q_D(KoCreatePathTool);

    if (d->shape) {
        canvas()->updateCanvas(handlePaintRect(d->firstPoint->point()));
        canvas()->updateCanvas(d->shape->boundingRect());
        d->firstPoint = 0;
        d->activePoint = 0;
    }
    d->cleanUp();
}

void KoCreatePathTool::resourceChanged(int key, const QVariant & res)
{
    Q_D(KoCreatePathTool);

    switch (key) {
    case KoCanvasResource::HandleRadius: {
        d->handleRadius = res.toUInt();
    }
    break;
    default:
        return;
    }
}

void KoCreatePathTool::addPathShape(KoPathShape *pathShape)
{
    Q_D(KoCreatePathTool);

    KoPathShape *startShape = 0;
    KoPathShape *endShape = 0;
    pathShape->normalize();

    // check if existing start/end points are still valid
    d->existingStartPoint.validate(canvas());
    d->existingEndPoint.validate(canvas());

    if (d->connectPaths(pathShape, d->existingStartPoint, d->existingEndPoint)) {
        if (d->existingStartPoint.isValid())
            startShape = d->existingStartPoint.path;
        if (d->existingEndPoint.isValid() && d->existingEndPoint != d->existingStartPoint)
            endShape = d->existingEndPoint.path;
    }

    QUndoCommand *cmd = canvas()->shapeController()->addShape(pathShape);
    if (cmd) {
        KoSelection *selection = canvas()->shapeManager()->selection();
        selection->deselectAll();
        selection->select(pathShape);
        if (startShape)
            canvas()->shapeController()->removeShape(startShape, cmd);
        if (endShape && startShape != endShape)
            canvas()->shapeController()->removeShape(endShape, cmd);
        canvas()->addCommand(cmd);
    } else {
        canvas()->updateCanvas(pathShape->boundingRect());
        delete pathShape;
    }
}

QMap<QString, QWidget *> KoCreatePathTool::createOptionWidgets()
{
    Q_D(KoCreatePathTool);

    QMap<QString, QWidget *> map;
    SnapGuideConfigWidget *widget = new SnapGuideConfigWidget(canvas()->snapGuide());
    map.insert(i18n("Snapping"), widget);

    QWidget *angleWidget = new QWidget();
    angleWidget->setObjectName("Angle Constraints");
    QGridLayout *layout = new QGridLayout(angleWidget);
    layout->addWidget( new QLabel(i18n("Angle snapping delta"), angleWidget), 0, 0);
    KIntNumInput *angleEdit = new KIntNumInput(d->angleSnappingDelta, angleWidget);
    angleEdit->setRange(1, 360, 1);
    angleEdit->setSuffix(QChar(Qt::Key_degree));
    layout->addWidget( angleEdit, 0, 1);
    map.insert(i18n("Angle Constraints"), angleWidget);

    connect(angleEdit, SIGNAL(valueChanged(int)), this, SLOT(angleDeltaChanged(int)));

    return map;
}

#include <KoCreatePathTool.moc>
