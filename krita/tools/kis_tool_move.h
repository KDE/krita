/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *                1999 Michael Koch    <koch@kde.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#if !defined KIS_TOOL_MOVE_H_
#define KIS_TOOL_MOVE_H_

#include <qcursor.h>
#include <qpoint.h>
#include "kis_types.h"
#include "kis_tool.h"
#include "kis_tool_non_paint.h"

class KisDoc;
class KisView;

class KisToolMove : public KisToolNonPaint {
	Q_OBJECT
	typedef KisToolNonPaint super;

public:
	KisToolMove(KisView *view, KisDoc *doc);
	virtual ~KisToolMove();

public:
	virtual void mousePress(QMouseEvent *e);
	virtual void mouseMove(QMouseEvent *e);
	virtual void mouseRelease(QMouseEvent *e);
	virtual void keyPress(QKeyEvent *e);
	virtual void cursor(QWidget *w) const;
	virtual void setCursor(const QCursor& cursor);

public slots:
	virtual void activateSelf();

private:
	KisView *m_view;
	KisDoc *m_doc;
	QPoint m_dragStart;
	QPoint m_layerStart;
	QPoint m_layerPosition;
	QCursor m_cursor;
	bool m_dragging;
};

#endif // KIS_TOOL_MOVE_H_

