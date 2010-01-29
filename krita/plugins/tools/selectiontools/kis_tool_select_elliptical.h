/*
 *  kis_tool_select_elliptical.h - part of Krayon^WKrita
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

#ifndef __KIS_TOOL_SELECT_ELLIPTICAL_H__
#define __KIS_TOOL_SELECT_ELLIPTICAL_H__

#include <QPoint>
#include "KoToolFactoryBase.h"
#include "krita/ui/tool/kis_tool_select_base.h"
#include "kis_tool_ellipse_base.h"


class KisToolSelectElliptical : public KisToolSelectBase
{

    Q_OBJECT

public:
    KisToolSelectElliptical(KoCanvasBase * canvas);
    virtual ~KisToolSelectElliptical();

    virtual QWidget * createOptionWidget();

private:
    class LokalTool : public KisToolEllipseBase {
    public:
        LokalTool(KoCanvasBase * canvas, KisToolSelectElliptical* selectingTool)
            : KisToolEllipseBase(canvas), m_selectingTool(selectingTool) {}
        void finishEllipse(const QRectF &rect);
    private:
        KisToolSelectElliptical* const m_selectingTool;
    };
    LokalTool m_lokalTool;

    virtual void paint(QPainter& gc, const KoViewConverter &converter) {m_lokalTool.paint(gc, converter);}
    virtual void mousePressEvent(KoPointerEvent *e) {m_lokalTool.mousePressEvent(e);}
    virtual void mouseMoveEvent(KoPointerEvent *e) {m_lokalTool.mouseMoveEvent(e);}
    virtual void mouseReleaseEvent(KoPointerEvent *e) {m_lokalTool.mouseReleaseEvent(e);}
    virtual void deactivate() {m_lokalTool.deactivate();}

    friend class LokalTool;
};

class KisToolSelectEllipticalFactory : public KoToolFactoryBase
{

public:
    KisToolSelectEllipticalFactory(QObject *parent, const QStringList&)
            : KoToolFactoryBase(parent, "KisToolSelectElliptical") {
        setToolTip(i18n("Select an elliptical area"));
        setToolType(TOOL_TYPE_SELECTED);
        //setActivationShapeId( KIS_NODE_SHAPE_ID );
        setIcon("tool_elliptical_selection");
        setShortcut(KShortcut(Qt::Key_J));
        setPriority(53);
    }

    virtual ~KisToolSelectEllipticalFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return  new KisToolSelectElliptical(canvas);
    }

};





#endif //__KIS_TOOL_SELECT_ELLIPTICAL_H__

