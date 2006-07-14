/*
 *  kis_tool_star.h - part of Krita
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

/* Initial commit using tool_star. Emanuele Tamponi */

#ifndef KIS_TOOL_EXAMPLE_H_
#define KIS_TOOL_EXAMPLE_H_

#include "kis_tool_curve.h"
#include "kis_point.h"

class CurvePoint;
class KisPoint;
class KisCanvas;
class KisCurve;
class KisPainter;
class KisPoint;
class WdgToolExample;

class KisToolExample : public KisToolCurve {

    typedef KisToolCurve super;
    Q_OBJECT

    KisPoint m_start;
    KisPoint m_end;
    bool m_dragging;
    bool m_editing;

public:
    KisToolExample();
    virtual ~KisToolExample();

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
    virtual void doubleClick(KisDoubleClickEvent *);
    virtual void keyPress(QKeyEvent *);

public slots:

    void deactivate();

protected:
/*
    virtual void paint(KisCanvasPainter&);
    virtual void paint(KisCanvasPainter&, const QRect&);
*/
    KisPoint mouseOnHandle(KisPoint);

};


#include "kis_tool_factory.h"

class KisToolExampleFactory : public KisToolFactory {
    typedef KisToolFactory super;
public:
    KisToolExampleFactory() : super() {};
    virtual ~KisToolExampleFactory(){};

    virtual KisTool * createTool(KActionCollection * ac) {
        KisTool * t =  new KisToolExample();
        Q_CHECK_PTR(t);
        t->setup(ac);
        return t;
    }
    virtual KisID id() { return KisID("exampleshape", i18n("Example Tool")); }
};


#endif //__KIS_TOOL_EXAMPLE_H__
