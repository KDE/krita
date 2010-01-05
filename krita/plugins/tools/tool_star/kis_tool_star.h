/*
 *  kis_tool_star.h - part of Krita
 *
 *  Copyright (c) 2004 Michael Thaler <michael Thaler@physik.tu-muenchen.de>
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

#ifndef KIS_TOOL_STAR_H_
#define KIS_TOOL_STAR_H_

#include "kis_tool_shape.h"
#include "ui_wdg_tool_star.h"
#include "flake/kis_node_shape.h"


class WdgToolStar : public QWidget, public Ui::WdgToolStar
{
    Q_OBJECT

public:
    WdgToolStar(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

class KisToolStar : public KisToolShape
{

    Q_OBJECT

public:
    KisToolStar(KoCanvasBase * canvas);
    virtual ~KisToolStar();

    virtual QWidget* createOptionWidget();

    virtual void mousePressEvent(KoPointerEvent *event);
    virtual void mouseMoveEvent(KoPointerEvent *event);
    virtual void mouseReleaseEvent(KoPointerEvent *event);

    virtual void paint(QPainter& gc, const KoViewConverter &converter);

protected:
    int m_lineThickness;

    QPointF m_dragStart;
    QPointF m_dragEnd;
    QRect m_final_lines;

    bool m_dragging;
private:
    vQPointF starCoordinates(int N, double mx, double my, double x, double y);
    QRectF boundingRect();

    qint32 m_innerOuterRatio;
    qint32 m_vertices;
    WdgToolStar* m_optWidget;
};


#include "KoToolFactory.h"

class KisToolStarFactory : public KoToolFactory
{

public:
    KisToolStarFactory(QObject *parent, const QStringList&)
            : KoToolFactory(parent, "KisToolStar") {
        setToolTip(i18n("Draw a star with the current brush"));
        setToolType(TOOL_TYPE_SHAPE);
        setPriority(6);
        setIcon("tool_star");
        //setActivationShapeId( KIS_NODE_SHAPE_ID );
        setInputDeviceAgnostic(false);
    }

    virtual ~KisToolStarFactory() {}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return new KisToolStar(canvas);
    }

};


#endif //__KIS_TOOL_STAR_H__
