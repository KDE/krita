/*
 *  kis_tool_polyline.h - part of Krita
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

#ifndef KIS_TOOL_POLYLINE_H_
#define KIS_TOOL_POLYLINE_H_

#include <q3valuevector.h>
#include <QString>
//Added by qt3to4:
#include <QKeyEvent>

#include "kis_tool_paint.h"
#include "kis_point.h"

class KisCanvas;
class KisDoc;
class KisPainter;
class KisView;
class KisRect;


class KisToolPolyline : public KisToolPaint {

    typedef KisToolPaint super;
    Q_OBJECT

public:
    KisToolPolyline();
    virtual ~KisToolPolyline();

    //
    // KisCanvasObserver interface
    //

    virtual void update (KisCanvasSubject *subject);

    //
    // KisToolPaint interface
    //

    virtual void setup(KActionCollection *collection);
    virtual enumToolType toolType() { return TOOL_SHAPE; }
    virtual quint32 priority() { return 5; }

    virtual void buttonPress(KisButtonPressEvent *event);
    virtual void doubleClick(KisDoubleClickEvent *e);
    virtual void move(KisMoveEvent *event);
    virtual void buttonRelease(KisButtonReleaseEvent *event);
    virtual QString quickHelp() const;
    void finish();
    virtual void keyPress(QKeyEvent *e);

public slots:

    void deactivate();
    
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
    typedef Q3ValueVector<KisPoint> KisPointVector;
    KisPointVector m_points;
};


#include "kis_tool_factory.h"

class KisToolPolylineFactory : public KisToolFactory {
    typedef KisToolFactory super;
public:
    KisToolPolylineFactory() : super() {};
    virtual ~KisToolPolylineFactory(){};

    virtual KisTool * createTool(KActionCollection * ac) {
        KisTool * t =  new KisToolPolyline();
        Q_CHECK_PTR(t);
        t->setup(ac);
        return t;
    }
    virtual KoID id() { return KoID("polyline", i18n("Polyline Tool")); }
};


#endif //__KIS_TOOL_POLYLINE_H__
