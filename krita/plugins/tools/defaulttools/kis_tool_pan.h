/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
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

#ifndef KIS_TOOL_PAN_H_
#define KIS_TOOL_PAN_H_

#include "kis_point.h"
#include "kis_tool_non_paint.h"
#include "kis_tool_factory.h"
#include <koffice_export.h>

class KisCanvasSubject;
class KisPoint;

class KRITATOOL_EXPORT KisToolPan : public KisToolNonPaint
{

    typedef KisToolNonPaint super;
    Q_OBJECT

public:
    KisToolPan();
    virtual ~KisToolPan();

    virtual void update(KisCanvasSubject *subject);

    virtual void setup(KActionCollection *collection);
        virtual enumToolType toolType() { return TOOL_VIEW; }

    virtual void buttonPress(KisButtonPressEvent *e);
    virtual void move(KisMoveEvent *e);
    virtual void buttonRelease(KisButtonReleaseEvent *e);

    virtual bool wantsAutoScroll() const { return false; }

private:
    KisCanvasSubject *m_subject;
    KisPoint m_dragPos;
    Q_INT32 m_origScrollX;
    Q_INT32 m_origScrollY;
    bool m_dragging;
    QCursor m_openHandCursor;
    QCursor m_closedHandCursor;
};

class KisToolPanFactory : public KisToolFactory {
    typedef KisToolFactory super;
public:
    KisToolPanFactory() : super() {};
    virtual ~KisToolPanFactory(){};

    virtual KisTool * createTool(KActionCollection * ac) {
        KisTool * t =  new KisToolPan();
        Q_CHECK_PTR(t);
        t->setup(ac);
        return t;
    }
    virtual KisID id() { return KisID("pan", i18n("Pan Tool")); }
};


#endif // KIS_TOOL_PAN_H_

