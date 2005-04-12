/*
 *  kis_tool_polyline.h - part of Krita
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

#ifndef KIS_TOOL_POLYLINE_H_
#define KIS_TOOL_POLYLINE_H_

#include <qpoint.h>
#include <qvaluevector.h>

#include "kis_tool.h"
#include "kis_tool_rectangle.h"

class KisCanvas;
class KisDoc;
class KisPainter;
class KisView;
class KisRect;


class KisToolPolyline : public KisToolPaint {

	typedef KisToolPaint super;
	Q_OBJECT

public:
	KisToolPolyline();
	virtual ~KisToolPolyline();

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

class KisToolPolylineFactory : public KisToolFactory {
	typedef KisToolFactory super;
public:
	KisToolPolylineFactory(KActionCollection * ac) : super(ac) {};
	virtual ~KisToolPolylineFactory(){};
	
	virtual KisTool * createTool() { 
		KisTool * t =  new KisToolPolyline(); 
		Q_CHECK_PTR(t);
		t -> setup(m_ac); 
		return t; 
	}
	virtual KisID id() { return KisID("polyline", i18n("Polyline tool")); }
};


#endif //__KIS_TOOL_POLYLINE_H__
