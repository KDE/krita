/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
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

#if !defined KIS_CANVAS_H_
#define KIS_CANVAS_H_

#include <qwidget.h>

class KisCanvas : public QWidget {
	Q_OBJECT
	typedef QWidget super;

public:
	KisCanvas(QWidget *parent = 0, const char *name = 0);
	virtual ~KisCanvas();
	void showScrollBars();
    
signals:
	void mousePressed(QMouseEvent*);
	void mouseMoved(QMouseEvent*);
	void mouseReleased(QMouseEvent*);
	void gotPaintEvent(QPaintEvent*);
	void gotEnterEvent(QEvent*);
	void gotLeaveEvent(QEvent*);
	void mouseWheelEvent(QWheelEvent*);
	void gotKeyPressEvent(QKeyEvent*);
	void gotKeyReleaseEvent(QKeyEvent*);

protected:
	virtual void paintEvent(QPaintEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void enterEvent(QEvent *event );
	virtual void leaveEvent(QEvent *event);
	virtual void wheelEvent(QWheelEvent *event);
	virtual void keyPressEvent(QKeyEvent *event);
	virtual void keyReleaseEvent(QKeyEvent *event);
};

#endif // KIS_CANVAS_H_

