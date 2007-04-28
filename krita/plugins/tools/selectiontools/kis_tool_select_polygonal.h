/*
 *  kis_tool_select_polygonal.h - part of Krayon^WKrita
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef __selecttoolpolygonal_h__
#define __selecttoolpolygonal_h__

#include "KoToolFactory.h"

#include "kis_layer_shape.h"
#include "kis_selection.h"
#include "kis_tool.h"

class KisSelectionOptions;

class KisToolSelectPolygonal : public KisTool {

    typedef KisTool super;
    Q_OBJECT
public:
    KisToolSelectPolygonal(KoCanvasBase *canvas);
    virtual ~KisToolSelectPolygonal();

    virtual void mousePressEvent(KoPointerEvent *event);
    virtual void mouseMoveEvent(KoPointerEvent *event);
    virtual void mouseReleaseEvent(KoPointerEvent *event);
    virtual void mouseDoubleClickEvent(KoPointerEvent * event);
    virtual void keyPressEvent(QKeyEvent *e);

    QWidget* createOptionWidget();
    virtual QWidget* optionWidget();

    virtual void paint( QPainter &painter, KoViewConverter &converter );

public slots:
    virtual void slotSetAction(int);
    virtual void activate();
    void deactivate();

protected:
    void finish();
    QRectF dragBoundingRect();

protected:
    QPointF m_dragStart;
    QPointF m_dragEnd;

    bool m_dragging;
private:
    vQPointF m_points;
    KisSelectionOptions * m_optWidget;
    enumSelectionMode m_selectAction;
};


class KisToolSelectPolygonalFactory : public KoToolFactory {

public:
    KisToolSelectPolygonalFactory(QObject *parent, const QStringList&)
        : KoToolFactory(parent, "KisToolSelectPolygonal",
            i18n( "Polygonal Selection") )
        {
            setToolTip( i18n( "Select a polygonal region" ) );
            setToolType( TOOL_TYPE_SELECTED );
            setIcon( "tool_polygonal_selection" );
            setPriority( 0 );
            setActivationShapeId( KIS_LAYER_SHAPE_ID );
        }

    virtual ~KisToolSelectPolygonalFactory(){}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return new KisToolSelectPolygonal(canvas);
    }
};

#endif //__selecttoolpolygonal_h__

