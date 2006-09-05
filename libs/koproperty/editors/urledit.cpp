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

#include "urledit.h"

#include <QLayout>
#include <QVariant>
#include <QHBoxLayout>

#include <kurlrequester.h>
#include <klineedit.h>

#include "property.h"


using namespace KoProperty;

URLEdit::URLEdit(Property *property, QWidget *parent)
 : Widget(property, parent)
{
	QHBoxLayout *l = new QHBoxLayout(this);
	l->setMargin(0);
	l->setSpacing(0);

	m_edit = new KUrlRequester(this);
	m_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_edit->setMinimumHeight(5);
	l->addWidget(m_edit);

	setProperty(property);

	connect(m_edit, SIGNAL(textChanged(const QString&)), this, SLOT(slotValueChanged(const QString&)));
	m_edit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

URLEdit::~URLEdit()
{}

QVariant
URLEdit::value() const
{
	return m_edit->url();
}

void
URLEdit::setValue(const QVariant &value, bool emitChange)
{
	m_edit->blockSignals(true);
	m_edit->setUrl(value.toString());
	m_edit->blockSignals(false);
	if (emitChange)
		emit valueChanged(this);
}

void
URLEdit::slotValueChanged(const QString&)
{
	emit valueChanged(this);
}

void
URLEdit::setProperty(Property *property)
{
	if(property) {
		uint mode;
		switch(property->type()) {
			case DirectoryURL:  mode = KFile::Directory|KFile::ExistingOnly;  break;
			case FileURL: case PictureFileURL: default: mode = KFile::File|KFile::ExistingOnly;
		}
		m_edit->setMode(mode);
	}

	Widget::setProperty(property);
}

void
URLEdit::setReadOnlyInternal(bool readOnly)
{
	m_edit->lineEdit()->setReadOnly(readOnly);
	m_edit->button()->setEnabled(!readOnly);
}

#include "urledit.moc"
