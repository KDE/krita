/*
 *  kis_tool_stamp.h - part of Krayon
 *
 *  Copyright (c) 2000 John Califf
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

#ifndef KIS_TOOL_STAMP_H_
#define KIS_TOOL_STAMP_H_

#include "kis_tool_paint.h"

#include "kis_global.h"
#include "kis_types.h"

class IntegerWidget;
class KisBrush;
class KisCmbComposite;
class KisPainter;

class QLabel;
class QPoint;
class QWidget;


class KisToolStamp : public KisToolPaint {

	typedef KisToolPaint super;
	Q_OBJECT

public:
	KisToolStamp();
	virtual ~KisToolStamp();

	virtual void setup(KActionCollection *collection);
	virtual void update(KisCanvasSubject *subject);

/* 	virtual bool shouldRepaint(); */
/* 	virtual void setPattern(KisPattern *pattern); */

	virtual void mousePress(QMouseEvent *event);
	virtual void mouseMove(QMouseEvent *event);
	virtual void mouseRelease(QMouseEvent *event);
	virtual void tabletEvent(QTabletEvent *event);

/* 	void setOpacity(int opacity); */
/* 	bool stampMonochrome(QPoint pos); */
/* 	bool stampColor(QPoint pos); */
/* 	bool stampToCanvas(QPoint pos); */

protected:

	QPoint m_oldp;
	QPoint m_hotSpot;
	Q_INT32 m_hotSpotX;
	Q_INT32 m_hotSpotY;
	QSize m_patternSize;
	Q_INT32 patternWidth;
	Q_INT32 patternHeight;

	QPoint m_dragStart;
	bool m_dragging;
	float m_dragdist;
	Q_INT32 spacing;


	KisCanvasSubject *m_subject;
	KisImageSP m_currentImage;
	KisPainter *m_painter;

};

#endif //KIS_TOOL_STAMP_H_
