/*
 * This file is part of Krita
 *
 * Copyright (c) 1999 Matthias Elter (me@kde.org)
 * Copyright (c) 2001-2002 Igor Jansen (rm@kde.org)
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <qpushbutton.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qcolor.h>
#include <qdrawutil.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qspinbox.h>
#include <qstyle.h>
#include <qtooltip.h>

#include <klocale.h>
#include <knuminput.h>
#include <koFrameButton.h>

#include <kis_canvas_subject.h>
#include <kis_colorspace_registry.h>
#include <kis_color.h>
#include <kis_color_cup.h>



KisColorCup::KisColorCup(QWidget * parent, const char * name)
	: QPushButton(parent, name)
{
	m_color = Qt::black;
	connect(this, SIGNAL(clicked()), this, SLOT(slotClicked()));
}

KisColorCup::KisColorCup(const QColor & c, QWidget * parent, const char * name)
	: QPushButton(parent, name)
{
	m_color = c;
	connect(this, SIGNAL(clicked()), this, SLOT(slotClicked()));
}

void KisColorCup::slotClicked()
{
	emit changed(m_color);
}

QSize KisColorCup::sizeHint() const
{
	return style().sizeFromContents(QStyle::CT_PushButton, this, QSize(24, 24)).
			expandedTo(QApplication::globalStrut());
}

void KisColorCup::drawButtonLabel( QPainter *painter )
{
	int x, y, w, h;
	QRect r = style().subRect( QStyle::SR_PushButtonContents, this );
	r.rect(&x, &y, &w, &h);

	int margin = 2; //style().pixelMetric( QStyle::PM_ButtonMargin, this );
	x += margin;
	y += margin;
	w -= 2*margin;
	h -= 2*margin;

	if (isOn() || isDown()) {
		x += style().pixelMetric( QStyle::PM_ButtonShiftHorizontal, this );
		y += style().pixelMetric( QStyle::PM_ButtonShiftVertical, this );
	}

	qDrawShadePanel( painter, x, y, w, h, colorGroup(), true, 1, NULL);
	if ( m_color.isValid() )
		painter->fillRect( x+1, y+1, w-2, h-2, m_color );

	if ( hasFocus() ) {
		QRect focusRect = style().subRect( QStyle::SR_PushButtonFocusRect, this );
		style().drawPrimitive( QStyle::PE_FocusRect, painter, focusRect, colorGroup() );
	}

}

#include "kis_color_cup.moc"
