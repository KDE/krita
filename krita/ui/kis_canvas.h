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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
    
	// When enabled, the canvas may throw away move events if the application
	// is unable to keep up with them, i.e. intermediate move events in the event
	// queue are skipped.
	void enableMoveEventCompressionHint(bool enableMoveCompression) { m_enableMoveEventCompressionHint = enableMoveCompression; }

signals:
	void mousePressed(QMouseEvent*);
	void mouseMoved(QMouseEvent*);
	void mouseReleased(QMouseEvent*);
	void gotTabletEvent(QTabletEvent*);
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
        virtual void tabletEvent(QTabletEvent *event);
	virtual void enterEvent(QEvent *event );
	virtual void leaveEvent(QEvent *event);
	virtual void wheelEvent(QWheelEvent *event);
	virtual void keyPressEvent(QKeyEvent *event);
	virtual void keyReleaseEvent(QKeyEvent *event);

	bool m_enableMoveEventCompressionHint;

#ifdef Q_WS_X11
	// On X11 systems, Qt throws away mouse move events if the application
	// is unable to keep up with them. We override this behaviour so that
	// we receive all move events, so that painting follows the mouse's motion
	// accurately.
	void initX11Support();
	bool x11Event(XEvent *event);
	int translateX11ButtonState(int state);

	static bool X11SupportInitialised;

	// Modifier masks for alt/meta - detected at run-time
	static long X11AltMask;
	static long X11MetaMask;

	int m_lastRootX;
	int m_lastRootY;
#endif
};

#endif // KIS_CANVAS_H_

