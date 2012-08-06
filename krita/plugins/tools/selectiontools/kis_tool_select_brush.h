/*
 *  kis_tool_select_brush.h - part of Krita
 *
 *  Copyright (C) 2010 Celarek Adam <kdedev at xibo dot at>
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

#ifndef __KIS_TOOL_SELECT_BRUSH_H__
#define __KIS_TOOL_SELECT_BRUSH_H__

#include "KoToolFactoryBase.h"
#include "krita/ui/tool/kis_tool_select_base.h"
#include <KoIcon.h>
#include <QPointF>
#include <QPainterPath>


class KisToolSelectBrush : public KisToolSelectBase
{

    Q_OBJECT

public:
    KisToolSelectBrush(KoCanvasBase * canvas);
    virtual ~KisToolSelectBrush();

    virtual QWidget * createOptionWidget();

public:
    virtual void paint(QPainter& gc, const KoViewConverter &converter);
    virtual void mousePressEvent(KoPointerEvent *e);
    virtual void mouseMoveEvent(KoPointerEvent *e);
    virtual void mouseReleaseEvent(KoPointerEvent *e);

public slots:
    virtual void deactivate();

    void slotSetBrushSize(int size);

protected:
    void applyToSelection(const QPainterPath& selection);
    void resetSelection();
    void addPoint(const QPointF& point);
    void addGap(const QPointF& start, const QPointF& end);

private:
    qreal m_brushRadius;
    bool m_dragging;
    QPainterPath m_selection;
    QPointF m_lastPoint;
    QPoint m_lastMousePosition;

};

class KisToolSelectBrushFactory : public KoToolFactoryBase
{

public:
    KisToolSelectBrushFactory(const QStringList&)
            : KoToolFactoryBase("KisToolSelectBrush") {
        setToolTip(i18n("Select by brush"));
        setToolType(TOOL_TYPE_SELECTED);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setIconName(koIconNameCStr("tool_brush_selection"));
        //setShortcut(KShortcut(Qt::Key_B));
        setPriority(53);
    }

    virtual ~KisToolSelectBrushFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return  new KisToolSelectBrush(canvas);
    }

};





#endif //__KIS_TOOL_SELECT_BRUSH_H__

