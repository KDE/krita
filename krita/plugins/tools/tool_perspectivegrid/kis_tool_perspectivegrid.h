/*
 *  kis_tool_perspectivegrid.h - part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License.
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
#include <kis_tool_non_paint.h>
#include <KoToolFactory.h>

class KisToolPerspectiveGrid : public KisToolNonPaint {
    Q_OBJECT
    enum PerspectiveGridEditionMode {
        MODE_CREATION, // This is the mode when there is not yet a perspective grid
        MODE_EDITING, // This is the mode when the grid has been created, and we are waiting for the user to click on a control box
        MODE_DRAGING_NODE, // In this mode one node is translated
        MODE_DRAGING_TRANSLATING_TWONODES // This mode is used when creating a new sub perspective grid
    };
    typedef KisToolNonPaint super;
public:
    KisToolPerspectiveGrid();
    virtual ~KisToolPerspectiveGrid();

    //
    // KisToolPaint interface
    //

    virtual void setup(KActionCollection *collection);
    virtual Q_UINT32 priority() { return 3; }
    virtual enumToolType toolType() { return TOOL_VIEW; }
    virtual void buttonPress(KoPointerEvent *event);
    virtual void move(KoPointerEvent *event);
    virtual void buttonRelease(KoPointerEvent *event);

//     QWidget* createOptionWidget(QWidget* parent);
//     virtual QWidget* optionWidget();

public slots:
    virtual void activate();
    void deactivate();

protected:
    virtual void paint(QPainter& gc);
    virtual void paint(QPainter& gc, const QRect& rc);
    void drawGridCreation(QPainter& gc);
    void drawGridCreation();
    void drawGrid(QPainter& gc);
    void drawGrid();

private:
    void drawSmallRectangle(QPainter& gc, QPoint p);
    bool mouseNear(const QPoint& mousep, const QPoint point);

protected:
    QPointF m_dragStart;
    QPointF m_dragEnd;

    bool m_dragging;
private:
    typedef QVector<QPointF> QPointFVector;
    KisCanvasSubject *m_subject;
    QPointFVector m_points;
    PerspectiveGridEditionMode m_mode;
    Q_INT32 m_handleSize, m_handleHalfSize;
    KisPerspectiveGridNodeSP m_selectedNode1, m_selectedNode2;

};


class KisToolPerspectiveGridFactory : public KoToolFactory {

public:
    KisToolPerspectiveGridFactory(QObject *parent, const QStringList&)
        : KoToolFactory(parent, "KisToolPerspectiveGrid", i18n( "Perspective grid" ))
        {
            setToolTip( i18n( "Edit the perspective grid" ) );
            setToolType( TOOL_TYPE_VIEW );
            setIcon( "tool_perspectivegrid" );
            setPriority( 0 );
        };


    virtual ~KisToolPerspectiveGridFactory() {}

    virtual KoTool * createTool(KoCanvasBase * canvas) {
        return new KisToolPerspectiveGrid(canvas);
    }

};


#endif

