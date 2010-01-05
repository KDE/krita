/*
 *  kis_tool_select_rectangular.h - part of Krita
 *
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
 *                2002 Patrick Julien <freak@codepimps.org>
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

#ifndef KIS_TOOL_SELECT_RECTANGULAR_H_
#define KIS_TOOL_SELECT_RECTANGULAR_H_

#include "krita/ui/tool/kis_tool_select_base.h"
#include "KoToolFactory.h"
#include "kis_tool_rectangle_base.h"

class KoCanvasBase;

class KisToolSelectRectangular : public KisToolSelectBase
{

    Q_OBJECT

public:
    KisToolSelectRectangular(KoCanvasBase * canvas);
    virtual ~KisToolSelectRectangular();

    virtual QWidget * createOptionWidget();

private:
    class LokalTool : public KisToolRectangleBase {
    public:
        LokalTool(KoCanvasBase * canvas, KisToolSelectRectangular* selectingTool)
            : KisToolRectangleBase(canvas), m_selectingTool(selectingTool) {}
    public:
        void finishRect(const QRectF& rect);
    private:
        KisToolSelectRectangular* const m_selectingTool;
    };
    LokalTool m_lokalTool;

    virtual void paint(QPainter& gc, const KoViewConverter &converter) {m_lokalTool.paint(gc, converter);}
    virtual void mousePressEvent(KoPointerEvent *e) {m_lokalTool.mousePressEvent(e);}
    virtual void mouseMoveEvent(KoPointerEvent *e) {m_lokalTool.mouseMoveEvent(e);}
    virtual void mouseReleaseEvent(KoPointerEvent *e) {m_lokalTool.mouseReleaseEvent(e);}
    virtual void deactivate() {m_lokalTool.deactivate();}
};

class KisToolSelectRectangularFactory : public KoToolFactory
{

public:
    KisToolSelectRectangularFactory(QObject *parent, const QStringList&)
            : KoToolFactory(parent, "KisToolSelectRectangular") {
        setToolTip(i18n("Select a rectangular area"));
        setToolType(TOOL_TYPE_SELECTED);
        //setActivationShapeId( KIS_NODE_SHAPE_ID );
        setIcon("tool_rect_selection");
        setShortcut(KShortcut(Qt::Key_R));
        setPriority(52);
    }

    virtual ~KisToolSelectRectangularFactory() {}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return  new KisToolSelectRectangular(canvas);
    }
};



#endif // KIS_TOOL_SELECT_RECTANGULAR_H_

