/*
 *  kis_tool_perspectivegrid.h - part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_TOOL_PERSPECTIVE_GRID_H_
#define _KIS_TOOL_PERSPECTIVE_GRID_H_

#include <kis_perspective_grid.h>
#include <kis_tool.h>
#include <KoToolFactoryBase.h>

class KisCanvas2;

class KisToolPerspectiveGrid : public KisTool
{
    Q_OBJECT
    enum PerspectiveGridEditionMode {
        MODE_CREATION, // This is the mode when there is not yet a perspective grid
        MODE_EDITING, // This is the mode when the grid has been created, and we are waiting for the user to click on a control box
        MODE_DRAGING_NODE, // In this mode one node is translated
        MODE_DRAGING_TRANSLATING_TWONODES // This mode is used when creating a new sub perspective grid
    };

public:
    KisToolPerspectiveGrid(KoCanvasBase * canvas);
    virtual ~KisToolPerspectiveGrid();

    //
    // KisToolPaint interface
    //

    virtual quint32 priority() {
        return 3;
    }
    virtual void mousePressEvent(KoPointerEvent *event);
    virtual void mouseMoveEvent(KoPointerEvent *event);
    virtual void mouseReleaseEvent(KoPointerEvent *event);

//     QWidget* createOptionWidget();
//     virtual QWidget* optionWidget();

public slots:
    virtual void activate(bool temp = false);
    void deactivate();

protected:

    virtual void paint(QPainter& gc, const KoViewConverter &converter);
    void drawGridCreation(QPainter& gc);
    void drawGrid(QPainter& gc);

private:
    void drawSmallRectangle(QPainter& gc, const QPointF& p);
    bool mouseNear(const QPointF& mousep, const QPointF& point);
    KisPerspectiveGridNodeSP nodeNearPoint(KisSubPerspectiveGrid* grid, QPointF point);

protected:
    QPointF m_dragEnd;

    bool m_drawing;
    bool m_isFirstPoint;
    QPointF m_currentPt;
private:
    typedef QVector<QPointF> QPointFVector;

    QPointFVector m_points;
    PerspectiveGridEditionMode m_mode;
    qint32 m_handleSize, m_handleHalfSize;
    KisPerspectiveGridNodeSP m_selectedNode1, m_selectedNode2, m_higlightedNode;
    KisCanvas2* m_canvas;
};


class KisToolPerspectiveGridFactory : public KoToolFactoryBase
{

public:
    KisToolPerspectiveGridFactory(QObject *parent, const QStringList&)
            : KoToolFactoryBase(parent, "KisToolPerspectiveGrid") {
        setToolTip(i18n("Edit the perspective grid"));
        setToolType(TOOL_TYPE_VIEW);
        setIcon("tool_perspectivegrid");
        setPriority(16);
    };


    virtual ~KisToolPerspectiveGridFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase * canvas) {
        return new KisToolPerspectiveGrid(canvas);
    }

};


#endif

