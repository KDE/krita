/* This file is part of the KDE project
 * Copyright (C) 2007,2009,2011 Jan Hambrecht <jaham@gmx.net>
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

#include "KoPencilTool.h"
#include "KoCurveFit.h"

#include <KoPathShape.h>
#include <KoParameterShape.h>
#include <KoShapeStroke.h>
#include <KoPointerEvent.h>
#include <KoCanvasBase.h>
#include <KoShapeController.h>
#include <KoShapeManager.h>
#include <KoSelection.h>
#include <KoCanvasResourceManager.h>
#include <KoColor.h>
#include <KoPathPoint.h>
#include <KoPathPointData.h>
#include <KoPathPointMergeCommand.h>
#include <KoShapePaintingContext.h>
#include <KoStrokeConfigWidget.h>

#include <knuminput.h>
#include <klocale.h>
#include <kcombobox.h>

#include <QStackedWidget>
#include <QGroupBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QPainter>
#include <QLabel>

#include <math.h>

#include "KoCreatePathTool_p.h"

KoPencilTool::KoPencilTool(KoCanvasBase *canvas)
        : KoToolBase(canvas),  m_mode(ModeCurve), m_optimizeRaw(false)
        , m_optimizeCurve(false), m_combineAngle(15.0), m_fittingError(5.0)
        , m_close(false), m_shape(0)
        , m_existingStartPoint(0), m_existingEndPoint(0), m_hoveredPoint(0)
{
}

KoPencilTool::~KoPencilTool()
{
}

void KoPencilTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (m_shape) {
        painter.save();

        painter.setTransform(m_shape->absoluteTransformation(&converter) * painter.transform());

        painter.save();
        KoShapePaintingContext paintContext; //FIXME
        m_shape->paint(painter, converter, paintContext);
        painter.restore();

        if (m_shape->stroke()) {
            painter.save();
            m_shape->stroke()->paint(m_shape, painter, converter);
            painter.restore();
        }

        painter.restore();
    }

    if (m_hoveredPoint) {
        painter.save();
        painter.setTransform(m_hoveredPoint->parent()->absoluteTransformation(&converter), true);
        KoShape::applyConversion(painter, converter);

        painter.setPen(Qt::blue);      //TODO make configurable
        painter.setBrush(Qt::white);   //TODO make configurable
        m_hoveredPoint->paint(painter, handleRadius(), KoPathPoint::Node);

        painter.restore();
    }
}

void KoPencilTool::repaintDecorations()
{
}

void KoPencilTool::mousePressEvent(KoPointerEvent *event)
{
    if (! m_shape) {
        m_shape = new KoPathShape();
        m_shape->setShapeId(KoPathShapeId);
        m_shape->setStroke(currentStroke());
        m_points.clear();

        QPointF point = event->point;
        m_existingStartPoint = endPointAtPosition(point);
        if (m_existingStartPoint)
            point = m_existingStartPoint->parent()->shapeToDocument(m_existingStartPoint->point());

        addPoint(point);
    }
}

void KoPencilTool::mouseMoveEvent(KoPointerEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
        addPoint(event->point);

    KoPathPoint * endPoint = endPointAtPosition(event->point);
    if (m_hoveredPoint != endPoint) {
        if (m_hoveredPoint) {
            QPointF nodePos = m_hoveredPoint->parent()->shapeToDocument(m_hoveredPoint->point());
            canvas()->updateCanvas(handlePaintRect(nodePos));
        }
        m_hoveredPoint = endPoint;
        if (m_hoveredPoint) {
            QPointF nodePos = m_hoveredPoint->parent()->shapeToDocument(m_hoveredPoint->point());
            canvas()->updateCanvas(handlePaintRect(nodePos));
        }
    }
}

void KoPencilTool::mouseReleaseEvent(KoPointerEvent *event)
{
    if (! m_shape)
        return;

    QPointF point = event->point;
    m_existingEndPoint = endPointAtPosition(point);
    if (m_existingEndPoint)
        point = m_existingEndPoint->parent()->shapeToDocument(m_existingEndPoint->point());

    addPoint(point);
    finish(event->modifiers() & Qt::ShiftModifier);

    m_existingStartPoint = 0;
    m_existingEndPoint = 0;
    m_hoveredPoint = 0;

    // the original path may be different from the one added
    canvas()->updateCanvas(m_shape->boundingRect());
    delete m_shape;
    m_shape = 0;
    m_points.clear();
}

void KoPencilTool::keyPressEvent(QKeyEvent *event)
{
    if (m_shape) {
        event->accept();
    } else {
        event->ignore();
    }
}

void KoPencilTool::activate(ToolActivation, const QSet<KoShape*> &)
{
    m_points.clear();
    m_close = false;
    useCursor(Qt::ArrowCursor);
}

void KoPencilTool::deactivate()
{
    m_points.clear();
    delete m_shape;
    m_shape = 0;
    m_existingStartPoint = 0;
    m_existingEndPoint = 0;
    m_hoveredPoint = 0;
}

void KoPencilTool::addPoint(const QPointF & point)
{
    if (! m_shape)
        return;

    // do a moveTo for the first point added
    if (m_points.empty())
        m_shape->moveTo(point);
    // do not allow coincident points
    else if (point != m_points.last())
        m_shape->lineTo(point);
    else
        return;

    m_points.append(point);
    canvas()->updateCanvas(m_shape->boundingRect());
}

qreal KoPencilTool::lineAngle(const QPointF &p1, const QPointF &p2)
{
    qreal angle = atan2(p2.y() - p1.y(), p2.x() - p1.x());
    if (angle < 0.0)
        angle += 2 * M_PI;

    return angle * 180.0 / M_PI;
}

void KoPencilTool::finish(bool closePath)
{
    if (m_points.count() < 2)
        return;

    KoPathShape * path = 0;
    QList<QPointF> complete;
    QList<QPointF> *points = &m_points;

    if (m_mode == ModeStraight || m_optimizeRaw || m_optimizeCurve) {
        float combineAngle;

        if (m_mode == ModeStraight)
            combineAngle = m_combineAngle;
        else
            combineAngle = 0.50f;

        //Add the first two points
        complete.append(m_points[0]);
        complete.append(m_points[1]);

        //Now we need to get the angle of the first line
        float lastAngle = lineAngle(complete[0], complete[1]);

        uint pointCount = m_points.count();
        for (uint i = 2; i < pointCount; ++i) {
            float angle = lineAngle(complete.last(), m_points[i]);
            if (qAbs(angle - lastAngle) < combineAngle)
                complete.removeLast();
            complete.append(m_points[i]);
            lastAngle = angle;
        }

        m_points.clear();
        points = &complete;
    }

    switch (m_mode) {
    case ModeCurve: {
        path = bezierFit(*points, m_fittingError);
    }
    break;
    case ModeStraight:
    case ModeRaw: {
        path = new KoPathShape();
        uint pointCount = points->count();
        path->moveTo(points->at(0));
        for (uint i = 1; i < pointCount; ++i)
            path->lineTo(points->at(i));
    }
    break;
    }

    if (! path)
        return;

    addPathShape(path, closePath);
}

QList<QWidget *> KoPencilTool::createOptionWidgets()
{
    QList<QWidget *> widgets;
    QWidget *optionWidget = new QWidget();
    QVBoxLayout * layout = new QVBoxLayout(optionWidget);

    QHBoxLayout *modeLayout = new QHBoxLayout;
    modeLayout->setSpacing(3);
    QLabel *modeLabel = new QLabel(i18n("Precision:"), optionWidget);
    KComboBox * modeBox = new KComboBox(optionWidget);
    modeBox->addItem(i18nc("The raw line data", "Raw"));
    modeBox->addItem(i18n("Curve"));
    modeBox->addItem(i18n("Straight"));
    modeLayout->addWidget(modeLabel);
    modeLayout->addWidget(modeBox, 1);
    layout->addLayout(modeLayout);

    QStackedWidget * stackedWidget = new QStackedWidget(optionWidget);

    QWidget * rawBox = new QWidget(stackedWidget);
    QVBoxLayout * rawLayout = new QVBoxLayout(rawBox);
    QCheckBox * optimizeRaw = new QCheckBox(i18n("Optimize"), rawBox);
    rawLayout->addWidget(optimizeRaw);
    rawLayout->setContentsMargins(0, 0, 0, 0);

    QWidget * curveBox = new QWidget(stackedWidget);
    QHBoxLayout * curveLayout = new QHBoxLayout(curveBox);
    QCheckBox * optimizeCurve = new QCheckBox(i18n("Optimize"), curveBox);
    KDoubleNumInput * fittingError = new KDoubleNumInput(0.0, 400.0, m_fittingError, curveBox, 0.50, 3);
    fittingError->setToolTip(i18n("Exactness:"));
    curveLayout->addWidget(optimizeCurve);
    curveLayout->addWidget(fittingError);
    curveLayout->setContentsMargins(0, 0, 0, 0);

    QWidget * straightBox = new QWidget(stackedWidget);
    QVBoxLayout * straightLayout = new QVBoxLayout(straightBox);
    KDoubleNumInput * combineAngle = new KDoubleNumInput(0.0, 360.0, m_combineAngle, straightBox, 0.50, 3);
    combineAngle->setSuffix(" deg");
    combineAngle->setLabel(i18n("Combine angle:"), Qt::AlignLeft | Qt::AlignVCenter);
    straightLayout->addWidget(combineAngle);
    straightLayout->setContentsMargins(0, 0, 0, 0);

    stackedWidget->addWidget(rawBox);
    stackedWidget->addWidget(curveBox);
    stackedWidget->addWidget(straightBox);
    layout->addWidget(stackedWidget);
    layout->addStretch(1);

    connect(modeBox, SIGNAL(activated(int)), stackedWidget, SLOT(setCurrentIndex(int)));
    connect(modeBox, SIGNAL(activated(int)), this, SLOT(selectMode(int)));
    connect(optimizeRaw, SIGNAL(stateChanged(int)), this, SLOT(setOptimize(int)));
    connect(optimizeCurve, SIGNAL(stateChanged(int)), this, SLOT(setOptimize(int)));
    connect(fittingError, SIGNAL(valueChanged(double)), this, SLOT(setDelta(double)));
    connect(combineAngle, SIGNAL(valueChanged(double)), this, SLOT(setDelta(double)));

    modeBox->setCurrentIndex(m_mode);
    stackedWidget->setCurrentIndex(m_mode);
    optionWidget->setObjectName(i18n("Pencil"));
    optionWidget->setWindowTitle(i18n("Pencil"));
    widgets.append(optionWidget);
    
    KoStrokeConfigWidget *strokeWidget = new KoStrokeConfigWidget(0);
    strokeWidget->setWindowTitle(i18n("Line"));
    strokeWidget->setCanvas(canvas());
    widgets.append(strokeWidget);
    return widgets;
}

void KoPencilTool::addPathShape(KoPathShape* path, bool closePath)
{
    KoShape * startShape = 0;
    KoShape * endShape = 0;

    if (closePath) {
        path->close();
        path->normalize();
    } else {
        path->normalize();
        if (connectPaths(path, m_existingStartPoint, m_existingEndPoint)) {
            if (m_existingStartPoint)
                startShape = m_existingStartPoint->parent();
            if (m_existingEndPoint && m_existingEndPoint != m_existingStartPoint)
                endShape = m_existingEndPoint->parent();
        }
    }

    // set the proper shape id
    path->setShapeId(KoPathShapeId);
    path->setStroke(currentStroke());
    KUndo2Command * cmd = canvas()->shapeController()->addShape(path);
    if (cmd) {
        KoSelection *selection = canvas()->shapeManager()->selection();
        selection->deselectAll();
        selection->select(path);

        if (startShape)
            canvas()->shapeController()->removeShape(startShape, cmd);
        if (endShape && startShape != endShape)
            canvas()->shapeController()->removeShape(endShape, cmd);

        canvas()->addCommand(cmd);
    } else {
        canvas()->updateCanvas(path->boundingRect());
        delete path;
    }
}

void KoPencilTool::selectMode(int mode)
{
    m_mode = static_cast<PencilMode>(mode);
}

void KoPencilTool::setOptimize(int state)
{
    if (m_mode == ModeRaw)
        m_optimizeRaw = state == Qt::Checked ? true : false;
    else
        m_optimizeCurve = state == Qt::Checked ? true : false;
}

void KoPencilTool::setDelta(double delta)
{
    if (m_mode == ModeCurve)
        m_fittingError = delta;
    else if (m_mode == ModeStraight)
        m_combineAngle = delta;
}

KoShapeStroke * KoPencilTool::currentStroke()
{
    KoShapeStroke * stroke = new KoShapeStroke(canvas()->resourceManager()->activeStroke());
    stroke->setColor(canvas()->resourceManager()->foregroundColor().toQColor());
    return stroke;
}

KoPathPoint* KoPencilTool::endPointAtPosition(const QPointF &position)
{
    QRectF roi = handleGrabRect(position);
    QList<KoShape *> shapes = canvas()->shapeManager()->shapesAt(roi);

    KoPathPoint * nearestPoint = 0;
    qreal minDistance = HUGE_VAL;
    qreal maxDistance = canvas()->viewConverter()->viewToDocumentX(grabSensitivity());

    foreach(KoShape *shape, shapes) {
        KoPathShape * path = dynamic_cast<KoPathShape*>(shape);
        if (!path)
            continue;
        KoParameterShape *paramShape = dynamic_cast<KoParameterShape*>(shape);
        if (paramShape && paramShape->isParametricShape())
            continue;

        KoPathPoint * p = 0;
        uint subpathCount = path->subpathCount();
        for (uint i = 0; i < subpathCount; ++i) {
            if (path->isClosedSubpath(i))
                continue;
            p = path->pointByIndex(KoPathPointIndex(i, 0));
            // check start of subpath
            qreal d = squareDistance(position, path->shapeToDocument(p->point()));
            if (d < minDistance && d < maxDistance) {
                nearestPoint = p;
                minDistance = d;
            }
            // check end of subpath
            p = path->pointByIndex(KoPathPointIndex(i, path->subpathPointCount(i) - 1));
            d = squareDistance(position, path->shapeToDocument(p->point()));
            if (d < minDistance && d < maxDistance) {
                nearestPoint = p;
                minDistance = d;
            }
        }
    }

    return nearestPoint;
}

bool KoPencilTool::connectPaths(KoPathShape *pathShape, KoPathPoint *pointAtStart, KoPathPoint *pointAtEnd)
{
    // at least one point must be valid
    if (!pointAtStart && !pointAtEnd)
        return false;
    // do not allow connecting to the same point twice
    if (pointAtStart == pointAtEnd)
        pointAtEnd = 0;

    // we have hit an existing path point on start/finish
    // what we now do is:
    // 1. combine the new created path with the ones we hit on start/finish
    // 2. merge the endpoints of the corresponding subpaths

    uint newPointCount = pathShape->subpathPointCount(0);
    KoPathPointIndex newStartPointIndex(0, 0);
    KoPathPointIndex newEndPointIndex(0, newPointCount - 1);
    KoPathPoint * newStartPoint = pathShape->pointByIndex(newStartPointIndex);
    KoPathPoint * newEndPoint = pathShape->pointByIndex(newEndPointIndex);

    KoPathShape * startShape = pointAtStart ? pointAtStart->parent() : 0;
    KoPathShape * endShape = pointAtEnd ? pointAtEnd->parent() : 0;

    // combine with the path we hit on start
    KoPathPointIndex startIndex(-1, -1);
    if (pointAtStart) {
        startIndex = startShape->pathPointIndex(pointAtStart);
        pathShape->combine(startShape);
        pathShape->moveSubpath(0, pathShape->subpathCount() - 1);
    }
    // combine with the path we hit on finish
    KoPathPointIndex endIndex(-1, -1);
    if (pointAtEnd) {
        endIndex = endShape->pathPointIndex(pointAtEnd);
        if (endShape != startShape) {
            endIndex.first += pathShape->subpathCount();
            pathShape->combine(endShape);
        }
    }
    // do we connect twice to a single subpath ?
    bool connectToSingleSubpath = (startShape == endShape && startIndex.first == endIndex.first);

    if (startIndex.second == 0 && !connectToSingleSubpath) {
        pathShape->reverseSubpath(startIndex.first);
        startIndex.second = pathShape->subpathPointCount(startIndex.first) - 1;
    }
    if (endIndex.second > 0 && !connectToSingleSubpath) {
        pathShape->reverseSubpath(endIndex.first);
        endIndex.second = 0;
    }

    // after combining we have a path where with the subpaths in the following
    // order:
    // 1. the subpaths of the pathshape we started the new path at
    // 2. the subpath we just created
    // 3. the subpaths of the pathshape we finished the new path at

    // get the path points we want to merge, as these are not going to
    // change while merging
    KoPathPoint * existingStartPoint = pathShape->pointByIndex(startIndex);
    KoPathPoint * existingEndPoint = pathShape->pointByIndex(endIndex);

    // merge first two points
    if (existingStartPoint) {
        KoPathPointData pd1(pathShape, pathShape->pathPointIndex(existingStartPoint));
        KoPathPointData pd2(pathShape, pathShape->pathPointIndex(newStartPoint));
        KoPathPointMergeCommand cmd1(pd1, pd2);
        cmd1.redo();
    }
    // merge last two points
    if (existingEndPoint) {
        KoPathPointData pd3(pathShape, pathShape->pathPointIndex(newEndPoint));
        KoPathPointData pd4(pathShape, pathShape->pathPointIndex(existingEndPoint));
        KoPathPointMergeCommand cmd2(pd3, pd4);
        cmd2.redo();
    }

    return true;
}

#include "KoPencilTool.moc"
