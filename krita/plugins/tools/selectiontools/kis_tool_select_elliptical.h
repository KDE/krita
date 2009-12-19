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
#include "KoToolFactory.h"
#include "kis_tool_select_base.h"


class KisToolSelectElliptical : public KisToolSelectBase
{

    Q_OBJECT

public:
    KisToolSelectElliptical(KoCanvasBase * canvas);
    virtual ~KisToolSelectElliptical();

    virtual QWidget * createOptionWidget();
//     virtual enumToolType toolType() { return TOOL_SELECT; }

    virtual void paint(QPainter& gc, const KoViewConverter &converter);

    virtual void mousePressEvent(KoPointerEvent *e);
    virtual void mouseMoveEvent(KoPointerEvent *e);
    virtual void mouseReleaseEvent(KoPointerEvent *e);

private:
    void clearSelection();

private:
    QPointF m_centerPos;
    QPointF m_startPos;
    QPointF m_endPos;
    bool m_selecting;

};

class KisToolSelectEllipticalFactory : public KoToolFactory
{

public:
    KisToolSelectEllipticalFactory(QObject *parent, const QStringList&)
            : KoToolFactory(parent, "KisToolSelectElliptical", i18n("Elliptical Selection")) {
        setToolTip(i18n("Select an elliptical area"));
        setToolType(TOOL_TYPE_SELECTED);
        //setActivationShapeId( KIS_NODE_SHAPE_ID );
        setIcon("tool_elliptical_selection");
        setShortcut(KShortcut(Qt::Key_J));
        setPriority(53);
    }

    virtual ~KisToolSelectEllipticalFactory() {}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return  new KisToolSelectElliptical(canvas);
    }

};





#endif //__KIS_TOOL_SELECT_ELLIPTICAL_H__

