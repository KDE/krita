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

class QWidget;
class QLabel;
class KisPainter;
class IntegerWidget;
class KisCmbComposite;
class KisBrush;
class KisPoint;
class KisEvent;


// XXX: rename this to KisToolFreehand -- Freehand is one word, so
// 'hand' must not be capitalized.
class KisToolFreeHand : public KisToolPaint {
	Q_OBJECT
	typedef KisToolPaint super;

public:
	KisToolFreeHand(const QString transactionText);
	virtual ~KisToolFreeHand();

	virtual void update(KisCanvasSubject *subject);

	virtual void buttonPress(KisButtonPressEvent *e); 
	virtual void move(KisMoveEvent *e);
	virtual void buttonRelease(KisButtonReleaseEvent *e);

	virtual QWidget* createOptionWidget(QWidget* parent);
	virtual QWidget* optionWidget();

public slots:
	virtual void slotSetOpacity(int);
	virtual void slotSetCompositeMode(int);

protected:
	virtual void paintAt(const KisPoint &pos,
			     const double pressure,
			     const double xTilt,
			     const double yTilt);

	virtual void paintLine(const KisPoint & pos1,
			       const double pressure1,
			       const double xtilt1,
			       const double ytilt1,
			       const KisPoint & pos2,
			       const double pressure2,
			       const double xtilt2,
			       const double ytilt2);

	// XXX: why not make this a protected member attribute for the
	// use of subclasses? BSAR.
	inline KisPainter * painter() { return m_painter; };
	virtual void initPaint(KisEvent *e);
	virtual void endPaint();

	KisImageSP currentImage();

protected:
	KisPoint m_prevPos;
	double m_prevPressure;
	double m_prevXTilt;
	double m_prevYTilt;
	double m_dragDist;

	QString m_transactionText;
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
#endif // KIS_TOOL_FREEHAND_H_

