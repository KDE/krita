/*
 *  kis_tool_line.h - part of Krayon
 *
 *  Copyright (c) 2000 John Califf <jcaliff@comuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
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

#ifndef KIS_TOOL_GRADIENT_H_
#define KIS_TOOL_GRADIENT_H_

#include "kis_tool_paint.h"

#include "kis_global.h"
#include "kis_types.h"
#include "kis_painter.h"

class IntegerWidget;
class KisCmbComposite;
class KisPainter;
class KisDoubleWidget;

class QLabel;
class QPoint;
class QWidget;
class QCheckBox;

class KisToolGradient : public KisToolPaint {

	Q_OBJECT
	typedef KisToolPaint super;

public:
	KisToolGradient();
	virtual ~KisToolGradient();

	virtual void setup(KActionCollection *collection);
	virtual void update(KisCanvasSubject *subject);

	virtual void buttonPress(KisButtonPressEvent *event);
	virtual void move(KisMoveEvent *event);
	virtual void buttonRelease(KisButtonReleaseEvent *event);

	virtual void paint(QPainter& gc);
	virtual void paint(QPainter& gc, const QRect& rc);

	QWidget* createOptionWidget(QWidget* parent);
	QWidget* optionWidget();

public slots:
	void slotSetOpacity(int);
	void slotSetCompositeMode(int);
	void slotSetShape(int);
	void slotSetRepeat(int);
	void slotSetReverse(bool);
	void slotSetAntiAliasThreshold(double);

private:
	void paintLine();
	void paintLine(QPainter& gc);

	KisPoint straightLine(KisPoint point);

	bool m_dragging;

	KisPoint m_startPos;
	KisPoint m_endPos;

	KisCanvasSubject *m_subject;

	QUANTUM m_opacity;
	CompositeOp m_compositeOp;

	KisPainter::enumGradientShape m_shape;
	KisPainter::enumGradientRepeat m_repeat;

	bool m_reverse;
	double m_antiAliasThreshold;

	QWidget *m_optWidget;
	QLabel *m_lbOpacity;
	IntegerWidget *m_slOpacity;
	QLabel *m_lbComposite;
	KisCmbComposite *m_cmbComposite;
	QLabel *m_lbShape;
	QLabel *m_lbRepeat;
	QCheckBox *m_ckReverse;
	QComboBox *m_cmbShape;
	QComboBox *m_cmbRepeat;
	QLabel *m_lbAntiAliasThreshold;
	KisDoubleWidget *m_slAntiAliasThreshold;
};

#endif //KIS_TOOL_GRADIENT_H_

