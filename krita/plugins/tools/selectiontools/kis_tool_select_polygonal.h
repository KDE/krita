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

#ifndef KIS_TOOL_SELECT_POLYGONAL_H_
#define KIS_TOOL_SELECT_POLYGONAL_H_

#include "KoToolFactory.h"
#include "krita/ui/tool/kis_tool_select_base.h"
#include "kis_tool_polyline_base.h"

class KisToolSelectPolygonal : public KisToolSelectBase
{

    Q_OBJECT
public:
    KisToolSelectPolygonal(KoCanvasBase *canvas);
    virtual ~KisToolSelectPolygonal();

    virtual void keyPressEvent(QKeyEvent *e);

    QWidget* createOptionWidget();

private:
    class LokalTool : public KisToolPolylineBase {
    public:
        LokalTool(KoCanvasBase * canvas, KisToolSelectPolygonal* selectingTool)
            : KisToolPolylineBase(canvas), m_selectingTool(selectingTool) {}
        void finishPolyline(const QVector<QPointF> &points);
    private:
        KisToolSelectPolygonal* const m_selectingTool;
    };
    LokalTool m_lokalTool;

    virtual void paint(QPainter& gc, const KoViewConverter &converter) {m_lokalTool.paint(gc, converter);}
    virtual void mousePressEvent(KoPointerEvent *e) {m_lokalTool.mousePressEvent(e);}
    virtual void mouseMoveEvent(KoPointerEvent *e) {m_lokalTool.mouseMoveEvent(e);}
    virtual void mouseReleaseEvent(KoPointerEvent *e) {m_lokalTool.mouseReleaseEvent(e);}
    virtual void mouseDoubleClickEvent(KoPointerEvent * e) {m_lokalTool.mouseDoubleClickEvent(e);}
    virtual void deactivate() {m_lokalTool.deactivate();}

    friend class LokalTool;
};


class KisToolSelectPolygonalFactory : public KoToolFactory
{

public:
    KisToolSelectPolygonalFactory(QObject *parent, const QStringList&)
            : KoToolFactory(parent, "KisToolSelectPolygonal") {
        setToolTip(i18n("Select a polygonal region"));
        setToolType(TOOL_TYPE_SELECTED);
        setIcon("tool_polygonal_selection");
        setPriority(54);
        //setActivationShapeId( KIS_NODE_SHAPE_ID );
    }

    virtual ~KisToolSelectPolygonalFactory() {}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return new KisToolSelectPolygonal(canvas);
    }
};

#endif //__selecttoolpolygonal_h__

