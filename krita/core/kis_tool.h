/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2002, 2003 Patrick Julien <freak@codepimps.org>
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

#if !defined KIS_TOOL_H_
#define KIS_TOOL_H_

#include <qobject.h>
#include "kis_canvas_observer.h"

class QCursor;
class QEvent;
class QKeyEvent;
class QPainter;
class QRect;
class QWidget;
class KActionCollection;
class KRadioAction;
class KDialog;
class KoColor;
class KisBrush;
class KisGradient;
class KisPattern;
class KisButtonPressEvent;
class KisButtonReleaseEvent;
class KisMoveEvent;

class KisTool : public QObject, public KisCanvasObserver {
	Q_OBJECT

public:
	KisTool();
	virtual ~KisTool();

public:
	virtual void paint(QPainter& gc) = 0;
	virtual void paint(QPainter& gc, const QRect& rc) = 0;
	virtual void clear() = 0;
	virtual void clear(const QRect& rc) = 0;

	virtual void setup(KActionCollection *collection) = 0;

	virtual void enter(QEvent *e) = 0;
	virtual void leave(QEvent *e) = 0;
	virtual void buttonPress(KisButtonPressEvent *e) = 0;
	virtual void move(KisMoveEvent *e) = 0;
	virtual void buttonRelease(KisButtonReleaseEvent *e) = 0;
	virtual void keyPress(QKeyEvent *e) = 0;
	virtual void keyRelease(QKeyEvent *e) = 0;

	virtual void setCursor(const QCursor& cursor) = 0;
	virtual void cursor(QWidget *w) const = 0;
	virtual QWidget* createoptionWidget(QWidget* parent) = 0;
	virtual QWidget* optionWidget() = 0;
	KRadioAction *action() const { return m_action; }

private:
	KisTool(const KisTool&);
	KisTool& operator=(const KisTool&);

protected:
	KRadioAction *m_action;
	bool m_ownAction;
};

#endif // KIS_TOOL_H_

