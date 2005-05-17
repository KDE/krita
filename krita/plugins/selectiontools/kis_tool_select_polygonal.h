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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef __selecttoolpolygonal_h__
#define __selecttoolpolygonal_h__

#include <qpoint.h>
#include <qpointarray.h>

#include "kis_tool.h"
#include "kis_tool_non_paint.h"

#include "kis_tool_factory.h"

class KisToolSelectPolygonal : public KisToolNonPaint {

	typedef KisToolNonPaint super;
	Q_OBJECT
public:
	KisToolSelectPolygonal();
	virtual ~KisToolSelectPolygonal();

        //
        // KisCanvasObserver interface
        //

	virtual void update (KisCanvasSubject *subject);

        //
        // KisToolPaint interface
        //

	virtual void setup(KActionCollection *collection);

	virtual void buttonPress(KisButtonPressEvent *event);
	virtual void move(KisMoveEvent *event);
	virtual void buttonRelease(KisButtonReleaseEvent *event);

	QWidget* createOptionWidget(QWidget* parent);
	virtual QWidget* optionWidget();

protected:
	virtual void paint(QPainter& gc);
	virtual void paint(QPainter& gc, const QRect& rc);
	void draw(QPainter& gc);
	void draw();

protected:
	KisPoint m_dragStart;
	KisPoint m_dragEnd;

	bool m_dragging;
private:
	typedef QValueVector<KisPoint> KisPointVector;
	KisCanvasSubject *m_subject;
	KisPointVector m_points;
	QWidget * m_optWidget;
};


class KisToolSelectPolygonalFactory : public KisToolFactory {
	typedef KisToolFactory super;
public:
	KisToolSelectPolygonalFactory(KActionCollection * ac) : super(ac) {};
	virtual ~KisToolSelectPolygonalFactory(){};

	virtual KisTool * createTool() { 
		KisTool * t =  new KisToolSelectPolygonal(); 
		Q_CHECK_PTR(t);
		t -> setup(m_ac); 
		return t; 
	}
	virtual KisID id() { return KisID("polygonalselect", i18n("Polygonal select tool")); }
};


#endif //__selecttoolpolygonal_h__

