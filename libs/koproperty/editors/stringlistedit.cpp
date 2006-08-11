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

#include "stringlistedit.h"

#include <QLineEdit>
#include <QLayout>
#include <QDialog>
#include <QPainter>
#include <QVariant>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <keditlistbox.h>
#include <kdialog.h>
#include <kstdguiitem.h>
#include <klocale.h>
#include <kdebug.h>

#include "property.h"

using namespace KoProperty;

StringListEdit::StringListEdit(Property *property, QWidget *parent)
 : Widget(property, parent)
{
	setHasBorders(false);
	QHBoxLayout *l = new QHBoxLayout(this);
	l->setMargin(0);
	l->setSpacing(0);

	m_edit = new QLineEdit(this);
//	m_edit->setLineWidth(0);
	m_edit->setReadOnly(true);
	m_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_edit->setMinimumHeight(5);
	l->addWidget(m_edit);

	m_selectButton = new QPushButton("...", this);
	m_selectButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
	l->addWidget(m_selectButton);
	setFocusWidget(m_selectButton);

	connect(m_selectButton, SIGNAL(clicked()), this, SLOT(showEditor()));
}

StringListEdit::~StringListEdit()
{}

QVariant
StringListEdit::value() const
{
	return m_list;
}

void
StringListEdit::setValue(const QVariant &value, bool emitChange)
{
	m_list = value.toStringList();
	m_edit->setText(value.toStringList().join(", "));
	if(emitChange)
		emit valueChanged(this);
}

void
StringListEdit::drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value)
{
//	p->eraseRect(r);
//	p->drawText(r, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, value.toStringList().join(", "));
	Widget::drawViewer(p, cg, r, value.toStringList().join(", "));
}

void
StringListEdit::showEditor()
{
	KDialog dialog(this->topLevelWidget() );
	dialog.setCaption( i18n("Edit List of Items") );
	dialog.setObjectName( "stringlist_dialog" );
	dialog.setButtons( KDialog::Ok|KDialog::Cancel );
	dialog.setDefaultButton( KDialog::Ok );
	dialog.setModal( false );
	dialog.showButtonSeparator( true );
	KEditListBox *edit = new KEditListBox(i18n("Contents of %1", property()->caption()), &dialog, "editlist");
	dialog.setMainWidget(edit);
	edit->insertStringList(m_list);

	if(dialog.exec() == QDialog::Accepted)
	{
		m_list = edit->items();
		m_edit->setText(m_list.join(", "));
		emit valueChanged(this);
	}
}

void
StringListEdit::setReadOnlyInternal(bool readOnly)
{
	m_selectButton->setEnabled(!readOnly);
}

#include "stringlistedit.moc"
