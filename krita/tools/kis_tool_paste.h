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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#if !defined KIS_TOOL_PASTE_H_
#define KIS_TOOL_PASTE_H_

#include "kis_selection.h"
#include "kis_tool_non_paint.h"
#include "kis_tool_move.h"
#include "kis_strategy_move.h"

class KisToolPaste : public KisToolNonPaint, private KisStrategyMove {
	typedef KisToolNonPaint super;

public:
	KisToolPaste(KisView *view, KisDoc *doc);
	virtual ~KisToolPaste();

public:
	virtual void activate();
	virtual void clear();
	virtual void enter(QEvent *e);
	virtual void leave(QEvent *e);
	virtual void mousePress(QMouseEvent *e);
	virtual void mouseMove(QMouseEvent *e);
	virtual void mouseRelease(QMouseEvent *e);

private:
	KisView *m_view;
	KisDoc *m_doc;
	KisSelectionSP m_clip;
	bool m_justEntered;
	QUANTUM m_oldOpacity;
	KisSelectionSP m_selection;
};

#endif // KIS_TOOL_PASTE_H_

