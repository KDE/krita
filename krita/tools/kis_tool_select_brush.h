/*
 *  kis_tool_select_brush.h - part of Krita
 *
 *  Copyright (c) 2003-2004 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_TOOL_SELECT_BRUSH_H_
#define KIS_TOOL_SELECT_BRUSH_H_

#include "kis_tool_freehand.h"

class KisPoint;

/**
 * The selection brush creates a selection by painting with the current
 * brush shape. Not sure what kind of an icon could represent this... 
 * Depends a bit on how we're going to visualize selections.
 */
class KisToolSelectBrush : public KisToolFreeHand {
	Q_OBJECT
	typedef KisToolFreeHand super;

public:
	KisToolSelectBrush();
	virtual ~KisToolSelectBrush();

	virtual void setup(KActionCollection *collection);
	virtual QWidget* createOptionWidget(QWidget* parent);
	virtual QWidget* optionWidget();

protected:

	virtual void initPaint(KisEvent *e);
	virtual void endPaint();
};
#endif // KIS_TOOL_SELECT_BRUSH_H_

