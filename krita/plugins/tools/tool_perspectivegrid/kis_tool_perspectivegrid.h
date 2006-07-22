/*
 *  kis_tool_perspectivegrid.h - part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_TOOL_PERSPECTIVE_GRID_H_
#define _KIS_TOOL_PERSPECTIVE_GRID_H_

#include <kis_perspective_grid.h>
#include <kis_point.h>
#include <kis_tool_non_paint.h>
#include <kis_tool_factory.h>

class KisToolPerspectiveGrid : public KisToolNonPaint {
    enum PerspectiveGridEditionMode {
        MODE_CREATION, // This is the mode when there is not yet a perspective grid
        MODE_EDITING, // This is the mode when the grid has been created, and we are waiting for the user to click on a control box
        MODE_DRAGING_NODE, // In this mode one node is translated
        MODE_DRAGING_TRANSLATING_TWONODES // This mode is used when creating a new sub perspective grid
    };
    typedef KisToolNonPaint super;
    Q_OBJECT
public:
    KisToolPerspectiveGrid();
    virtual ~KisToolPerspectiveGrid();

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
    
//     QWidget* createOptionWidget(QWidget* parent);
//     virtual QWidget* optionWidget();

public slots:
    virtual void activate();
    void deactivate();

protected:
    virtual void paint(KisCanvasPainter& gc);
    virtual void paint(KisCanvasPainter& gc, const QRect& rc);
    void drawGridCreation(KisCanvasPainter& gc);
    void drawGridCreation();
    void drawGrid(KisCanvasPainter& gc);
    void drawGrid();

private:
    void drawSmallRectangle(KisCanvasPainter& gc, QPoint p);
    bool mouseNear(const QPoint& mousep, const QPoint point);

protected:
    KisPoint m_dragStart;
    KisPoint m_dragEnd;

    bool m_dragging;
private:
    typedef QValueVector<KisPoint> KisPointVector;
    KisCanvasSubject *m_subject;
    KisPointVector m_points;
    PerspectiveGridEditionMode m_mode;
    Q_INT32 m_handleSize, m_handleHalfSize;
    KisPerspectiveGridNodeSP m_selectedNode1, m_selectedNode2;

};


class KisToolPerspectiveGridFactory : public KisToolFactory {
    typedef KisToolFactory super;
public:
    KisToolPerspectiveGridFactory() : super() {};
    virtual ~KisToolPerspectiveGridFactory(){};

    virtual KisTool * createTool(KActionCollection * ac) {
        KisTool * t =  new KisToolPerspectiveGrid();
        Q_CHECK_PTR(t);
        t->setup(ac);
        return t;
    }
    virtual KisID id() { return KisID("perspectivegridtool", i18n("Perspective Grid Tool")); }
};


#endif

