/*
 *  Copyright (c) 1999 Matthias Elter
 *  Copyright (c) 2002 Patrick Julien
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
#if !defined KIS_TOOL_COLOR_PICKER_H_
#define KIS_TOOL_COLOR_PICKER_H_

#include <qcursor.h>
#include "kis_tool.h"
#include "kis_tool_non_paint.h"

class KisDoc;
class KisView;

class KisToolColorPicker : public KisToolNonPaint {
	typedef KisToolNonPaint super;

public:
	KisToolColorPicker(KisView *view, KisDoc *doc);
	virtual ~KisToolColorPicker();
  
	virtual void mousePress(QMouseEvent *e);

	virtual void setCursor(const QCursor& cursor);
	virtual void cursor(QWidget *w) const;

private:
	QCursor m_cursor;
	KisView *m_view;
};

#endif // KIS_TOOL_COLOR_PICKER_H_

