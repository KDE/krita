/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *                1999 Michael Koch    <koch@kde.org>
 *                2003 Patrick Julien  <freak@codepimps.org>
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

#ifndef KIS_TOOL_MOVE_H_
#define KIS_TOOL_MOVE_H_

#include "kis_strategy_move.h"
#include "kis_tool_non_paint.h"

class KisToolMove : public KisToolNonPaint {
	typedef KisToolNonPaint super;
	Q_OBJECT

public:
	KisToolMove();
	virtual ~KisToolMove();

public:
	virtual void update(KisCanvasSubject *subject);

public:
	virtual void setup(KActionCollection *collection);
	virtual void mousePress(QMouseEvent *e);
	virtual void mouseMove(QMouseEvent *e);
	virtual void mouseRelease(QMouseEvent *e);

private:
	KisCanvasSubject *m_subject;
	KisStrategyMove m_strategy;
};

#endif // KIS_TOOL_MOVE_H_

