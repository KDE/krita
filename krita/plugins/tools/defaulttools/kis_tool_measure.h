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

#include "kis_tool.h"

#include "kis_global.h"
#include "kis_types.h"
#include "KoToolFactory.h"
#include "kis_layer_shape.h"

class QPointF;
class QWidget;

class KoCanvasBase;

class KisToolMeasure : public KisTool {

    Q_OBJECT
    typedef KisTool super;

 public:
    KisToolMeasure(KoCanvasBase * canvas);
    virtual ~KisToolMeasure();

    virtual void mousePressEvent(KoPointerEvent *event);
    virtual void mouseMoveEvent(KoPointerEvent *event);
    virtual void mouseReleaseEvent(KoPointerEvent *event);

    virtual void paint(QPainter& gc, KoViewConverter &converter);

    QWidget * createOptionWidget();

signals:
    void sigDistanceChanged(double distance);
    void sigAngleChanged(double angle);

private:
    QRectF boundingRect();
    double angle();
    double distance();

    double deltaX() { return m_endPos.x() - m_startPos.x(); }
    double deltaY() { return m_startPos.y() - m_endPos.y(); }

private:
    bool m_dragging;
    QWidget *m_optWidget;

    QPointF m_startPos;
    QPointF m_endPos;
};


class KisToolMeasureFactory : public KoToolFactory {

public:

    KisToolMeasureFactory(QObject *parent, const QStringList&)
        : KoToolFactory(parent, "KritaShape/KisToolMeasure", i18n( "Measure" ))
        {
            setToolType( dynamicToolType() );
            setActivationShapeId( KIS_LAYER_SHAPE_ID );
            setPriority(0);
        }

    virtual ~KisToolMeasureFactory(){}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return new KisToolMeasure(canvas);
    }

};




#endif //KIS_TOOL_MEASURE_H_

