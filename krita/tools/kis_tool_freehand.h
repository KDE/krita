/*
 *  kis_tool_brush.h - part of Krita
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

#ifndef KIS_TOOL_FREEHAND_H_
#define KIS_TOOL_FREEHAND_H_

#include "kis_tool_paint.h"
#include <qthread.h>

class QPoint;
class QWidget;
class QLabel;
class KisPainter;
class IntegerWidget;
class KisCmbComposite;
class KisBrush;
class KisPoint;

class KisToolFreeHand : public KisToolPaint {
	Q_OBJECT
	typedef KisToolPaint super;

public:
	KisToolFreeHand();
	virtual ~KisToolFreeHand();

	virtual void update(KisCanvasSubject *subject);

	virtual void mousePress(QMouseEvent *e);
	virtual void mouseMove(QMouseEvent *e);
	virtual void mouseRelease(QMouseEvent *e);
	virtual void tabletEvent(QTabletEvent *e);

	virtual QWidget* createoptionWidget(QWidget* parent);
	virtual QWidget* optionWidget();

public slots:
	virtual void slotSetOpacity(int);
	virtual void slotSetCompositeMode(int);

protected:
	virtual void paintAt(const QPoint &pos,
		     const double pressure,
		     const double /*xTilt*/,
		     const double /*yTilt*/) =0;

	virtual void paintLine(const QPoint & pos1,
			       const QPoint & pos2,
			       const double pressure,
			       const double xtilt,
			       const double ytilt) =0;
	inline KisPainter * painter() { return m_painter; };
	inline KisImageSP currentImage() const { return m_currentImage; };
	virtual void initPaint(const QPoint & pos);
	virtual void endPaint();

protected:
	QPoint m_dragStart;
	float m_dragDist;


private:
	enumBrushMode m_mode;
	KisPainter *m_painter;
	QUANTUM m_opacity;
	CompositeOp m_compositeOp;

	KisImageSP m_currentImage;
	QWidget *m_optWidget;
	QLabel *m_lbOpacity;
	IntegerWidget *m_slOpacity;
	QLabel *m_lbComposite;
	KisCmbComposite *m_cmbComposite;
};
#endif // KIS_TOOL_BRUSH_H_

