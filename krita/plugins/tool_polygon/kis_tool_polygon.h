/*
 *  kis_tool_polygon.h - part of Krita
 *
 *  Copyright (c) 2004 Michael Thaler <michael Thaler@physik.tu-muenchen.de>
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

#ifndef KIS_TOOL_POLYGON_H_
#define KIS_TOOL_POLYGON_H_

#include <qpoint.h>
#include <qvaluevector.h>

#include "kis_tool.h"
#include "kis_tool_rectangle.h"

class KisCanvas;
class KisDoc;
class KisPainter;
class KisView;
class KisRect;


class KisToolPolygon : public KisToolPaint {

	typedef KisToolPaint super;
	Q_OBJECT

public:
	KisToolPolygon();
	virtual ~KisToolPolygon();

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

protected:
	virtual void draw(const KisPoint& start, const KisPoint& stop);
	//virtual void draw(KisPainter *gc, const QRect& rc);

protected:
	int m_lineThickness;

	KisPoint m_dragStart;
	KisPoint m_dragEnd;
	QRect m_final_lines;

	bool m_dragging;
	KisImageSP m_currentImage;
private:
        typedef QValueVector<KisPoint> KisPointVector;
        KisPointVector * m_points;
        bool m_polyLineStarted;
};


#include "kis_tool_factory.h"

class KisToolPolygonFactory : public KisToolFactory {
	typedef KisToolFactory super;
public:
	KisToolPolygonFactory(KActionCollection * ac) : super(ac) {};
	virtual ~KisToolPolygonFactory(){};
	
	virtual KisTool * createTool() { KisTool * t =  new KisToolPolygon(); t -> setup(m_ac); return t; }
	virtual QString name() { return i18n("Polygon tool"); }
};


#endif //__KIS_TOOL_POLYGON_H__