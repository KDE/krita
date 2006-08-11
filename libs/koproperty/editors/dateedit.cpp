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

#include "dateedit.h"

#include <q3datetimeedit.h>
#include <q3rangecontrol.h>
#include <QObject>
#include <QLayout>
#include <QVariant>
#include <QPainter>
#include <QHBoxLayout>

#include <klocale.h>
#include <kglobal.h>

using namespace KoProperty;

DateEdit::DateEdit(Property *property, QWidget *parent)
 : Widget(property, parent)
{
	QHBoxLayout *l = new QHBoxLayout(this);
	l->setMargin(0);
	l->setSpacing(0);

	m_edit = new Q3DateEdit(this);
	m_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_edit->setMinimumHeight(5);
	l->addWidget(m_edit);

	setLeavesTheSpaceForRevertButton(true);

	setFocusWidget(m_edit);
	connect(m_edit, SIGNAL(valueChanged(const QDate&)), this, SLOT(slotValueChanged(const QDate&)));
}

DateEdit::~DateEdit()
{}

QVariant
DateEdit::value() const
{
	return m_edit->date();
}

void
DateEdit::setValue(const QVariant &value, bool emitChange)
{
	m_edit->blockSignals(true);
	m_edit->setDate(value.toDate());
	m_edit->blockSignals(false);
	if (emitChange)
		emit valueChanged(this);
}

void
DateEdit::drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value)
{
	p->eraseRect(r);
	Widget::drawViewer(p, cg, r, KGlobal::locale()->formatDate(value.toDate(), true /* use short format*/ ));
//	p->drawText(r, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, KGlobal::locale()->formatDate(value.toDate(), true /* use short format*/ ));
}

void
DateEdit::slotValueChanged(const QDate&)
{
	emit valueChanged(this);
}

void
DateEdit::setReadOnlyInternal(bool readOnly)
{
	setVisibleFlag(!readOnly);
}

#include "dateedit.moc"
