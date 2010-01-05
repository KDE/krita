/*
 *  kis_tool_select_freehand.h - part of Krayon^WKrita
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

#ifndef KIS_TOOL_SELECT_OUTLINE_H_
#define KIS_TOOL_SELECT_OUTLINE_H_

#include <QPoint>
#include <KoToolFactory.h>
#include "krita/ui/tool/kis_tool_select_base.h"

class KisToolSelectOutline : public KisToolSelectBase
{

    Q_OBJECT
public:
    KisToolSelectOutline(KoCanvasBase *canvas);
    virtual ~KisToolSelectOutline();

    virtual void mousePressEvent(KoPointerEvent *event);
    virtual void mouseMoveEvent(KoPointerEvent *event);
    virtual void mouseReleaseEvent(KoPointerEvent *event);

    virtual void paint(QPainter& gc, const KoViewConverter &converter);

    QWidget* createOptionWidget();

public slots:
    virtual void deactivate();

private:
    void updateFeedback();

    bool m_dragging;
    vQPointF m_points;
};


class KisToolSelectOutlineFactory : public KoToolFactory
{

public:
    KisToolSelectOutlineFactory(QObject *parent, const QStringList&)
            : KoToolFactory(parent, "KisToolSelectOutline") {
        setToolTip(i18n("Select an area by its outline"));
        setToolType(TOOL_TYPE_SELECTED);
        setIcon("tool_outline_selection");
        setPriority(55);
        //setActivationShapeId( KIS_NODE_SHAPE_ID );
    }

    virtual ~KisToolSelectOutlineFactory() {}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return new KisToolSelectOutline(canvas);
    }
};


#endif //__selecttoolfreehand_h__

