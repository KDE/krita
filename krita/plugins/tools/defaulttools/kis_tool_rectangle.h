/*
 *  kis_tool_rectangle.h - part of KImageShop^WKrayon^WKrita
 *
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
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

#ifndef __KIS_TOOL_RECTANGLE_H__
#define __KIS_TOOL_RECTANGLE_H__

#include <QRect>

#include "kis_tool_shape.h"
#include "kis_types.h"
#include "KoToolFactory.h"
#include "flake/kis_node_shape.h"


class QPainter;
class KisPainter;

class QPoint;

class KoCanvasBase;

class KisToolRectangle : public KisToolShape
{

    Q_OBJECT

public:
    KisToolRectangle(KoCanvasBase * canvas);
    virtual ~KisToolRectangle();

    virtual void mousePressEvent(KoPointerEvent *event);
    virtual void mouseMoveEvent(KoPointerEvent *event);
    virtual void mouseReleaseEvent(KoPointerEvent *event);

    virtual void paint(QPainter& gc, const KoViewConverter &converter);

private:
    void paintRectangle(QPainter& gc, const QRect& rc);

protected:
    int m_lineThickness;

    QPointF m_dragCenter;
    QPointF m_dragStart;
    QPointF m_dragEnd;
    QRect m_final_lines;

    bool m_dragging;

    KisPainter *m_painter;

    //KisImageWSP m_currentImage;
};

class KisToolRectangleFactory : public KoToolFactory
{

public:
    KisToolRectangleFactory(QObject *parent, const QStringList&)
            : KoToolFactory(parent, "KritaShape/KisToolRectangle", i18n("Rectangle")) {
        setToolTip(i18n("Draw a rectangle"));

        setToolType(TOOL_TYPE_SHAPE);
        //setActivationShapeId( KIS_NODE_SHAPE_ID );
        setIcon("krita_tool_rectangle");
        //setShortcut( Qt::Key_F6 );
        setPriority(2);
        setInputDeviceAgnostic(false);
    }

    virtual ~KisToolRectangleFactory() {}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return  new KisToolRectangle(canvas);
    }

};


#endif // __KIS_TOOL_RECTANGLE_H__

