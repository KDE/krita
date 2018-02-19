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
#include "KoCreatePathTool_p.h"

#include <KoUnit.h>
#include "KoPointerEvent.h"
#include "KoPathShape.h"
#include "KoSelection.h"
#include "KoDocumentResourceManager.h"
#include "KoShapePaintingContext.h"
#include "KoShapeStroke.h"
#include "KoCanvasBase.h"
#include "kis_int_parse_spin_box.h"
#include <KoColor.h>
#include "kis_canvas_resource_provider.h"
#include <KoShapeHandlesCollection.h>
#include <KoFlakeUtils.h>
#include "kis_painting_tweaks.h"

#include <klocalizedstring.h>

#include <QSpinBox>
#include <QPainter>
#include <QLabel>
#include <QGridLayout>
#include <QCheckBox>


KoCreatePathTool::KoCreatePathTool(KoCanvasBase *canvas)
    : KoToolBase(*(new KoCreatePathToolPrivate(this, canvas)))
{
}

KoCreatePathTool::~KoCreatePathTool()
{
}

QVector<KoFlake::HandlesRecord> KoCreatePathTool::handlesCollection() const
{
    Q_D(const KoCreatePathTool);

    QVector<KoFlake::HandlesRecord> result;

    if (pathStarted()) {
        const bool firstPointActive = d->firstPoint == d->activePoint;

        if (d->pointIsDragged) {
            const bool onlyPaintActivePoints = false;
            KoPathPoint::PointTypes paintFlags = KoPathPoint::ControlPoint2;

            if (d->activePoint->activeControlPoint1()) {
                paintFlags |= KoPathPoint::ControlPoint1;
            }

            result << KoFlake::HandlesRecord(d->activePoint->parent(),
                                             KisHandleStyle::highlightedPrimaryHandles(),
                                             d->activePoint->handles(paintFlags, onlyPaintActivePoints));
        }

        if (!(d->pointIsDragged && firstPointActive)) {
            result << KoFlake::HandlesRecord(d->firstPoint->parent(),
                                             d->mouseOverFirstPoint ?
                                                 KisHandleStyle::highlightedPrimaryHandles() :
                                                 KisHandleStyle::primarySelection(),
                                             d->firstPoint->handles(KoPathPoint::Node));
        }
    }

    if (d->hoveredPoint) {
        result << KoFlake::HandlesRecord(d->hoveredPoint->parent(),
                                         KisHandleStyle::highlightedPrimaryHandles(),
                                         d->hoveredPoint->handles(KoPathPoint::Node));
    }

    return result;
}

void KoCreatePathTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_D(KoCreatePathTool);

    if (pathStarted()) {
        KisPaintingTweaks::StateSaver saver(&painter);
        paintPath(*(d->shape), painter, converter);
    }

    KoShapeHandlesCollection collection;
    collection.addHandles(handlesCollection());
    collection.drawHandles(&painter, converter, d->handleRadius);

    {
        KisPaintingTweaks::StateSaver saver(&painter);
        KoShape::applyConversion(painter, converter);
        canvas()->snapGuide()->paint(painter, converter);
    }
}

void KoCreatePathTool::paintPath(KoPathShape& pathShape, QPainter &painter, const KoViewConverter &converter)
{
    Q_D(KoCreatePathTool);

    // applyConversion will be applied later in shape's methods
    painter.setTransform(pathShape.absoluteTransformation(&converter) * painter.transform());

    {
        KisPaintingTweaks::StateSaver saver(&painter);
        KoShapePaintingContext paintContext; //FIXME
        pathShape.paint(painter, converter, paintContext);
    }


    if (pathShape.stroke()) {
        KisPaintingTweaks::StateSaver saver(&painter);
        pathShape.stroke()->paint(d->shape, painter, converter);
    }
}

void KoCreatePathTool::mousePressEvent(KoPointerEvent *event)
{
    Q_D(KoCreatePathTool);

    //Right click removes last point
    if (event->button() == Qt::RightButton) {
        removeLastPoint();
        return;
    }

    KoCanvasUpdatesCollector pendingUpdates(canvas());
    setMouseOverFirstPoint(d->shape && isOverFirstPoint(event->point), pendingUpdates);

    const bool haveCloseModifier = listeningToModifiers() && (event->modifiers() & Qt::ShiftModifier);

    if ((event->button() == Qt::LeftButton) && haveCloseModifier && !d->mouseOverFirstPoint) {
        endPathWithoutLastPoint(pendingUpdates);
        return;
    }

    d->finishAfterThisPoint = false;
    bool shouldEndPath = false;

    pendingUpdates.addUpdate(shapeAndSnapGuidesRect(d->shape));
    pendingUpdates.addUpdate(activePointUpdateRects());

    const QPointF snappedPoint = canvas()->snapGuide()->snap(event->point, event->modifiers());

    if (pathStarted()) {
        if (d->mouseOverFirstPoint) {
            d->activePoint->setPoint(d->firstPoint->point());

            if (haveCloseModifier) {
                d->shape->closeMerge();
                // we are closing the path, so reset the existing start path point
                d->existingStartPoint = 0;
                shouldEndPath = true;
            } else {
                // the path shape will get closed when the user releases
                // the mouse button
                d->finishAfterThisPoint = true;
            }
        } else {
            QPointF newPoint = snappedPoint;

            // check whether we hit an start/end node of an existing path
            d->existingEndPoint =
                KoFlake::findNearestPathEndPoint(newPoint,
                                              canvas()->shapeManager(),
                                              grabSensitivity(),
                                              *canvas()->viewConverter());

            if (d->existingEndPoint.isValid() && d->existingEndPoint != d->existingStartPoint) {
                newPoint = d->existingEndPoint.path->shapeToDocument(d->existingEndPoint.point->point());
                shouldEndPath = true;
            }

            d->activePoint->setPoint(newPoint);
        }
    } else {
        KoPathShape *pathShape = new KoPathShape();
        d->shape = pathShape;
        pathShape->setShapeId(KoPathShapeId);

        KoShapeStrokeSP stroke(new KoShapeStroke());
        const qreal size = canvas()->resourceManager()->resource(KisCanvasResourceProvider::Size).toReal();

        stroke->setLineWidth(canvas()->unit().fromUserValue(size));
        stroke->setColor(canvas()->resourceManager()->foregroundColor().toQColor());

        pathShape->setStroke(stroke);

        QPointF newPoint = snappedPoint;

        // check whether we hit an start/end node of an existing path
        d->existingStartPoint = KoFlake::findNearestPathEndPoint(newPoint,
                                                              canvas()->shapeManager(),
                                                              grabSensitivity(),
                                                              *canvas()->viewConverter());

        if (d->existingStartPoint.isValid()) {
            newPoint = d->existingStartPoint.path->shapeToDocument(d->existingStartPoint.point->point());
        }

        d->activePoint = pathShape->moveTo(newPoint);
        d->firstPoint = d->activePoint;
        setMouseOverFirstPoint(true, pendingUpdates);

        canvas()->snapGuide()->setAdditionalEditedShape(pathShape);

        d->angleSnapStrategy = new AngleSnapStrategy(d->angleSnappingDelta, d->angleSnapStatus);
        canvas()->snapGuide()->addCustomSnapStrategy(d->angleSnapStrategy);
    }

    pendingUpdates.addUpdate(shapeAndSnapGuidesRect(d->shape));
    pendingUpdates.addUpdate(activePointUpdateRects());

    if (shouldEndPath) {
        endPath(pendingUpdates);
    } else {
        d->dragStartPoint = snappedPoint;
    }

    if (d->angleSnapStrategy) {
        d->angleSnapStrategy->setStartPoint(d->activePoint->point());
    }
}

bool KoCreatePathTool::listeningToModifiers()
{
    Q_D(KoCreatePathTool);
    return d->listeningToModifiers;
}

bool KoCreatePathTool::pathStarted() const
{
    Q_D(const KoCreatePathTool);
    return ((bool) d->shape);
}

void KoCreatePathTool::mouseDoubleClickEvent(KoPointerEvent *event)
{
    Q_UNUSED(event);

    KoCanvasUpdatesCollector pendingUpdates(canvas());
    endPathWithoutLastPoint(pendingUpdates);
}

void KoCreatePathTool::mouseMoveEvent(KoPointerEvent *event)
{
    Q_D(KoCreatePathTool);

    KoCanvasUpdatesCollector pendingUpdates(canvas());

    KoPathPoint *endPoint =
        KoFlake::findNearestPathEndPoint(event->point,
                                      canvas()->shapeManager(),
                                      grabSensitivity(),
                                      *canvas()->viewConverter());

    if (d->hoveredPoint != endPoint) {
        pendingUpdates.addUpdate(pathPointUpdateRects(d->hoveredPoint, KoPathPoint::Node));
        d->hoveredPoint = endPoint;
        pendingUpdates.addUpdate(pathPointUpdateRects(d->hoveredPoint, KoPathPoint::Node));
    }

    if (!pathStarted()) {
        pendingUpdates.addUpdate(canvas()->snapGuide()->boundingRect());
        canvas()->snapGuide()->snap(event->point, event->modifiers());
        pendingUpdates.addUpdate(canvas()->snapGuide()->boundingRect());

        setMouseOverFirstPoint(false, pendingUpdates);
        return;
    }

    setMouseOverFirstPoint(isOverFirstPoint(event->point), pendingUpdates);

    // --- here we are guaranteed to have a shape created ---

    pendingUpdates.addUpdate(canvas()->snapGuide()->boundingRect());
    const QPointF snappedPosition = canvas()->snapGuide()->snap(event->point, event->modifiers());
    pendingUpdates.addUpdate(canvas()->snapGuide()->boundingRect());

    pendingUpdates.addUpdate(activePointUpdateRects());

    if (event->buttons() & Qt::LeftButton) {
        if (d->pointIsDragged ||
            viewDistanceFromDocPoints(snappedPosition, d->dragStartPoint) > grabSensitivity()) {

            d->pointIsDragged = true;
            QPointF offset = snappedPosition - d->activePoint->point();
            d->activePoint->setControlPoint2(d->activePoint->point() + offset);

            // pressing <alt> stops controls points moving symmetrically
            if ((event->modifiers() & Qt::AltModifier) == 0) {
                d->activePoint->setControlPoint1(d->activePoint->point() - offset);
            }
        }
    } else if (d->shape->pointCount() > 1) {
        d->activePoint->setPoint(snappedPosition);
    }

    pendingUpdates.addUpdate(activePointUpdateRects());
}

void KoCreatePathTool::mouseReleaseEvent(KoPointerEvent *event)
{
    Q_D(KoCreatePathTool);

    if (!d->shape || (event->buttons() & Qt::RightButton)) return;

    d->listeningToModifiers = true; // After the first press-and-release

    KoCanvasUpdatesCollector pendingUpdates(canvas());

    pendingUpdates.addUpdate(shapeAndSnapGuidesRect(d->shape));
    pendingUpdates.addUpdate(activePointUpdateRects());

    d->pointIsDragged = false;

    KoPathPoint *lastActivePoint = d->activePoint;

    if (!d->finishAfterThisPoint) {
        d->activePoint = d->shape->lineTo(event->point);
        canvas()->snapGuide()->setIgnoredPathPoints((QList<KoPathPoint*>() << d->activePoint));
    }

    // apply symmetric point property if applicable
    if (lastActivePoint->activeControlPoint1() && lastActivePoint->activeControlPoint2()) {
        QPointF diff1 = lastActivePoint->point() - lastActivePoint->controlPoint1();
        QPointF diff2 = lastActivePoint->controlPoint2() - lastActivePoint->point();
        if (qFuzzyCompare(diff1.x(), diff2.x()) && qFuzzyCompare(diff1.y(), diff2.y())) {
            lastActivePoint->setProperty(KoPathPoint::IsSymmetric);
        }
    }

    if (d->finishAfterThisPoint) {
        d->firstPoint->setControlPoint1(d->activePoint->controlPoint1());
        delete d->shape->removePoint(d->shape->pathPointIndex(d->activePoint));
        d->activePoint = d->firstPoint;
        d->shape->closeMerge();

        // we are closing the path, so reset the existing start path point
        d->existingStartPoint = 0;
        // finish path
        endPath(pendingUpdates);
    }

    if (d->angleSnapStrategy && lastActivePoint->activeControlPoint2()) {
        d->angleSnapStrategy->deactivate();
    }

    pendingUpdates.addUpdate(shapeAndSnapGuidesRect(d->shape));
    pendingUpdates.addUpdate(activePointUpdateRects());
}

void KoCreatePathTool::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        emit done();
    } else {
        event->ignore();
    }
}

void KoCreatePathTool::endPath(KoCanvasUpdatesCollector &pendingUpdates)
{
    Q_D(KoCreatePathTool);
    d->addPathShape(pendingUpdates);
}

void KoCreatePathTool::endPathWithoutLastPoint(KoCanvasUpdatesCollector &pendingUpdates)
{
    Q_D(KoCreatePathTool);

    if (d->shape) {
        pendingUpdates.addUpdate(d->shape->boundingRect());

        delete d->shape->removePoint(d->shape->pathPointIndex(d->activePoint));

        d->addPathShape(pendingUpdates);
    }
}

void KoCreatePathTool::cancelPath()
{
    Q_D(KoCreatePathTool);

    KoCanvasUpdatesCollector pendingUpdates(canvas());
    d->cleanUp(d->shape ? d->shape->boundingRect() : QRectF(), pendingUpdates);
}

void KoCreatePathTool::removeLastPoint()
{
    Q_D(KoCreatePathTool);

    if (d->shape) {
        KoPathPointIndex lastPointIndex = d->shape->pathPointIndex(d->activePoint);

        if (lastPointIndex.second > 1) {
            lastPointIndex.second--;

            KoPathPoint *pointToDelete = d->shape->pointByIndex(lastPointIndex);

            KoCanvasUpdatesCollector pendingUpdates(canvas());

            KoShapeHandlesCollection collection;
            collection.addHandles(d->shape,
                                  KisHandleStyle::highlightedPrimaryHandles(),
                                  pointToDelete->handles(KoPathPoint::All));

            pendingUpdates.addUpdate(collection.updateDocRects(d->handleRadius));
            pendingUpdates.addUpdate(d->shape->boundingRect());

            delete d->shape->removePoint(lastPointIndex);
            d->hoveredPoint = 0;

            pendingUpdates.addUpdate(d->shape->boundingRect());
        }
    }
}

void KoCreatePathTool::activate(ToolActivation activation, const QSet<KoShape*> &shapes)
{
    KoToolBase::activate(activation, shapes);

    Q_D(KoCreatePathTool);
    useCursor(Qt::ArrowCursor);

    // retrieve the actual global handle radius
    d->handleRadius = handleRadius();

    // reset snap guide
    KoCanvasUpdatesCollector pendingUpdates(canvas());

    pendingUpdates.addUpdate(canvas()->snapGuide()->boundingRect());
    canvas()->snapGuide()->reset();
    pendingUpdates.addUpdate(canvas()->snapGuide()->boundingRect());
}

void KoCreatePathTool::deactivate()
{
    cancelPath();
    KoToolBase::deactivate();
}

void KoCreatePathTool::documentResourceChanged(int key, const QVariant & res)
{
    Q_D(KoCreatePathTool);

    switch (key) {
    case KoDocumentResourceManager::HandleRadius: {
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
        if (d->existingStartPoint.isValid()) {
            startShape = d->existingStartPoint.path;
        }
        if (d->existingEndPoint.isValid() && d->existingEndPoint != d->existingStartPoint) {
            endShape = d->existingEndPoint.path;
        }
    }

    KUndo2Command *cmd = canvas()->shapeController()->addShape(pathShape, 0);
    if (cmd) {
        KoSelection *selection = canvas()->shapeManager()->selection();
        selection->deselectAll();
        selection->select(pathShape);
        if (startShape) {
            canvas()->shapeController()->removeShape(startShape, cmd);
        }
        if (endShape && startShape != endShape) {
            canvas()->shapeController()->removeShape(endShape, cmd);
        }
        canvas()->addCommand(cmd);
    } else {
        canvas()->updateCanvas(pathShape->boundingRect());
        delete pathShape;
    }
}

QVector<QRectF> KoCreatePathTool::shapeAndSnapGuidesRect(KoShape *shape) const
{
    QVector<QRectF> result;

    result << canvas()->snapGuide()->boundingRect();
    if (shape) {
        result << shape->boundingRect();
    }
    return result;
}

QVector<QRectF> KoCreatePathTool::pathPointUpdateRects(KoPathPoint *pt, KoPathPoint::PointTypes types, bool activeOnly) const
{
    Q_D(const KoCreatePathTool);

    QVector<QRectF> result;

    if (pt) {
        KoShapeHandlesCollection collection;
        collection.addHandles(pt->parent(), KisHandleStyle::inheritStyle(), pt->handles(types, activeOnly));
        result = collection.updateDocRects(d->handleRadius);
    }

    return result;
}

QVector<QRectF> KoCreatePathTool::activePointUpdateRects() const
{
    Q_D(const KoCreatePathTool);

    QVector<QRectF> result;

    if (d->activePoint && d->pointIsDragged) {
        const bool onlyPaintActivePoints = false;
        KoPathPoint::PointTypes paintFlags = KoPathPoint::ControlPoint2;

        if (d->activePoint->activeControlPoint1()) {
            paintFlags |= KoPathPoint::ControlPoint1;
        }

        result = pathPointUpdateRects(d->activePoint, paintFlags, onlyPaintActivePoints);
    }

    return result;
}

void KoCreatePathTool::setMouseOverFirstPoint(bool value, KoCanvasUpdatesCollector &pendingUpdates)
{
    Q_D(KoCreatePathTool);

    if (d->mouseOverFirstPoint != value) {
        d->mouseOverFirstPoint = value;
        pendingUpdates.addUpdate(pathPointUpdateRects(d->firstPoint, KoPathPoint::Node));
    }
}

bool KoCreatePathTool::isOverFirstPoint(const QPointF &pos) const
{
    Q_D(const KoCreatePathTool);

    return d->firstPoint && viewDistanceFromDocPoints(pos, d->firstPoint->point()) < grabSensitivity();
}

QList<QPointer<QWidget> > KoCreatePathTool::createOptionWidgets()
{
    Q_D(KoCreatePathTool);

    QList<QPointer<QWidget> > list;

    QWidget *angleWidget = new QWidget();
    angleWidget->setObjectName("Angle Constraints");
    QGridLayout *layout = new QGridLayout(angleWidget);
    layout->addWidget(new QLabel(i18n("Angle snapping delta:"), angleWidget), 0, 0);
    QSpinBox *angleEdit = new KisIntParseSpinBox(angleWidget);
    angleEdit->setValue(d->angleSnappingDelta);
    angleEdit->setRange(1, 360);
    angleEdit->setSingleStep(1);
    angleEdit->setSuffix(QChar(Qt::Key_degree));
    layout->addWidget(angleEdit, 0, 1);
    layout->addWidget(new QLabel(i18n("Activate angle snap:"), angleWidget), 1, 0);
    QCheckBox *angleSnap = new QCheckBox(angleWidget);
    angleSnap->setChecked(false);
    angleSnap->setCheckable(true);
    layout->addWidget(angleSnap, 1, 1);
    QWidget *specialSpacer = new QWidget();
    specialSpacer->setObjectName("SpecialSpacer");
    layout->addWidget(specialSpacer, 2, 1);
    angleWidget->setWindowTitle(i18n("Angle Constraints"));
    list.append(angleWidget);

    connect(angleEdit, SIGNAL(valueChanged(int)), this, SLOT(angleDeltaChanged(int)));
    connect(angleSnap, SIGNAL(stateChanged(int)), this, SLOT(angleSnapChanged(int)));

    return list;
}

//have to include this because of Q_PRIVATE_SLOT
#include <moc_KoCreatePathTool.cpp>
