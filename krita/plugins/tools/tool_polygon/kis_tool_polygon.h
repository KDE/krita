/*
 *  kis_tool_polygon.h - part of Krita
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

#ifndef KIS_TOOL_POLYGON_H_
#define KIS_TOOL_POLYGON_H_

#include <qvaluevector.h>

#include "kis_tool_shape.h"

class KisCanvas;
class KisDoc;
class KisPainter;
class KisView;
class KisRect;

class KisToolPolygon : public KisToolShape {

    typedef KisToolShape super;
    Q_OBJECT

public:
    KisToolPolygon();
    virtual ~KisToolPolygon();

        //
        // KisCanvasObserver interface
        //

        virtual void update (KisCanvasSubject *subject);

        //
        // KisToolPaint interface
        //

    virtual void setup(KActionCollection *collection);
    virtual enumToolType toolType() { return TOOL_SHAPE; }
    virtual void buttonPress(KisButtonPressEvent *event);
    virtual void move(KisMoveEvent *event);
    virtual void buttonRelease(KisButtonReleaseEvent *event);

protected:
    virtual void paint(QPainter& gc);
    virtual void paint(QPainter& gc, const QRect& rc);
    void draw(QPainter& gc);
    void draw();

protected:
    KisPoint m_dragStart;
    KisPoint m_dragEnd;

    bool m_dragging;
    KisImageSP m_currentImage;
private:
        typedef QValueVector<KisPoint> KisPointVector;
        KisPointVector m_points;
};


#include "kis_tool_factory.h"

class KisToolPolygonFactory : public KisToolFactory {
    typedef KisToolFactory super;
public:
    KisToolPolygonFactory() : super() {};
    virtual ~KisToolPolygonFactory(){};

    virtual KisTool * createTool(KActionCollection * ac) {
        KisTool * t =  new KisToolPolygon();
        Q_CHECK_PTR(t);
        t -> setup(ac);
        return t;
    }
    virtual KisID id() { return KisID("polygon", i18n("Polygon tool")); }
};


#endif //__KIS_TOOL_POLYGON_H__
