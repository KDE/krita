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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#if !defined KIS_ZOOM_TOOL_H_
#define KIS_ZOOM_TOOL_H_

#include "kis_tool_non_paint.h"

class KisZoomTool : public KisToolNonPaint {
	typedef KisToolNonPaint super;

public:
	KisZoomTool(KisView *view, KisDoc *doc);

public:
	virtual void mousePress(QMouseEvent *e);
	virtual void setup();

private:
	KisView *m_view;
};

#endif // KIS_ZOOM_TOOL_H_
