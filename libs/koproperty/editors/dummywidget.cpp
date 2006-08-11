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

#include "dummywidget.h"

#include <QPainter>

using namespace KoProperty;

DummyWidget::DummyWidget(Property *property, QWidget *parent)
 : Widget(property, parent)
{}

DummyWidget::~DummyWidget()
{}

QVariant
DummyWidget::value() const
{
	return m_value;
}

void
DummyWidget::setValue(const QVariant &value, bool emitChange)
{
	m_value = value;
	if(emitChange)
		emit valueChanged(this);
}

void
DummyWidget::drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &)
{
	p->setBrush(cg.window());
	p->setPen(Qt::NoPen);
	p->drawRect(r);
}

void
DummyWidget::setReadOnlyInternal(bool readOnly)
{
	Q_UNUSED(readOnly);
}

#include "dummywidget.moc"

