/*
 *  Copyright (c) 2003 Boudewijn Rempt
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

#ifndef KIS_TOOL_PAINT_H_
#define KIS_TOOL_PAINT_H_

#include <qcursor.h>
#include "kis_tool.h"

class QEvent;
class QKeyEvent;
class QPaintEvent;
class QRect;
class KDialog;
class KisCanvasSubject;


enum enumBrushMode {
	PAINT,
	PAINT_STYLUS,
	ERASE,
	ERASE_STYLUS,
	HOVER
};

class KisToolPaint : public KisTool {

	Q_OBJECT
	typedef KisTool super;

public:
	KisToolPaint();
	virtual ~KisToolPaint();

public:
	virtual void update(KisCanvasSubject *subject);

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
	virtual QWidget* createOptionWidget(QWidget* parent);
	virtual QWidget* optionWidget();

protected:
	void notifyModified() const;
	KisCanvasSubject* canvasSubject();

protected slots:
	virtual void activate();

private:
	QCursor m_cursor;
	QCursor m_toolCursor;
protected:
	KisCanvasSubject *m_subject;
	QString m_name;
};

inline KisCanvasSubject* KisToolPaint::canvasSubject()
{
	return m_subject;
}

#endif // KIS_TOOL_PAINT_H_

