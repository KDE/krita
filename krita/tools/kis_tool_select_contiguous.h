/*
 *  kis_tool_select_contiguous.h - part of KImageShop^WKrayon^Krita
 *
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#ifndef __KIS_TOOL_SELECT_CONTIGUOUS_H__
#define __KIS_TOOL_SELECT_CONTIGUOUS_H__

#include <qpoint.h>
#include "kis_tool.h"
#include "kis_tool_non_paint.h"

/**
 * The 'magic wand' selection tool -- in fact just 
 * a floodfill that only creates a selection.
 */
class KisToolSelectContiguous : public KisToolNonPaint {

	typedef KisToolNonPaint super;
	Q_OBJECT

public:
	KisToolSelectContiguous();
	virtual ~KisToolSelectContiguous();

	virtual void setup(KActionCollection *collection);

	virtual void clearOld();
	virtual bool willModify() const;

	virtual void buttonPress(KisButtonPressEvent *event);
	virtual void move(KisMoveEvent *event);
	virtual void buttonRelease(KisButtonReleaseEvent *event);

protected:
	void drawRect(const QPoint&, const QPoint&); 

protected:
	QPoint m_dragStart;
	QPoint m_dragEnd;
	bool m_dragging;
	bool m_drawn;   
	bool m_init;
	QRect m_selectRect;

private:
	KisCanvasSubject *m_subject;
};

#endif //__KIS_TOOL_SELECT_CONTIGUOUS_H__

