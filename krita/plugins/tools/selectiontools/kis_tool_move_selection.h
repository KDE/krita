/*
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

#ifndef KIS_TOOL_MOVE_H_
#define KIS_TOOL_MOVE_H_

#include "kis_tool_non_paint.h"
#include "kis_tool_factory.h"

// XXX: Moving is not nearly smooth enough!
class KisToolMoveSelection : public KisToolNonPaint {

    typedef KisToolNonPaint super;
    Q_OBJECT

public:
    KisToolMoveSelection();
    virtual ~KisToolMoveSelection();

public:
    virtual void update(KisCanvasSubject *subject);

public:
    virtual void setup(KActionCollection *collection);
    virtual enumToolType toolType() { return TOOL_SELECT; }
    virtual Q_UINT32 priority() { return 10; }

    virtual void buttonPress(KisButtonPressEvent *e);
    virtual void move(KisMoveEvent *e);
    virtual void buttonRelease(KisButtonReleaseEvent *e);

private:
    KisCanvasSubject *m_subject;
    QPoint m_dragStart;
    QPoint m_layerStart;
    QPoint m_layerPosition;
    bool m_dragging;
};


class KisToolMoveSelectionFactory : public KisToolFactory {
    typedef KisToolFactory super;
public:
    KisToolMoveSelectionFactory() : super() {};
    virtual ~KisToolMoveSelectionFactory(){};
    
    virtual KisTool * createTool(KActionCollection * ac) {
        KisTool * t =  new KisToolMoveSelection(); 
        Q_CHECK_PTR(t);
        t->setup(ac); 
        return t; 
    }
    virtual KisID id() { return KisID("moveselection", i18n("Move Selection Tool")); }
};



#endif // KIS_TOOL_MOVE_H_

