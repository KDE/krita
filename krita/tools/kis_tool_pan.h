/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
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

#ifndef KIS_TOOL_PAN_H_
#define KIS_TOOL_PAN_H_

#include "kis_tool_non_paint.h"

class KisCanvasSubject;
class KisPoint;

class KisToolPan : public KisToolNonPaint {

	typedef KisToolNonPaint super;
	Q_OBJECT

public:
	KisToolPan();
	virtual ~KisToolPan();

	virtual void update(KisCanvasSubject *subject);

	virtual void setup(KActionCollection *collection);
	virtual void buttonPress(KisButtonPressEvent *e);
	virtual void move(KisMoveEvent *e);
	virtual void buttonRelease(KisButtonReleaseEvent *e);

private:
	KisCanvasSubject *m_subject;
	KisPoint m_dragPos;
	Q_INT32 m_origScrollX;
	Q_INT32 m_origScrollY;
	bool m_dragging;
	QCursor m_openHandCursor;
	QCursor m_closedHandCursor;
};

#endif // KIS_TOOL_PAN_H_

