/*
 *  kis_tool_select_polygonal.h - part of Krayon^WKrita
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

#ifndef __selecttoolpolygonal_h__
#define __selecttoolpolygonal_h__

#include "kis_point.h"
#include "kis_tool_non_paint.h"
#include "kis_tool_factory.h"
#include "kis_selection.h"

class KisSelectionOptions;

class KisToolSelectPolygonal : public KisToolNonPaint {

    typedef KisToolNonPaint super;
    Q_OBJECT
public:
    KisToolSelectPolygonal();
    virtual ~KisToolSelectPolygonal();

        //
        // KisCanvasObserver interface
        //

    virtual void update (KisCanvasSubject *subject);

        //
        // KisToolPaint interface
        //

    virtual void setup(KActionCollection *collection);
    virtual Q_UINT32 priority() { return 5; }
    virtual enumToolType toolType() { return TOOL_SELECT; }
    virtual void buttonPress(KisButtonPressEvent *event);
    virtual void move(KisMoveEvent *event);
    virtual void buttonRelease(KisButtonReleaseEvent *event);
    virtual void doubleClick(KisDoubleClickEvent * event);
    
    void finish();
    QWidget* createOptionWidget(QWidget* parent);
    virtual QWidget* optionWidget();

public slots:
    virtual void slotSetAction(int);
    virtual void activate();
    void deactivate();

protected:
    virtual void paint(KisCanvasPainter& gc);
    virtual void paint(KisCanvasPainter& gc, const QRect& rc);
    void draw(KisCanvasPainter& gc);
    void draw();

protected:
    KisPoint m_dragStart;
    KisPoint m_dragEnd;

    bool m_dragging;
private:
    typedef QValueVector<KisPoint> KisPointVector;
    KisCanvasSubject *m_subject;
    KisPointVector m_points;
    KisSelectionOptions * m_optWidget;
    enumSelectionMode m_selectAction;
};


class KisToolSelectPolygonalFactory : public KisToolFactory {
    typedef KisToolFactory super;
public:
    KisToolSelectPolygonalFactory() : super() {};
    virtual ~KisToolSelectPolygonalFactory(){};

    virtual KisTool * createTool(KActionCollection * ac) {
        KisTool * t =  new KisToolSelectPolygonal();
        Q_CHECK_PTR(t);
        t->setup(ac);
        return t;
    }
    virtual KisID id() { return KisID("polygonalselect", i18n("Polygonal Select Tool")); }
};


#endif //__selecttoolpolygonal_h__

