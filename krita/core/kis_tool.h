/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#if !defined KIS_TOOL_H_
#define KIS_TOOL_H_

#include <qevent.h>
#include <qobject.h>
#include <qstring.h>
#include <ksharedptr.h>
#include "kis_types.h"
#include "kis_cursor.h"

class QPainter;
class QRect;
class KDialog;

class KisToolInterface : public QObject, public KShared {
	Q_OBJECT

public:
	KisToolInterface();
	virtual ~KisToolInterface();

public:
	virtual void paint(QPainter& gc) = 0;
	virtual void paint(QPainter& gc, const QRect& rc) = 0;
	virtual void clear() = 0;
	virtual void clear(const QRect& rc) = 0;

	virtual void enter(QEvent *e) = 0;
	virtual void leave(QEvent *e) = 0;
	virtual void mousePress(QMouseEvent *e) = 0;
	virtual void mouseMove(QMouseEvent *e) = 0;
	virtual void mouseRelease(QMouseEvent *e) = 0;
	virtual void keyPress(QKeyEvent *e) = 0;
	virtual void keyRelease(QKeyEvent *e) = 0;

	virtual void setCursor(const QCursor& cursor) = 0;
	virtual void cursor(QWidget *w) const = 0;
	virtual KDialog *options() = 0;
	
	virtual void save(KisToolMementoSP memento) = 0;
	virtual void restore(KisToolMementoSP memento) = 0;

public slots:
	virtual void activateSelf() = 0;

private:
	KisToolInterface(const KisToolInterface&);
	KisToolInterface& operator=(const KisToolInterface&);
};

inline
KisToolInterface::KisToolInterface() : QObject(), KShared()
{
}

inline
KisToolInterface::~KisToolInterface()
{
}

#endif // KIS_TOOL_H_

