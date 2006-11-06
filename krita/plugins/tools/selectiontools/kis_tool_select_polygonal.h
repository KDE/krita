/*
 *  kis_tool_select_polygonal.h - part of Krayon^WKrita
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

#ifndef __selecttoolpolygonal_h__
#define __selecttoolpolygonal_h__

#include "KoPoint.h"
#include "kis_tool_non_paint.h"
#include "KoToolFactory.h"
#include "kis_selection.h"

class KisSelectionOptions;

class KisToolSelectPolygonal : public KisToolNonPaint {

    typedef KisToolNonPaint super;
    Q_OBJECT
public:
    KisToolSelectPolygonal();
    virtual ~KisToolSelectPolygonal();

    virtual void setup(KActionCollection *collection);
    virtual quint32 priority() { return 5; }
    virtual enumToolType toolType() { return TOOL_SELECT; }
    virtual void buttonPress(KoPointerEvent *event);
    virtual void move(KoPointerEvent *event);
    virtual void buttonRelease(KoPointerEvent *event);
    virtual void doubleClick(KoPointerEvent * event);

    void finish();
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


class KisToolSelectPolygonalFactory : public KoToolFactory {

public:
    KisToolSelectPolygonalFactory(QObject *parent, const QStringList&)
        : KoToolFactory(parent, "KisToolSelectPolygonal",
            i18n( "Polygonal Selection") )
        {
            setToolTip( i18n( "Select a polygonal region" ) );
            setToolType( TOOL_TYPE_SELECTED );
            setIcon( "tool_polygonal_selection" );
            setPriority( 0 );
        }

    virtual ~KisToolSelectPolygonalFactory(){}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return =  new KisToolSelectPolygonal(canvas);
    }
};


#endif //__selecttoolpolygonal_h__

