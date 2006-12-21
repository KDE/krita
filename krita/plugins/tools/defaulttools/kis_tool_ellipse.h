/*
 *  kis_tool_ellipse.h - part of Krayon
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#ifndef __KIS_TOOL_ELLIPSE_H__
#define __KIS_TOOL_ELLIPSE_H__

#include "kis_tool_shape.h"
#include "kis_types.h"
#include "KoToolFactory.h"
#include "kis_layer_shape.h"


class QPainter;
class KisPainter;

class KoCanvasBase;

class KisToolEllipse : public KisToolShape {

    typedef KisToolShape super;
    Q_OBJECT

public:
    KisToolEllipse(KoCanvasBase * canvas);
    virtual ~KisToolEllipse();

    virtual quint32 priority() { return 3; }
    //virtual enumToolType toolType() { return TOOL_SHAPE; }

    virtual void mousePressEvent(KoPointerEvent *event);
    virtual void mouseMoveEvent(KoPointerEvent *event);
    virtual void mouseReleaseEvent(KoPointerEvent *event);


    virtual void paint(QPainter& gc);
    virtual void paint(QPainter& gc, const QRect& rc);
    virtual void paint(QPainter& gc, KoViewConverter &converter);

    //protected:
    //virtual void draw(const QPointF& start, const QPointF& stop);

private:
    void paintEllipse();
    void paintEllipse(QPainter& gc, const QRect& rc);


protected:
    QPointF m_dragCenter;
    QPointF m_dragStart;
    QPointF m_dragEnd;

    bool m_dragging;
    KisPainter *m_painter;
};


#include "KoToolFactory.h"

class KisToolEllipseFactory : public KoToolFactory {

public:
    KisToolEllipseFactory(QObject *parent, const QStringList&)
        : KoToolFactory(parent, "KisToolEllipse", i18n( "Ellipse" ))
        {
            setToolTip( i18n( "Draw an ellipse" ) );
            setToolType( TOOL_TYPE_SHAPE );
            setIcon( "tool_ellipse" );
            setPriority( 0 );
        }

    virtual ~KisToolEllipseFactory(){}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return  new KisToolEllipse(canvas);
    }

};


#endif //__KIS_TOOL_ELLIPSE_H__

