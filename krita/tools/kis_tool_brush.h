/*
 *  Copyright (c) 2003 Boudewijn Rempt <boud@valdyas.org>
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

#if !defined KIS_TOOL_BRUSH_H_
#define KIS_TOOL_BRUSH_H_

#include <qpoint.h>

#include <kcommand.h>

#include "kis_painter.h"
#include "kis_tool.h"
#include "kis_tool_paint.h"

#define noSPOTTYLINE
#define noSLOWLINE
#define noTRACERLINE
#define PERICOLINE

class KisToolBrush : public KisToolPaint {
	typedef KisToolPaint super;

public:
	KisToolBrush();
	virtual ~KisToolBrush();

        virtual void setup(KActionCollection *collection);

	virtual void update(KisCanvasSubject *subject);
#if defined TRACERLINE	
	virtual void paint(QPainter& gc);
	virtual void paint(QPainter& gc, const QRect& rc);
#endif
	virtual void mousePress(QMouseEvent *e);
	virtual void mouseMove(QMouseEvent *e);
	virtual void mouseRelease(QMouseEvent *e);
	virtual void tabletEvent(QTabletEvent *e);


private:

	virtual void paint(const QPoint & pos,
			   const Q_INT32 pressure,
			   const Q_INT32 xtilt,
			   const Q_INT32 ytilt);

	virtual void initPaint();
	virtual void endPaint();
#if defined TRACERLINE
	void paintLine(Q_INT32 s = 0);
	void paintLine(QPainter& gc, const QRect& rc, Q_INT32 s = 0);
	QPoint translateImageXYtoViewPort(const QPoint& p);
#endif

	enumBrushMode m_mode;
	KisPainter *m_painter;

	KisLayerSP m_dab;

	QPoint m_hotSpot;
        Q_INT32 m_hotSpotX;
        Q_INT32 m_hotSpotY;

        Q_INT32 m_brushWidth;
        Q_INT32 m_brushHeight;

        Q_INT32 m_spacing;


#if defined PERICOLINE
	Q_INT32 m_x1, m_y1;
#endif

	bool m_usePattern;
	bool m_useGradient;

#if defined SLOWLINE
        QPoint m_dragStart;
        float m_dragDist;
#endif

#if defined TRACERLINE
	QPointArray *m_points;
#endif
	KisCanvasSubject *m_subject;
	KisImageSP m_currentImage;
};
#endif // KIS_TOOL_BRUSH_H_

