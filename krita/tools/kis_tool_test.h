/*
 *  Copyright (c) 2003 Boudewijn Rempt <boud@valdyas.org>
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

#if !defined KIS_TOOL_TEST_H_
#define KIS_TOOL_TEST_H_

#include <qpoint.h>
#include "kis_tool.h"
#include "kis_tool_paint.h"

class KisToolTest : public KisToolPaint {
	typedef KisToolPaint super;

public:
	KisToolTest(KisView *view, KisDoc *doc);
	virtual ~KisToolTest();

public:
        virtual void setup();
	virtual void mousePress(QMouseEvent *e);
	virtual void tabletEvent(QTabletEvent *e);

private:
	KisView *m_view;
	KisDoc *m_doc;
};

#endif // KIS_TOOL_TEST_H_

