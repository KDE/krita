/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_TOOL_GRID_H_
#define _KIS_TOOL_GRID_H_

#include <kis_tool.h>
#include <KoToolFactoryBase.h>
#include <kis_icon.h>

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


    void beginPrimaryAction(KoPointerEvent *event);
    void continuePrimaryAction(KoPointerEvent *event);
    void endPrimaryAction(KoPointerEvent *event);

    void beginAlternateAction(KoPointerEvent *event, AlternateAction action);
    void continueAlternateAction(KoPointerEvent *event, AlternateAction action);
    void endAlternateAction(KoPointerEvent *event, AlternateAction action);

    virtual void keyPressEvent(QKeyEvent* event);

public Q_SLOTS:
    virtual void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes);

protected:

    virtual void paint(QPainter& gc, const KoViewConverter &converter);

private:
    KisCanvas2* m_canvas;
    QPointF m_dragStart;
    QPointF m_dragEnd;
    QPoint m_initialOffset;
    QPoint m_initialSpacing;
};


class KisToolGridFactory : public KoToolFactoryBase
{

public:
    KisToolGridFactory(const QStringList&)
            : KoToolFactoryBase("KisToolGrid") {
        setToolTip(i18n("Grid Tool"));
        setToolType(TOOL_TYPE_VIEW);
        setIconName(koIconNameCStr("krita_tool_grid"));
        setPriority(17);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    };


    virtual ~KisToolGridFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase * canvas) {
        return new KisToolGrid(canvas);
    }

};


#endif

