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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef KIS_TOOL_COLOR_PICKER_H_
#define KIS_TOOL_COLOR_PICKER_H_

#include "kis_tool.h"
#include "kis_tool_non_paint.h"

class KisCanvasSubject;

class KisToolColorPicker : public KisToolNonPaint {
	Q_OBJECT
	typedef KisToolNonPaint super;

public:
	KisToolColorPicker();
	virtual ~KisToolColorPicker();

public:
	virtual void update(KisCanvasSubject *subject);
	virtual void setup(KActionCollection *collection);
	virtual void mousePress(QMouseEvent *e);

private:
	KisCanvasSubject *m_subject;
};

#endif // KIS_TOOL_COLOR_PICKER_H_

