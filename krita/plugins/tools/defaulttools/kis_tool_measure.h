/*
 *
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_TOOL_MEASURE_H_
#define KIS_TOOL_MEASURE_H_

#include <QWidget>
#include <QLabel>

#include <KoUnit.h>

#include "kis_tool.h"
#include "kis_global.h"
#include "kis_types.h"
#include "KoToolFactory.h"
#include "flake/kis_node_shape.h"

class QPointF;
class QWidget;

class KoCanvasBase;


class KisToolMeasureOptionsWidget : public QWidget
{
    Q_OBJECT

public:
    KisToolMeasureOptionsWidget(QWidget* parent, double resolution);

public slots:
    void slotSetDistance(double distance);
    void slotSetAngle(double angle);
    void slotUnitChanged(int index);

private:
    void updateDistance();

    double m_resolution;
    QLabel* m_distanceLabel;
    QLabel* m_angleLabel;
    double m_distance;
    KoUnit m_unit;
};

class KisToolMeasure : public KisTool
{

    Q_OBJECT

public:
    KisToolMeasure(KoCanvasBase * canvas);
    virtual ~KisToolMeasure();

    virtual void mousePressEvent(KoPointerEvent *event);
    virtual void mouseMoveEvent(KoPointerEvent *event);
    virtual void mouseReleaseEvent(KoPointerEvent *event);

    virtual void paint(QPainter& gc, const KoViewConverter &converter);

    QWidget * createOptionWidget();

signals:
    void sigDistanceChanged(double distance);
    void sigAngleChanged(double angle);

private:
    QRectF boundingRect();
    double angle();
    double distance();

    double deltaX() {
        return m_endPos.x() - m_startPos.x();
    }
    double deltaY() {
        return m_startPos.y() - m_endPos.y();
    }

private:
    bool m_dragging;
    KisToolMeasureOptionsWidget *m_optWidget;

    QPointF m_startPos;
    QPointF m_endPos;
};


class KisToolMeasureFactory : public KoToolFactory
{

public:

    KisToolMeasureFactory(QObject *parent, const QStringList&)
            : KoToolFactory(parent, "KritaShape/KisToolMeasure") {
        setToolType(TOOL_TYPE_TRANSFORM);
        setToolTip(i18n("Measure the distance between two points"));
        setIcon("krita_tool_measure");
        //setActivationShapeId( KIS_NODE_SHAPE_ID );
        setPriority(16);
        setActivationShapeId("krita/always");
    }

    virtual ~KisToolMeasureFactory() {}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return new KisToolMeasure(canvas);
    }

};




#endif //KIS_TOOL_MEASURE_H_

