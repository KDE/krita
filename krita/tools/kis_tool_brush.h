/*
 *  kis_tool_brush.h - part of Krita
 *
 *  Copyright (c) 2003-2004 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_tool_paint.h"

class KisPainter;
class QPoint;


class KisToolBrush : public KisToolPaint {
	typedef KisToolPaint super;

public:
	KisToolBrush();
	virtual ~KisToolBrush();

        virtual void setup(KActionCollection *collection);

	virtual void update(KisCanvasSubject *subject);

	virtual void mousePress(QMouseEvent *e);
	virtual void mouseMove(QMouseEvent *e);
	virtual void mouseRelease(QMouseEvent *e);
	virtual void tabletEvent(QTabletEvent *e);

	virtual KDialog *options(QWidget * parent);

private:
	virtual void paintLine(const QPoint & pos1,
			       const QPoint & pos2,
			       const Q_INT32 pressure,
			       const Q_INT32 xtilt,
			       const Q_INT32 ytilt);

	virtual void initPaint(const QPoint & pos);
	virtual void endPaint();

	enumBrushMode m_mode;
	KisPainter *m_painter;

        QPoint m_dragStart;
        float m_dragDist;

	KisCanvasSubject *m_subject;
	KisImageSP m_currentImage;

};
#endif // KIS_TOOL_BRUSH_H_

