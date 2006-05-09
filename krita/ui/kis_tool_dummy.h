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

#ifndef KIS_TOOL_DUMMY_H_
#define KIS_TOOL_DUMMY_H_

#include "kis_tool_non_paint.h"
#include "kis_tool_factory.h"
#include <krita_export.h>

#include "kis_point.h"
//Added by qt3to4:
#include <QLabel>

class QLabel;
class KisCanvasSubject;

/**
 * The dummy tool is activated when a layer does not permit painting
 * or any other destructive action. It shows a forbidden cursor, making
 * it clear that you really cannot do anything here.
 *
 * Furthermore, it implements more or less the same things as the pan tool,
 * so we can at least move the canvas around.
 */
class KRITAUI_EXPORT KisToolDummy : public KisToolNonPaint {

    typedef KisToolNonPaint super;
    Q_OBJECT

public:
    KisToolDummy();
    virtual ~KisToolDummy();

    virtual void update(KisCanvasSubject *subject);

    virtual void setup(KActionCollection *collection);
    virtual void buttonPress(KisButtonPressEvent *e);
    virtual void move(KisMoveEvent *e);
    virtual void buttonRelease(KisButtonReleaseEvent *e);

    virtual QWidget* createOptionWidget(QWidget* parent);
    virtual QWidget* optionWidget();

private:
    QLabel * m_optionWidget;
    KisCanvasSubject *m_subject;
    KisPoint m_dragPos;
    qint32 m_origScrollX;
    qint32 m_origScrollY;
    bool m_dragging;
};

class KisToolDummyFactory : public KisToolFactory {
    typedef KisToolFactory super;
public:
    KisToolDummyFactory() : super() {};
    virtual ~KisToolDummyFactory() {};

    virtual KisTool * createTool(KActionCollection * ac) {
        KisTool * t =  new KisToolDummy();
        Q_CHECK_PTR(t);
        t->setup(ac);
        return t;
    }
    virtual KisID id() { return KisID("dummy", i18n("Dummy Tool")); }
};


#endif // KIS_TOOL_DUMMY_H_

