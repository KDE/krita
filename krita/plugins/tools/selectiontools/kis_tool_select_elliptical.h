/*
 *  kis_tool_select_elliptical.h - part of Krayon^WKrita
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

#ifndef __KIS_TOOL_SELECT_ELLIPTICAL_H__
#define __KIS_TOOL_SELECT_ELLIPTICAL_H__

#include <qpoint.h>

#include "kis_point.h"
#include "kis_selection.h"
#include "kis_tool_factory.h"
#include "kis_tool_non_paint.h"

class KisSelectionOptions;

class KisToolSelectElliptical : public KisToolNonPaint {

    typedef KisToolNonPaint super;
    Q_OBJECT

public:
    KisToolSelectElliptical();
    virtual ~KisToolSelectElliptical();

    virtual void update(KisCanvasSubject *subject);

    virtual void setup(KActionCollection *collection);
    virtual Q_UINT32 priority() { return 4; }
    virtual QWidget * createOptionWidget(QWidget* parent);
        virtual QWidget* optionWidget();
    virtual enumToolType toolType() { return TOOL_SELECT; }

    virtual void paint(KisCanvasPainter& gc);
    virtual void paint(KisCanvasPainter& gc, const QRect& rc);
    virtual void buttonPress(KisButtonPressEvent *e);
    virtual void move(KisMoveEvent *e);
    virtual void buttonRelease(KisButtonReleaseEvent *e);

public slots:
    virtual void slotSetAction(int);
    virtual void activate();


private:
    void clearSelection();
    void paintOutline();
    void paintOutline(KisCanvasPainter& gc, const QRect& rc);

private:
    KisCanvasSubject *m_subject;
    KisPoint m_centerPos;
    KisPoint m_startPos;
    KisPoint m_endPos;
    bool m_selecting;
    KisSelectionOptions * m_optWidget;
    enumSelectionMode m_selectAction;
};

class KisToolSelectEllipticalFactory : public KisToolFactory {
    typedef KisToolFactory super;
public:
    KisToolSelectEllipticalFactory() : super() {};
    virtual ~KisToolSelectEllipticalFactory(){};

    virtual KisTool * createTool(KActionCollection * ac) {
        KisTool * t =  new KisToolSelectElliptical();
        Q_CHECK_PTR(t);
        t->setup(ac);
        return t;
    }
    virtual KisID id() { return KisID("ellipticalselect", i18n("Elliptical Select Tool")); }
};





#endif //__KIS_TOOL_SELECT_ELLIPTICAL_H__

