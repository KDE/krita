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
#include <KisHandlePainterHelper.h>
#include "KoPathPointTypeCommand.h"

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

void KoCreatePathTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_D(KoCreatePathTool);

    if (pathStarted()) {

        painter.save();
        paintPath(*(d->shape), painter, converter);
        painter.restore();

        KisHandlePainterHelper helper =
            KoShape::createHandlePainterHelper(&painter, d->shape, converter, d->handleRadius);

        const bool firstPointActive = d->firstPoint == d->activePoint;

        if (d->pointIsDragged || firstPointActive) {
            const bool onlyPaintActivePoints = false;
            KoPathPoint::PointTypes paintFlags = KoPathPoint::ControlPoint2;

            if (d->activePoint->activeControlPoint1()) {
                paintFlags |= KoPathPoint::ControlPoint1;
            }

            helper.setHandleStyle(KisHandleStyle::highlightedPrimaryHandles());
            d->activePoint->paint(helper, paintFlags, onlyPaintActivePoints);
        }

        if (!firstPointActive) {
            helper.setHandleStyle(d->mouseOverFirstPoint ?
                                      KisHandleStyle::highlightedPrimaryHandles() :
                                      KisHandleStyle::primarySelection());
            d->firstPoint->paint(helper, KoPathPoint::Node);
        }
    }

    if (d->hoveredPoint) {
        KisHandlePainterHelper helper = KoShape::createHandlePainterHelper(&painter, d->hoveredPoint->parent(), converter, d->handleRadius);
        helper.setHandleStyle(KisHandleStyle::highlightedPrimaryHandles());
        d->hoveredPoint->paint(helper, KoPathPoint::Node);
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

    KoShapePaintingContext paintContext; //FIXME
    pathShape.paint(painter, converter, paintContext);
    painter.restore();

    if (pathShape.stroke()) {
        painter.save();
        pathShape.stroke()->paint(d->shape, painter, converter);
        painter.restore();
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

    const bool isOverFirstPoint = d->shape &&
            handleGrabRect(d->firstPoint->point()).contains(event->point);

    const bool haveCloseModifier = d->enableClosePathShortcut
            && d->shape
            && d->shape->pointCount() > 2
            && (event->modifiers() & Qt::ShiftModifier);

    if ((event->button() == Qt::LeftButton) && haveCloseModifier && !isOverFirstPoint) {
        endPathWithoutLastPoint();
        return;
    }

    d->finishAfterThisPoint = false;

    if (d->shape && pathStarted()) {
        if (isOverFirstPoint) {
            d->activePoint->setPoint(d->firstPoint->point());
            canvas()->updateCanvas(d->shape->boundingRect());
            canvas()->updateCanvas(canvas()->snapGuide()->boundingRect());

            if (haveCloseModifier) {
                d->shape->closeMerge();
                // we are closing the path, so reset the existing start path point
                d->existingStartPoint = 0;
                // finish path
                endPath();
            } else {
                // the path shape will get closed when the user releases
                // the mouse button
                d->finishAfterThisPoint = true;
            }
        } else {
            canvas()->updateCanvas(canvas()->snapGuide()->boundingRect());

            QPointF point = canvas()->snapGuide()->snap(event->point, event->modifiers());

            // check whether we hit an start/end node of an existing path
            d->existingEndPoint = d->endPointAtPosition(point);
            if (d->existingEndPoint.isValid() && d->existingEndPoint != d->existingStartPoint) {
                point = d->existingEndPoint.path->shapeToDocument(d->existingEndPoint.point->point());
                d->activePoint->setPoint(point);
                // finish path
                endPath();
            } else {
                d->activePoint->setPoint(point);
                canvas()->updateCanvas(d->shape->boundingRect());
                canvas()->updateCanvas(canvas()->snapGuide()->boundingRect());
            }
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

        canvas()->snapGuide()->setAdditionalEditedShape(pathShape);

        d->angleSnapStrategy = new AngleSnapStrategy(d->angleSnappingDelta, d->angleSnapStatus);
        canvas()->snapGuide()->addCustomSnapStrategy(d->angleSnapStrategy);
    }

    d->dragStartPoint = event->point;

    if (d->angleSnapStrategy)
        d->angleSnapStrategy->setStartPoint(d->activePoint->point());
}

bool KoCreatePathTool::pathStarted()
{
    Q_D(KoCreatePathTool);
    return ((bool) d->shape);
}

bool KoCreatePathTool::tryMergeInPathShape(KoPathShape *pathShape)
{
    return addPathShapeImpl(pathShape, true);
}

void KoCreatePathTool::setEnableClosePathShortcut(bool value)
{
    Q_D(KoCreatePathTool);
    d->enableClosePathShortcut = value;
}

void KoCreatePathTool::mouseDoubleClickEvent(KoPointerEvent *event)
{
    //remove handle
    canvas()->updateCanvas(handlePaintRect(event->point));

    endPathWithoutLastPoint();
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

    if (!pathStarted()) {

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
        if (d->pointIsDragged ||
            !handleGrabRect(d->dragStartPoint).contains(event->point)) {

            d->pointIsDragged = true;
            QPointF offset = snappedPosition - d->activePoint->point();
            d->activePoint->setControlPoint2(d->activePoint->point() + offset);
            // pressing <alt> stops controls points moving symmetrically
            if ((event->modifiers() & Qt::AltModifier) == 0) {
                d->activePoint->setControlPoint1(d->activePoint->point() - offset);
            }
            d->repaintActivePoint();
        }
    } else {
        d->activePoint->setPoint(snappedPosition);

        if (!d->prevPointWasDragged && d->autoSmoothCurves) {
            KoPathPointIndex index = d->shape->pathPointIndex(d->activePoint);
            if (index.second > 0) {

                KoPathPointIndex prevIndex(index.first, index.second - 1);
                KoPathPoint *prevPoint = d->shape->pointByIndex(prevIndex);

                if (prevPoint) {
                    KoPathPoint *prevPrevPoint = 0;

                    if (index.second > 1) {
                        KoPathPointIndex prevPrevIndex(index.first, index.second - 2);
                        prevPrevPoint = d->shape->pointByIndex(prevPrevIndex);
                    }

                    if (prevPrevPoint) {
                        const QPointF control1 = prevPoint->point() + 0.3 * (prevPrevPoint->point() - prevPoint->point());
                        prevPoint->setControlPoint1(control1);
                    }

                    const QPointF control2 = prevPoint->point() + 0.3 * (d->activePoint->point() - prevPoint->point());
                    prevPoint->setControlPoint2(control2);

                    const QPointF activeControl = d->activePoint->point() + 0.3 * (prevPoint->point() - d->activePoint->point());
                    d->activePoint->setControlPoint1(activeControl);

                    KoPathPointTypeCommand::makeCubicPointSmooth(prevPoint);
                }
            }
        }

    }

    canvas()->updateCanvas(d->shape->boundingRect());
    canvas()->updateCanvas(canvas()->snapGuide()->boundingRect());
}

void KoCreatePathTool::mouseReleaseEvent(KoPointerEvent *event)
{
    Q_D(KoCreatePathTool);

    if (! d->shape || (event->buttons() & Qt::RightButton)) return;

    d->repaintActivePoint();
    d->prevPointWasDragged  = d->pointIsDragged;
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
        if (qFuzzyCompare(diff1.x(), diff2.x()) && qFuzzyCompare(diff1.y(), diff2.y()))
            lastActivePoint->setProperty(KoPathPoint::IsSymmetric);
    }

    if (d->finishAfterThisPoint) {

        d->firstPoint->setControlPoint1(d->activePoint->controlPoint1());
        delete d->shape->removePoint(d->shape->pathPointIndex(d->activePoint));
        d->activePoint = d->firstPoint;

        if (!d->prevPointWasDragged && d->autoSmoothCurves) {
            KoPathPointTypeCommand::makeCubicPointSmooth(d->activePoint);
        }

        d->shape->closeMerge();

        // we are closing the path, so reset the existing start path point
        d->existingStartPoint = 0;
        // finish path
        endPath();
    }

    if (d->angleSnapStrategy && lastActivePoint->activeControlPoint2()) {
        d->angleSnapStrategy->deactivate();
    }
}

void KoCreatePathTool::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        emit done();
    } else {
        event->ignore();
    }
}

void KoCreatePathTool::endPath()
{
    Q_D(KoCreatePathTool);

    d->addPathShape();
}

void KoCreatePathTool::endPathWithoutLastPoint()
{
    Q_D(KoCreatePathTool);

    if (d->shape) {
        QRectF dirtyRect = d->shape->boundingRect();
        delete d->shape->removePoint(d->shape->pathPointIndex(d->activePoint));
        canvas()->updateCanvas(dirtyRect);

        d->addPathShape();
    }
}

void KoCreatePathTool::cancelPath()
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

void KoCreatePathTool::removeLastPoint()
{
    Q_D(KoCreatePathTool);

    if ((d->shape)) {
        KoPathPointIndex lastPointIndex = d->shape->pathPointIndex(d->activePoint);

        if (lastPointIndex.second > 1) {
            lastPointIndex.second--;
            delete d->shape->removePoint(lastPointIndex);

            d->hoveredPoint = 0;

            d->repaintActivePoint();
            canvas()->updateCanvas(d->shape->boundingRect());
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
    d->loadAutoSmoothValueFromConfig();

    // reset snap guide
    canvas()->updateCanvas(canvas()->snapGuide()->boundingRect());
    canvas()->snapGuide()->reset();
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

bool KoCreatePathTool::addPathShapeImpl(KoPathShape *pathShape, bool tryMergeOnly)
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

    if (tryMergeOnly && !startShape && !endShape) {
        return false;
    }

    KUndo2Command *cmd = canvas()->shapeController()->addShape(pathShape, 0);
    KIS_SAFE_ASSERT_RECOVER(cmd) {
        canvas()->updateCanvas(pathShape->boundingRect());
        delete pathShape;
        return true;
    }

    KoSelection *selection = canvas()->shapeManager()->selection();
    selection->deselectAll();
    selection->select(pathShape);

    if (startShape) {
        pathShape->setBackground(startShape->background());
        pathShape->setStroke(startShape->stroke());
    } else if (endShape) {
        pathShape->setBackground(endShape->background());
        pathShape->setStroke(endShape->stroke());
    }


    if (startShape) {
        canvas()->shapeController()->removeShape(startShape, cmd);
    }
    if (endShape && startShape != endShape) {
        canvas()->shapeController()->removeShape(endShape, cmd);
    }
    canvas()->addCommand(cmd);

    return true;
}


void KoCreatePathTool::addPathShape(KoPathShape *pathShape)
{
    addPathShapeImpl(pathShape, false);
}

QList<QPointer<QWidget> > KoCreatePathTool::createOptionWidgets()
{
    Q_D(KoCreatePathTool);

    QList<QPointer<QWidget> > list;

    QCheckBox *smoothCurves = new QCheckBox(i18n("Autosmooth curve"));
    smoothCurves->setObjectName("smooth-curves-widget");
    smoothCurves->setChecked(d->autoSmoothCurves);
    connect(smoothCurves, SIGNAL(toggled(bool)), this, SLOT(autoSmoothCurvesChanged(bool)));
    connect(this, SIGNAL(sigUpdateAutoSmoothCurvesGUI(bool)), smoothCurves, SLOT(setChecked(bool)));

    list.append(smoothCurves);

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
