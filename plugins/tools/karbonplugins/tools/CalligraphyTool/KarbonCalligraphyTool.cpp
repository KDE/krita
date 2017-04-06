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
#include <KoCanvasResourceManager.h>
#include <kis_canvas_resource_provider.h>
#include <KoColor.h>
#include <KoShapePaintingContext.h>
#include <KoViewConverter.h>
#include <kis_canvas2.h>

#include <kis_painting_information_builder.h>
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
    : KisToolShape(canvas, QCursor(Qt::CrossCursor))
    , m_shape(0)
    , m_selectedPath(0)
    , m_isDrawing(false)
    , m_speed(0, 0)
    , m_lastShape(0)
{
    connect(canvas->selectedShapesProxy(), SIGNAL(selectionChanged()), SLOT(updateSelectedPath()));
    m_infoBuilder = new KisPaintingInformationBuilder();
    updateSelectedPath();
}

KarbonCalligraphyTool::~KarbonCalligraphyTool()
{
}

void KarbonCalligraphyTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (m_selectedPath && m_usePath) {
        painter.save();
        painter.setRenderHints(QPainter::Antialiasing, false);
        painter.setPen(Qt::red);   // TODO make configurable
        QRectF rect = m_selectedPath->boundingRect();
        QPointF p1 = converter.documentToView(rect.topLeft());
        QPointF p2 = converter.documentToView(rect.bottomRight());
        painter.drawRect(QRectF(p1, p2));
        painter.restore();
    }

    if (!m_intervalStore.isEmpty() && m_shape) {
        painter.save();
        painter.setPen(QColor(0, 200, 255));
        Q_FOREACH(KisPaintInformation p, m_intervalStore) {
            painter.drawEllipse(converter.documentToView(p.pos()), 1, 1);
        }
        if (!m_intervalStoreOld.isEmpty()) {
            Q_FOREACH(KisPaintInformation p, m_intervalStoreOld) {
                painter.drawEllipse(converter.documentToView(p.pos()), 1, 1);
            }
        }
        painter.restore();
    }

    if (!m_shape) {
        return;
    }

    painter.save();

    painter.setTransform(m_shape->absoluteTransformation(&converter) *
                         painter.transform());
    KoShapePaintingContext paintContext; //FIXME
    m_shape->paint(painter, converter, paintContext);

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
    m_intervalStore.clear();
    m_strokeTime.start();
    m_lastInfo = m_infoBuilder->startStroke(event, m_strokeTime.elapsed(), canvas()->resourceManager());
    if (!m_settings) {
        m_settings = new KisPropertiesConfiguration();
    }
    m_settings->setProperty("strokeWidth", currentStrokeWidth());
    m_shape = new KarbonCalligraphicShape(m_settings);
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
        addPoint(event, true);        // add last point
        m_isDrawing = false;
    }

    //m_shape->simplifyGuidePath();

    KUndo2Command *cmd = canvas()->shapeController()->addShape(m_shape);
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

void KarbonCalligraphyTool::addPoint(KoPointerEvent *event, bool lastPoint)
{
    if (m_pointCount == 0) {
        if (m_usePath && m_selectedPath) {
            m_selectedPathOutline = m_selectedPath->outline();
        }
        m_pointCount = 1;
        m_endOfPath = false;
        m_followPathPosition = 0;
        m_lastMousePos = event->point;
        m_firstPathPosition = event->point;
        m_deviceSupportsTilt = (event->xTilt() != 0 || event->yTilt() != 0);
        return;
    }

    if (m_endOfPath) {
        return;
    }

    ++m_pointCount;

    //setAngle(event);

    //QPointF newSpeed;
    //QPointF newPoint = calculateNewPoint(event->point, &newSpeed);
    // add the previous point
    KisPaintInformation paintInfo = m_infoBuilder->continueStroke(event, m_strokeTime.elapsed());
    //apply the path following:
    paintInfo.setPos(calculateNewPoint(paintInfo.pos(), m_firstPathPosition));
    m_intervalStore.append(paintInfo);

    qreal timeDiff = paintInfo.currentTime() - m_lastInfo.currentTime();
    if (timeDiff>m_smoothIntervalTime) {
        qreal distDiff = 0;
        for (int i=0; i<m_intervalStore.count()-1; i++) {
            distDiff += QLineF(m_intervalStore.at(i).pos(),
                               m_intervalStore.at(i+1).pos()).length();
        }
        distDiff = canvas()->viewConverter()->documentToView(QSizeF(distDiff, 0)).width();
        if (distDiff>m_smoothIntervalDistance) {
            m_shape->appendPoint(m_intervalStore.first());
            m_intervalStoreOld = m_intervalStore;
            m_intervalStore.clear();
            m_intervalStore.append(paintInfo);
            m_lastInfo = paintInfo;
        }
    }
    if (lastPoint) {
        qreal pressure = 0;
        for (int j=0; j<m_intervalStore.count(); j++) {
            pressure+=m_intervalStore.at(j).pressure();
        }
        paintInfo.setPressure(pressure / m_intervalStore.count());
        m_shape->appendPoint(paintInfo);
        m_intervalStore.count();
        m_intervalStore.clear();
        m_intervalStoreOld.clear();
    }
    //m_speed = newSpeed;
    //m_lastPoint = newPoint;
    canvas()->updateCanvas(m_shape->lastPieceBoundingRect());

}

QPointF KarbonCalligraphyTool::calculateNewPoint(const QPointF &mousePos, QPointF firstPathPosition)
{
    QPointF res = mousePos;
    if (m_usePath && m_selectedPath) {
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

        res = m_selectedPathOutline.pointAtPercent(t)
                + m_selectedPath->position();
    } else if (m_useAssistant) {
        if (static_cast<KisCanvas2*>(canvas())->paintingAssistantsDecoration()) {
            static_cast<KisCanvas2*>(canvas())->paintingAssistantsDecoration()->setOnlyOneAssistantSnap(false);
            res = static_cast<KisCanvas2*>(canvas())->paintingAssistantsDecoration()->adjustPosition(mousePos, firstPathPosition);
            //return (1.0 - m_magnetism) * point + m_magnetism * ap;
        }
    }
    return res;
}
void KarbonCalligraphyTool::activate(ToolActivation activation, const QSet<KoShape*> &shapes)
{
    KoToolBase::activate(activation, shapes);

    //useCursor(Qt::CrossCursor);
    m_lastShape = 0;
}

void KarbonCalligraphyTool::deactivate()
{
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

    KarbonCalligraphyOptionWidget *widget = new KarbonCalligraphyOptionWidget;
    connect(widget, SIGNAL(usePathChanged(bool)),
            this, SLOT(setUsePath(bool)));
    connect(widget, SIGNAL(useAssistantChanged(bool)),
            this, SLOT(setUseAssistant(bool)));
    connect(widget, SIGNAL(useNoAdjustChanged(bool)),
            this, SLOT(setNoAdjust(bool)));

    connect(widget, SIGNAL(settingsChanged(KisPropertiesConfigurationSP)),
            this, SLOT(setSettings(KisPropertiesConfigurationSP)));

    connect(widget, SIGNAL(smoothTimeChanged(double)),
            this, SLOT(setSmoothIntervalTime(double)));

    connect(widget, SIGNAL(smoothDistanceChanged(double)),
            this, SLOT(setSmoothIntervalDistance(double)));


    // add shortcuts
/*
    action = new QAction(i18n("Calligraphy: increase angle"), this);
    action->setShortcut(Qt::Key_Up);
    connect(action, SIGNAL(triggered()), widget, SLOT(increaseAngle()));
    addAction("calligraphy_increase_angle", action);

    action = new QAction(i18n("Calligraphy: decrease angle"), this);
    action->setShortcut(Qt::Key_Down);
    connect(action, SIGNAL(triggered()), widget, SLOT(decreaseAngle()));
    addAction("calligraphy_decrease_angle", action);
*/
    // sync all parameters with the loaded profile
    widget->emitAll();
    widget->setObjectName(i18n("Calligraphy"));
    widget->setWindowTitle(i18n("Calligraphy"));
    widgets.append(widget);

    return widgets;
}

void KarbonCalligraphyTool::setSmoothIntervalTime(double time)
{
    m_smoothIntervalTime = time;
}

void KarbonCalligraphyTool::setSmoothIntervalDistance(double dist)
{
    m_smoothIntervalDistance = dist;
}

void KarbonCalligraphyTool::setUsePath(bool usePath)
{
    m_usePath = usePath;
    m_useAssistant = !usePath;
}

void KarbonCalligraphyTool::setUseAssistant(bool useAssistant)
{
    m_usePath = !useAssistant;
    m_useAssistant = useAssistant;
}

void KarbonCalligraphyTool::setNoAdjust(bool none)
{
    if (none){
        m_usePath = false;
        m_useAssistant = false;
    }
}

void KarbonCalligraphyTool::setSettings(KisPropertiesConfigurationSP settings)
{
    settings->setProperty("strokeWidth", currentStrokeWidth());
    m_settings = settings;
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
