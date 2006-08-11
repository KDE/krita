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

#include <QVariant>
#include <QLayout>
#include <QColor>
#include <QPainter>

#include <kcolorcombo.h>

using namespace KoProperty;

ColorButton::ColorButton(Property *property, QWidget *parent)
 : Widget(property, parent)
{
	QHBoxLayout *l = new QHBoxLayout(this);
	l->setSpacing(0);
	l->setMargin(0);
	m_edit = new KColorCombo(this);
	m_edit->setFocusPolicy(Qt::NoFocus);
	connect(m_edit, SIGNAL(activated(int)), this, SLOT(slotValueChanged(int)));
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
	return m_edit->color();
}

void
ColorButton::setValue(const QVariant &value, bool emitChange)
{
	m_edit->blockSignals(true);
	m_edit->setColor(value.value<QColor>());
	m_edit->blockSignals(false);
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
ColorButton::slotValueChanged(int)
{
	emit valueChanged(this);
}


bool
ColorButton::eventFilter(QObject* watched, QEvent* e)
{
	return Widget::eventFilter(watched, e);
}

void
ColorButton::setReadOnlyInternal(bool readOnly)
{
	setVisibleFlag(!readOnly);
}

#include "coloredit.moc"
