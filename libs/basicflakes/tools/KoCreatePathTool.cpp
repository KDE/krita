/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2006 Thorsten Zachmann <zachmann@kde.org>
 * SPDX-FileCopyrightText: 2008-2010 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
#include <KisAngleSelector.h>

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

QRectF KoCreatePathTool::decorationsRect() const
{
    Q_D(const KoCreatePathTool);

    QRectF dirtyRect;

    if (pathStarted()) {
        dirtyRect |= kisGrowRect(d->shape->boundingRect(), handleDocRadius());
    }

    if (d->hoveredPoint) {
        dirtyRect |= kisGrowRect(d->hoveredPoint->boundingRect(false), handleDocRadius());
    }

    if (d->activePoint) {
        dirtyRect |= kisGrowRect(d->activePoint->boundingRect(false), handleDocRadius());

        if (d->pointIsDragged) {
            // the path is not closed, therefore the point is not marked as
            // active inside the path itself
            dirtyRect |= handlePaintRect(
                        d->activePoint->parent()->shapeToDocument(
                            d->activePoint->controlPoint2()));
        }

    }

    if (canvas()->snapGuide()->isSnapping()) {
        dirtyRect |= canvas()->snapGuide()->boundingRect();
    }

    return dirtyRect;
}

void KoCreatePathTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_D(KoCreatePathTool);

    if (pathStarted()) {

        painter.save();
        paintPath(*(d->shape), painter, converter);
        painter.restore();

        KisHandlePainterHelper helper =
            KoShape::createHandlePainterHelperView(&painter, d->shape, converter, d->handleRadius);

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
        KisHandlePainterHelper helper = KoShape::createHandlePainterHelperView(&painter, d->hoveredPoint->parent(), converter, d->handleRadius);
        helper.setHandleStyle(KisHandleStyle::highlightedPrimaryHandles());
        d->hoveredPoint->paint(helper, KoPathPoint::Node);
    }

    painter.save();
    painter.setTransform(converter.documentToView(), true);
    canvas()->snapGuide()->paint(painter, converter);
    painter.restore();
}

void KoCreatePathTool::paintPath(KoPathShape& pathShape, QPainter &painter, const KoViewConverter &converter)
{
    Q_D(KoCreatePathTool);
    painter.setTransform(pathShape.absoluteTransformation() *
                         converter.documentToView() *
                         painter.transform());
    painter.save();

    KoShapePaintingContext paintContext; //FIXME
    pathShape.paint(painter, paintContext);
    painter.restore();

    if (pathShape.stroke()) {
        painter.save();
        pathShape.stroke()->paint(d->shape, painter);
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
                repaintDecorations();
            }
        } else {
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
                repaintDecorations();
            }
        }
    } else {
        KoPathShape *pathShape = new KoPathShape();
        d->shape = pathShape;
        pathShape->setShapeId(KoPathShapeId);

        KoShapeStrokeSP stroke(new KoShapeStroke());
        const qreal size = canvas()->resourceManager()->resource(KoCanvasResource::Size).toReal();

        stroke->setLineWidth(canvas()->unit().fromUserValue(size));
        stroke->setColor(canvas()->resourceManager()->foregroundColor().toQColor());

        pathShape->setStroke(stroke);
        QPointF point = canvas()->snapGuide()->snap(event->point, event->modifiers());

        // check whether we hit an start/end node of an existing path
        d->existingStartPoint = d->endPointAtPosition(point);

        if (d->existingStartPoint.isValid()) {
            point = d->existingStartPoint.path->shapeToDocument(d->existingStartPoint.point->point());
        }

        d->activePoint = pathShape->moveTo(point);
        d->firstPoint = d->activePoint;

        canvas()->snapGuide()->setAdditionalEditedShape(pathShape);

        d->angleSnapStrategy = new AngleSnapStrategy(d->angleSnappingDelta, d->angleSnapStatus);
        canvas()->snapGuide()->addCustomSnapStrategy(d->angleSnapStrategy);

        repaintDecorations();
    }

    d->dragStartPoint = event->point;

    if (d->angleSnapStrategy)
        d->angleSnapStrategy->setStartPoint(d->activePoint->point());
}

bool KoCreatePathTool::pathStarted() const
{
    Q_D(const KoCreatePathTool);
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

    d->hoveredPoint = d->endPointAtPosition(event->point);

    if (!pathStarted()) {
        canvas()->snapGuide()->snap(event->point, event->modifiers());
        repaintDecorations();

        d->mouseOverFirstPoint = false;
        return;
    }

    d->mouseOverFirstPoint = handleGrabRect(d->firstPoint->point()).contains(event->point);

    QPointF snappedPosition = canvas()->snapGuide()->snap(event->point, event->modifiers());

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

    repaintDecorations();
}

void KoCreatePathTool::mouseReleaseEvent(KoPointerEvent *event)
{
    Q_D(KoCreatePathTool);

    if (! d->shape || (event->buttons() & Qt::RightButton)) return;

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

    repaintDecorations();
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
    repaintDecorations();
}

void KoCreatePathTool::endPathWithoutLastPoint()
{
    Q_D(KoCreatePathTool);

    if (d->shape) {
        delete d->shape->removePoint(d->shape->pathPointIndex(d->activePoint));
        d->addPathShape();

        repaintDecorations();
    }
}

void KoCreatePathTool::cancelPath()
{
    Q_D(KoCreatePathTool);

    if (d->shape) {
        d->firstPoint = 0;
        d->activePoint = 0;
    }
    d->cleanUp();
    repaintDecorations();
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

            repaintDecorations();
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
    canvas()->snapGuide()->reset();
    repaintDecorations();
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
    KisAngleSelector *angleEdit = new KisAngleSelector(angleWidget);
    angleEdit->setAngle(d->angleSnappingDelta);
    angleEdit->setRange(1, 360);
    angleEdit->setDecimals(0);
    angleEdit->setFlipOptionsMode(KisAngleSelector::FlipOptionsMode_MenuButton);
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

    connect(angleEdit, SIGNAL(angleChanged(qreal)), this, SLOT(angleDeltaChanged(qreal)));
    connect(angleSnap, SIGNAL(stateChanged(int)), this, SLOT(angleSnapChanged(int)));

    return list;
}

//have to include this because of Q_PRIVATE_SLOT
#include <moc_KoCreatePathTool.cpp>
