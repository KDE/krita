/*
 *  kis_tool_select_freehand.h - part of Krayon^WKrita
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

#ifndef __selecttoolfreehand_h__
#define __selecttoolfreehand_h__

#include <QPoint>
#include <q3pointarray.h>

#include "KoPoint.h"
#include "kis_tool_non_paint.h"
#include "KoToolFactory.h"
#include "kis_selection.h"

class KisSelectionOptions;

class KisToolSelectOutline : public KisToolNonPaint {

    typedef KisToolNonPaint super;
    Q_OBJECT
public:
    KisToolSelectOutline();
    virtual ~KisToolSelectOutline();

    virtual void setup(KActionCollection *collection);
    virtual quint32 priority() { return 6; }
    virtual enumToolType toolType() { return TOOL_SELECT; }

    virtual void buttonPress(KoPointerEvent *event);
    virtual void move(KoPointerEvent *event);
    virtual void buttonRelease(KoPointerEvent *event);

    QWidget* createOptionWidget(QWidget* parent);
    virtual QWidget* optionWidget();

public slots:
    virtual void slotSetAction(int);
    virtual void activate();
    void deactivate();

protected:
    virtual void paint(QPainter& gc);
    virtual void paint(QPainter& gc, const QRect& rc);
    void draw(QPainter& gc);
    void draw();


protected:
    KoPoint m_dragStart;
    KoPoint m_dragEnd;

    bool m_dragging;
private:
    typedef Q3ValueVector<KoPoint> KoPointVector;
    KisCanvasSubject *m_subject;
    KoPointVector m_points;
    KisSelectionOptions * m_optWidget;
    enumSelectionMode m_selectAction;
};


class KisToolSelectOutlineFactory : public KoToolFactory {
    typedef KoToolFactory super;
public:
    KisToolSelectOutlineFactory(QObject *parent, const QStringList&)
        : KoToolFactory(parent, "KisToolSelectOutline", i18n("Outline Selection"))
        {
            setToolTip( i18n( "Select an area by its outline" ) );
            setToolType( TOOL_TYPE_SELECTED );
            setIcon( "tool_outline_selection" );
            setPriority( 0 );
        }

    virtual ~KisToolSelectOutlineFactory(){}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return new KisToolSelectOutline(canvas);
    }

};


#endif //__selecttoolfreehand_h__

