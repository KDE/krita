/*
 *  kis_tool_eraser.h - part of Krita
 *
 *  Copyright (c) 1999 Matthias Elter
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

#if !defined KIS_TOOL_ERASER_H_
#define KIS_TOOL_ERASER_H_

#include "kis_tool_paint.h"

class KisPainter;
class QPoint;


class KisToolEraser : public KisToolPaint {
	typedef KisToolPaint super;

public:
	KisToolEraser();
	virtual ~KisToolEraser();
  
	virtual void setup(KActionCollection *collection);

	virtual void update(KisCanvasSubject *subject);

	virtual void mousePress(QMouseEvent *e); 
	virtual void mouseMove(QMouseEvent *e);
	virtual void mouseRelease(QMouseEvent *e);
	virtual void tabletEvent(QTabletEvent *e);

protected:

	virtual void initErase(const QPoint & pos);
	virtual void erase(const QPoint & pos);
	virtual void endErase();

	enumBrushMode m_mode;
	KisPainter *m_painter;

        QPoint m_dragStart;
        float m_dragDist;

	KisCanvasSubject *m_subject;
	KisImageSP m_currentImage;
};

#endif // KIS_TOOL_ERASER_H_

