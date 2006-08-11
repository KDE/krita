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

#include "pointedit.h"
#include "editoritem.h"

#include <QLabel>
#include <QLayout>
#include <QPainter>
#include <QToolTip>

#include <kactivelabel.h>
#include <klocale.h>

//"[ %1, %2 ]"
#define POINTEDIT_MASK "%1,%2"

using namespace KoProperty;

PointEdit::PointEdit(Property *property, QWidget *parent)
 : Widget(property, parent)
{
	setHasBorders(false);
	m_edit = new KActiveLabel(this);
	m_edit->setFocusPolicy(Qt::NoFocus);
//	m_edit->setIndent(KPROPEDITOR_ITEM_MARGIN);
	QPalette pal = m_edit->palette();
	pal.setColor(QPalette::Window, palette().color(QPalette::Active, QPalette::Base));
	m_edit->setPalette(pal);
	m_edit->setLineWrapMode( QTextEdit::NoWrap );
//	m_edit->setBackgroundMode(Qt::PaletteBase);
//	m_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_edit->setMinimumHeight(5);
	setEditor(m_edit);
//	setFocusWidget(m_edit);
}

PointEdit::~PointEdit()
{}

QVariant
PointEdit::value() const
{
	return m_value;
}

void
PointEdit::setValue(const QVariant &value, bool emitChange)
{
	m_value = value;
	m_edit->selectAll();
	m_edit->setPlainText(QString(POINTEDIT_MASK).arg(value.toPoint().x()).arg(value.toPoint().y()));
	this->setToolTip( QString("%1, %2").arg(value.toPoint().x()).arg(value.toPoint().y()));

	if (emitChange)
		emit valueChanged(this);
}

void
PointEdit::drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value)
{
	QRect rect(r);
	rect.setBottom(r.bottom()+1);
	Widget::drawViewer(p, cg, rect, QString(POINTEDIT_MASK).arg(value.toPoint().x()).arg(value.toPoint().y()));
//	p->eraseRect(r);
//	p->drawText(r, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine,
//		QString("[ %1, %2 ]").arg(value.toPoint().x()).arg(value.toPoint().y()));
}

void
PointEdit::setReadOnlyInternal(bool readOnly)
{
	Q_UNUSED(readOnly);
}

#include "pointedit.moc"
