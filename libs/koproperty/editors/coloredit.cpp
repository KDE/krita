/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004  Alexander Dymo <cloudtemple@mskat.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "coloredit.h"

#include <qvariant.h>
#include <qlayout.h>
#include <qcolor.h>
#include <qpainter.h>

#ifdef QT_ONLY
#include <qcolordialog.h>
#include <qpushbutton.h>
#include <qpixmap.h>
//Added by qt3to4:
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QEvent>
#else
#include <kcolorcombo.h>
#endif

using namespace KoProperty;

ColorButton::ColorButton(Property *property, QWidget *parent, const char *name)
 : Widget(property, parent, name)
{
	QHBoxLayout *l = new QHBoxLayout(this, 0, 0);
#ifdef QT_ONLY
	m_edit = new QPushButton(this);
	connect(m_edit, SIGNAL(clicked()), this, SLOT(selectColor()));
#else
	m_edit = new KColorCombo(this);
	m_edit->setFocusPolicy(Qt::NoFocus);
	connect(m_edit, SIGNAL(activated(int)), this, SLOT(slotValueChanged(int)));
#endif
	m_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_edit->setMinimumHeight(5);
	l->addWidget(m_edit);
	setFocusWidget(m_edit);
}

ColorButton::~ColorButton()
{}

QVariant
ColorButton::value() const
{
#ifdef QT_ONLY
	return m_color;
#else
	return m_edit->color();
#endif
}

void
ColorButton::setValue(const QVariant &value, bool emitChange)
{
#ifdef QT_ONLY
	m_color = value.toColor();
	m_edit->setText(m_color.name());
	QPixmap px;
	px.resize(14,14);
	px.fill(m_color);
	m_edit->setIconSet(px);
#else
	m_edit->blockSignals(true);
	m_edit->setColor(value.value<QColor>());
	m_edit->blockSignals(false);
#endif
	if (emitChange)
		emit valueChanged(this);
}

void
ColorButton::drawViewer(QPainter *p, const QColorGroup &, const QRect &r, const QVariant &value)
{
	p->eraseRect(r);

	p->setBrush(value.value<QColor>());
	p->setPen(Qt::SolidLine);
	QRect r2(r);
	r2.setTopLeft(r.topLeft() + QPoint(5,5));
	r2.setBottomRight(r.bottomRight() - QPoint(5,5));
	p->drawRect(r2);
}

void
ColorButton::selectColor()
{
#ifdef QT_ONLY
	m_color = QColorDialog::getColor(m_color,this);
	emit valueChanged(this);
	m_edit->setText(m_color.name());
	QPixmap px;
	px.resize(14,14);
	px.fill(m_color);
	m_edit->setIconSet(px);
#endif
}

void
ColorButton::slotValueChanged(int)
{
	emit valueChanged(this);
}


bool
ColorButton::eventFilter(QObject* watched, QEvent* e)
{
#ifdef QT_ONLY
	if(e->type() == QEvent::KeyPress) {
		QKeyEvent* ev = static_cast<QKeyEvent*>(e);
		if(ev->key() == Qt::Key_Space) {
			m_edit->animteClick();
			return true;
		}
	}
#endif
	return Widget::eventFilter(watched, e);
}

void
ColorButton::setReadOnlyInternal(bool readOnly)
{
	setVisibleFlag(!readOnly);
}

#include "coloredit.moc"
