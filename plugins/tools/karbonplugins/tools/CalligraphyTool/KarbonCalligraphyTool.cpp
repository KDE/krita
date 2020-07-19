/* This file is part of the KDE project
 * Copyright (C) 2008 Fela Winkelmolen <fela.kde@gmail.com>
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

#include "KarbonCalligraphyTool.h"
#include "KarbonCalligraphicShape.h"
#include "KarbonCalligraphyOptionWidget.h"

#include <KoPathShape.h>
#include <KoShapeGroup.h>
#include <KoPointerEvent.h>
#include <KoPathPoint.h>
#include <KoCanvasBase.h>
#include <KoShapeController.h>
#include <KoShapeManager.h>
#include <KoSelectedShapesProxy.h>
#include <KoSelection.h>
#include <KoCurveFit.h>
#include <KoColorBackground.h>
#include <KoCanvasResourceProvider.h>
#include <KoColor.h>
#include <KoShapePaintingContext.h>
#include <KoViewConverter.h>

#include <QAction>
#include <QDebug>
#include <klocalizedstring.h>
#include <QPainter>

#include <cmath>

#undef M_PI
const qreal M_PI = 3.1415927;
using std::pow;
using std::sqrt;

KarbonCalligraphyTool::KarbonCalligraphyTool(KoCanvasBase *canvas)
    : KoToolBase(canvas)
    , m_shape(0)
    , m_angle(0)
    , m_selectedPath(0)
    , m_isDrawing(false)
    , m_speed(0, 0)
    , m_lastShape(0)
{
    connect(canvas->selectedShapesProxy(), SIGNAL(selectionChanged()), SLOT(updateSelectedPath()));

    updateSelectedPath();
}

KarbonCalligraphyTool::~KarbonCalligraphyTool()
{
}

void KarbonCalligraphyTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (m_selectedPath) {
        painter.save();
        painter.setRenderHints(QPainter::Antialiasing, false);
        painter.setPen(Qt::red);   // TODO make configurable
        QRectF rect = m_selectedPath->boundingRect();
        QPointF p1 = converter.documentToView(rect.topLeft());
        QPointF p2 = converter.documentToView(rect.bottomRight());
        painter.drawRect(QRectF(p1, p2));
        painter.restore();
    }

    if (!m_shape) {
        return;
    }

    painter.save();

    painter.setTransform(m_shape->absoluteTransformation() *
                         converter.documentToView() *
                         painter.transform());
    KoShapePaintingContext paintContext; //FIXME
    m_shape->paint(painter, paintContext);

    painter.restore();
}

void KarbonCalligraphyTool::mousePressEvent(KoPointerEvent *event)
{
    if (m_isDrawing) {
        return;
    }

    m_lastPoint = event->point;
    m_speed = QPointF(0, 0);

    m_isDrawing = true;
    m_pointCount = 0;
    m_shape = new KarbonCalligraphicShape(m_caps);
    m_shape->setBackground(QSharedPointer<KoShapeBackground>(new KoColorBackground(canvas()->resourceManager()->foregroundColor().toQColor())));
    //addPoint( event );
}

void KarbonCalligraphyTool::mouseMoveEvent(KoPointerEvent *event)
{
    if (!m_isDrawing) {
        return;
    }

    addPoint(event);
}

void KarbonCalligraphyTool::mouseReleaseEvent(KoPointerEvent *event)
{
    if (!m_isDrawing) {
        return;
    }

    if (m_pointCount == 0) {
        // handle click: select shape (if any)
        if (event->point == m_lastPoint) {
            KoShapeManager *shapeManager = canvas()->shapeManager();
            KoShape *selectedShape = shapeManager->shapeAt(event->point);
            if (selectedShape != 0) {
                shapeManager->selection()->deselectAll();
                shapeManager->selection()->select(selectedShape);
            }
        }

        delete m_shape;
        m_shape = 0;
        m_isDrawing = false;
        return;
    } else {
        m_endOfPath = false;    // allow last point being added
        addPoint(event);        // add last point
        m_isDrawing = false;
    }

    m_shape->simplifyGuidePath();

    KUndo2Command *cmd = canvas()->shapeController()->addShape(m_shape, 0);
    if (cmd) {
        m_lastShape = m_shape;
        canvas()->addCommand(cmd);
        canvas()->updateCanvas(m_shape->boundingRect());
    } else {
        // don't leak shape when command could not be created
        delete m_shape;
    }

    m_shape = 0;
}

void KarbonCalligraphyTool::addPoint(KoPointerEvent *event)
{
    if (m_pointCount == 0) {
        if (m_usePath && m_selectedPath) {
            m_selectedPathOutline = m_selectedPath->outline();
        }
        m_pointCount = 1;
        m_endOfPath = false;
        m_followPathPosition = 0;
        m_lastMousePos = event->point;
        m_lastPoint = calculateNewPoint(event->point, &m_speed);
        m_deviceSupportsTilt = (event->xTilt() != 0 || event->yTilt() != 0);
        return;
    }

    if (m_endOfPath) {
        return;
    }

    ++m_pointCount;

    setAngle(event);

    QPointF newSpeed;
    QPointF newPoint = calculateNewPoint(event->point, &newSpeed);
    qreal width = calculateWidth(event->pressure());
    qreal angle = calculateAngle(m_speed, newSpeed);

    // add the previous point
    m_shape->appendPoint(m_lastPoint, angle, width);

    m_speed = newSpeed;
    m_lastPoint = newPoint;
    canvas()->updateCanvas(m_shape->lastPieceBoundingRect());

    if (m_usePath && m_selectedPath) {
        m_speed = QPointF(0, 0);    // following path
    }
}

void KarbonCalligraphyTool::setAngle(KoPointerEvent *event)
{
    if (!m_useAngle) {
        m_angle = (360.0 - m_customAngle + 90.0) / 180.0 * M_PI;
        return;
    }

    // setting m_angle to the angle of the device
    if (event->xTilt() != 0 || event->yTilt() != 0) {
        m_deviceSupportsTilt = false;
    }

    if (m_deviceSupportsTilt) {
        if (event->xTilt() == 0 && event->yTilt() == 0) {
            return;    // leave as is
        }
        if (event->x() == 0) {
            m_angle = M_PI / 2.0;
            return;
        }

        // y is inverted in qt painting
        m_angle = std::atan(static_cast<double>(-event->yTilt()) / static_cast<double>(event->xTilt())) + M_PI / 2.0;
    } else {
        m_angle = event->rotation() + M_PI / 2.0;
    }
}

QPointF KarbonCalligraphyTool::calculateNewPoint(const QPointF &mousePos, QPointF *speed)
{
    if (!m_usePath || !m_selectedPath) { // don't follow path
        QPointF force = mousePos - m_lastPoint;
        QPointF dSpeed = force / m_mass;
        *speed = m_speed * (1.0 - m_drag) + dSpeed;
        return m_lastPoint + *speed;
    }

    QPointF sp = mousePos - m_lastMousePos;
    m_lastMousePos = mousePos;

    // follow selected path
    qreal step = QLineF(QPointF(0, 0), sp).length();
    m_followPathPosition += step;

    qreal t;
    if (m_followPathPosition >= m_selectedPathOutline.length()) {
        t = 1.0;
        m_endOfPath = true;
    } else {
        t = m_selectedPathOutline.percentAtLength(m_followPathPosition);
    }

    QPointF res = m_selectedPathOutline.pointAtPercent(t)
            + m_selectedPath->position();
    *speed = res - m_lastPoint;
    return res;
}

qreal KarbonCalligraphyTool::calculateWidth(qreal pressure)
{
    // calculate the modulo of the speed
    qreal speed = std::sqrt(pow(m_speed.x(), 2) + pow(m_speed.y(), 2));
    qreal thinning =  m_thinning * (speed + 1) / 10.0; // can be negative

    if (thinning > 1) {
        thinning = 1;
    }

    if (!m_usePressure) {
        pressure = 1.0;
    }

    qreal strokeWidth = m_strokeWidth * pressure * (1 - thinning);

    const qreal MINIMUM_STROKE_WIDTH = 1.0;
    if (strokeWidth < MINIMUM_STROKE_WIDTH) {
        strokeWidth = MINIMUM_STROKE_WIDTH;
    }

    return strokeWidth;
}

qreal KarbonCalligraphyTool::calculateAngle(const QPointF &oldSpeed, const QPointF &newSpeed)
{
    // calculate the average of the speed (sum of the normalized values)
    qreal oldLength = QLineF(QPointF(0, 0), oldSpeed).length();
    qreal newLength = QLineF(QPointF(0, 0), newSpeed).length();
    QPointF oldSpeedNorm = !qFuzzyCompare(oldLength + 1, 1) ?
                oldSpeed / oldLength : QPointF(0, 0);
    QPointF newSpeedNorm = !qFuzzyCompare(newLength + 1, 1) ?
                newSpeed / newLength : QPointF(0, 0);
    QPointF speed = oldSpeedNorm + newSpeedNorm;

    // angle solely based on the speed
    qreal speedAngle = 0;
    if (speed.x() != 0) { // avoid division by zero
        speedAngle = std::atan(speed.y() / speed.x());
    } else if (speed.y() > 0) {
        // x == 0 && y != 0
        speedAngle = M_PI / 2;
    } else if (speed.y() < 0) {
        // x == 0 && y != 0
        speedAngle = -M_PI / 2;
    }
    if (speed.x() < 0) {
        speedAngle += M_PI;
    }

    // move 90 degrees
    speedAngle += M_PI / 2;

    qreal fixedAngle = m_angle;
    // check if the fixed angle needs to be flipped
    qreal diff = fixedAngle - speedAngle;
    while (diff >= M_PI) { // normalize diff between -180 and 180
        diff -= 2 * M_PI;
    }
    while (diff < -M_PI) {
        diff += 2 * M_PI;
    }

    if (std::abs(diff) > M_PI / 2) { // if absolute value < 90
        fixedAngle += M_PI;    // += 180
    }

    qreal dAngle = speedAngle - fixedAngle;

    // normalize dAngle between -90 and +90
    while (dAngle >= M_PI / 2) {
        dAngle -= M_PI;
    }
    while (dAngle < -M_PI / 2) {
        dAngle += M_PI;
    }

    qreal angle = fixedAngle + dAngle * (1.0 - m_fixation);

    return angle;
}

void KarbonCalligraphyTool::activate(ToolActivation activation, const QSet<KoShape*> &shapes)
{
    KoToolBase::activate(activation, shapes);

    if (!m_widget) {
        createOptionWidgets();
    }

    QAction *a = action("calligraphy_increase_width");
    connect(a, SIGNAL(triggered()), m_widget, SLOT(increaseWidth()), Qt::UniqueConnection);

    a = action("calligraphy_decrease_width");
    connect(a, SIGNAL(triggered()), m_widget, SLOT(decreaseWidth()), Qt::UniqueConnection);

    a = action("calligraphy_increase_angle");
    connect(a, SIGNAL(triggered()), m_widget, SLOT(increaseAngle()), Qt::UniqueConnection);

    a = action("calligraphy_decrease_angle");
    connect(a, SIGNAL(triggered()), m_widget, SLOT(decreaseAngle()), Qt::UniqueConnection);


    useCursor(Qt::CrossCursor);
    m_lastShape = 0;
}

void KarbonCalligraphyTool::deactivate()
{
    QAction *a = action("calligraphy_increase_width");
    disconnect(a, 0, this, 0);

    a = action("calligraphy_decrease_width");
    disconnect(a, 0, this, 0);

    a = action("calligraphy_increase_angle");
    disconnect(a, 0, this, 0);

    a = action("calligraphy_decrease_angle");
    disconnect(a, 0, this, 0);

    if (m_lastShape && canvas()->shapeManager()->shapes().contains(m_lastShape)) {
        KoSelection *selection = canvas()->shapeManager()->selection();
        selection->deselectAll();
        selection->select(m_lastShape);
    }

    KoToolBase::deactivate();
}

QList<QPointer<QWidget> > KarbonCalligraphyTool::createOptionWidgets()
{
    // if the widget don't exists yet create it
    QList<QPointer<QWidget> > widgets;

    //KoFillConfigWidget *fillWidget = new KoFillConfigWidget(0);
    //fillWidget->setWindowTitle(i18n("Fill"));
    //widgets.append(fillWidget);

    m_widget = new KarbonCalligraphyOptionWidget();
    connect(m_widget, SIGNAL(usePathChanged(bool)),
            this, SLOT(setUsePath(bool)));

    connect(m_widget, SIGNAL(usePressureChanged(bool)),
            this, SLOT(setUsePressure(bool)));

    connect(m_widget, SIGNAL(useAngleChanged(bool)),
            this, SLOT(setUseAngle(bool)));

    connect(m_widget, SIGNAL(widthChanged(double)),
            this, SLOT(setStrokeWidth(double)));

    connect(m_widget, SIGNAL(thinningChanged(double)),
            this, SLOT(setThinning(double)));

    connect(m_widget, SIGNAL(angleChanged(int)),
            this, SLOT(setAngle(int)));

    connect(m_widget, SIGNAL(fixationChanged(double)),
            this, SLOT(setFixation(double)));

    connect(m_widget, SIGNAL(capsChanged(double)),
            this, SLOT(setCaps(double)));

    connect(m_widget, SIGNAL(massChanged(double)),
            this, SLOT(setMass(double)));

    connect(m_widget, SIGNAL(dragChanged(double)),
            this, SLOT(setDrag(double)));

    connect(this, SIGNAL(pathSelectedChanged(bool)),
            m_widget, SLOT(setUsePathEnabled(bool)));

    // sync all parameters with the loaded profile
    m_widget->emitAll();
    m_widget->setObjectName(i18n("Calligraphy"));
    m_widget->setWindowTitle(i18n("Calligraphy"));
    widgets.append(m_widget);

    return widgets;
}

void KarbonCalligraphyTool::setStrokeWidth(double width)
{
    m_strokeWidth = width;
}

void KarbonCalligraphyTool::setThinning(double thinning)
{
    m_thinning = thinning;
}

void KarbonCalligraphyTool::setAngle(int angle)
{
    m_customAngle = angle;
}

void KarbonCalligraphyTool::setFixation(double fixation)
{
    m_fixation = fixation;
}

void KarbonCalligraphyTool::setMass(double mass)
{
    m_mass = mass * mass + 1;
}

void KarbonCalligraphyTool::setDrag(double drag)
{
    m_drag = drag;
}

void KarbonCalligraphyTool::setUsePath(bool usePath)
{
    m_usePath = usePath;
}

void KarbonCalligraphyTool::setUsePressure(bool usePressure)
{
    m_usePressure = usePressure;
}

void KarbonCalligraphyTool::setUseAngle(bool useAngle)
{
    m_useAngle = useAngle;
}

void KarbonCalligraphyTool::setCaps(double caps)
{
    m_caps = caps;
}

void KarbonCalligraphyTool::updateSelectedPath()
{
    KoPathShape *oldSelectedPath = m_selectedPath; // save old value

    KoSelection *selection = canvas()->shapeManager()->selection();
    if (selection) {
        // null pointer if it the selection isn't a KoPathShape
        // or if the selection is empty
        m_selectedPath =
                dynamic_cast<KoPathShape *>(selection->firstSelectedShape());

        // or if it's a KoPathShape but with no or more than one subpaths
        if (m_selectedPath && m_selectedPath->subpathCount() != 1) {
            m_selectedPath = 0;
        }

        // or if there ora none or more than 1 shapes selected
        if (selection->count() != 1) {
            m_selectedPath = 0;
        }

        // emit signal it there wasn't a selected path and now there is
        // or the other way around
        if ((m_selectedPath != 0) != (oldSelectedPath != 0)) {
            emit pathSelectedChanged(m_selectedPath != 0);
        }
    }
}
