/** kis_tool_select_eraser.h - part of Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_TOOL_SELECT_ERASER_
#define KIS_TOOL_SELECT_ERASER_

#include "kis_tool_paint.h"

class QPoint;
class QWidget;
class QLabel;
class KisPainter;
class KisBrush;

/**
 * The selection eraser creates a selection by painting with the current
 * eraser shape. Not sure what kind of an icon could represent this... 
 * Depends a bit on how we're going to visualize selections.
 */
class KisToolSelectEraser : public KisToolPaint {

	Q_OBJECT
	typedef KisToolPaint super;

public:
	KisToolSelectEraser();
	virtual ~KisToolSelectEraser();

        virtual void setup(KActionCollection *collection);

	virtual void update(KisCanvasSubject *subject);

	virtual void buttonPress(KisButtonPressEvent *e); 
	virtual void move(KisMoveEvent *e);
	virtual void buttonRelease(KisButtonReleaseEvent *e);

	virtual QWidget* createoptionWidget(QWidget* parent);
	virtual QWidget* optionWidget();


private:
	virtual void paintLine(const KisPoint & pos1,
			       const KisPoint & pos2,
			       const double pressure,
			       const double xtilt,
			       const double ytilt);

	virtual void initPaint(const KisPoint & pos);
	virtual void endPaint();

	enumBrushMode m_mode;

        KisPoint m_dragStart;
        double m_dragDist;

	KisCanvasSubject *m_subject;
	KisImageSP m_currentImage;

	QWidget *m_optWidget;


};

#endif // KIS_TOOL_SELECT_ERASER_

