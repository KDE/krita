/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_TOOL_GRID_H_
#define _KIS_TOOL_GRID_H_

#include <kis_tool.h>
#include <KoToolFactory.h>

class KisCanvas2;

class KisToolGrid : public KisTool
{
    Q_OBJECT
    enum Mode {
        TRANSLATION,
        SCALE
    };
public:
    KisToolGrid(KoCanvasBase * canvas);
    virtual ~KisToolGrid();

    virtual void mousePressEvent(KoPointerEvent *event);
    virtual void mouseMoveEvent(KoPointerEvent *event);
    virtual void mouseReleaseEvent(KoPointerEvent *event);


public slots:
    virtual void activate(bool temp = false);
    void deactivate();

protected:

    virtual void paint(QPainter& gc, const KoViewConverter &converter);

private:
    KisCanvas2* m_canvas;
    QPointF m_dragStart;
    QPointF m_dragEnd;
    QPoint m_initialOffset;
    QPoint m_initialSpacing;

    Mode m_currentMode;
    bool m_dragging;
};


class KisToolGridFactory : public KoToolFactory
{

public:
    KisToolGridFactory(QObject *parent, const QStringList&)
            : KoToolFactory(parent, "KisToolGrid") {
        setToolTip(i18n("Edit the grid"));
        setToolType(TOOL_TYPE_VIEW);
        setIcon("krita_tool_grid");
        setPriority(17);
        setActivationShapeId("krita/always");
    };


    virtual ~KisToolGridFactory() {}

    virtual KoTool * createTool(KoCanvasBase * canvas) {
        return new KisToolGrid(canvas);
    }

};


#endif

