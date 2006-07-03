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

#ifndef KIS_TOOL_STAR_H_
#define KIS_TOOL_STAR_H_

#include "kis_tool_shape.h"

class KisCanvas;
class KisDoc;
class KisPainter;
class KisView;
class KisRect;
class WdgToolCurves;

class KisToolCurves : public KisToolShape {

    typedef KisToolShape super;
    Q_OBJECT

public:
    KisToolCurves();
    virtual ~KisToolCurves();

        //
        // KisCanvasObserver interface
        //

        virtual void update (KisCanvasSubject *subject);

        virtual QWidget* createOptionWidget(QWidget* parent);

        //
        // KisToolPaint interface
        //

    virtual void setup(KActionCollection *collection);
    virtual enumToolType toolType() { return TOOL_SHAPE; }
    virtual void buttonPress(KisButtonPressEvent *event);
    virtual void move(KisMoveEvent *event);
    virtual void buttonRelease(KisButtonReleaseEvent *event);

protected:
    virtual void draw(const KisPoint& start, const KisPoint& stop);
    //virtual void draw(KisPainter *gc, const QRect& rc);

protected:
    int m_lineThickness;

    KisImageSP m_currentImage;
private:

    WdgToolCurves* m_optWidget;
};


#include "kis_tool_factory.h"

class KisToolCurvesFactory : public KisToolFactory {
    typedef KisToolFactory super;
public:
    KisToolCurvesFactory() : super() {};
    virtual ~KisToolCurvesFactory(){};

    virtual KisTool * createTool(KActionCollection * ac) {
        KisTool * t =  new KisToolCurves();
        Q_CHECK_PTR(t);
        t->setup(ac);
        return t;
    }
    virtual KisID id() { return KisID("curvesshape", i18n("Curves Tool")); }
};


#endif //__KIS_TOOL_STAR_H__
