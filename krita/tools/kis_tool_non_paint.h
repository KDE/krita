/*
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

#ifndef KIS_TOOL_NON_PAINT_H_
#define KIS_TOOL_NON_PAINT_H_

#include <qcursor.h>
#include <koColor.h>

#include "kis_global.h"
#include "kis_types.h"
#include "kis_tool.h"
#include "kis_brush.h"
#include "kis_pattern.h"
#include "kis_gradient.h"

class QEvent;
class QKeyEvent;
class QPaintEvent;
class QRect;
class KDialog;
class KisCanvasSubject;


class KisToolNonPaint : public KisTool {

	Q_OBJECT
	typedef KisTool super;

public:
	KisToolNonPaint();
	virtual ~KisToolNonPaint();

public:
	virtual void update(KisCanvasSubject *subject);

public:
	virtual void paint(QPainter& gc);
	virtual void paint(QPainter& gc, const QRect& rc);
	virtual void clear();
	virtual void clear(const QRect& rc);

	virtual void enter(QEvent *e);
	virtual void leave(QEvent *e);
	virtual void buttonPress(KisButtonPressEvent *e);
	virtual void move(KisMoveEvent *e);
	virtual void buttonRelease(KisButtonReleaseEvent *e);
	virtual void keyPress(QKeyEvent *e);
	virtual void keyRelease(QKeyEvent *e);

	virtual void cursor(QWidget *w) const;
	virtual void setCursor(const QCursor& cursor);
	virtual QWidget* createoptionWidget(QWidget* parent) ;
	virtual QWidget* optionWidget();

protected slots:
	virtual void activate();

private:
	QCursor m_cursor;
	KisCanvasSubject *m_subject;
	QWidget m_optWidget;
};

#endif // KIS_TOOL_NON_PAINT_H_

