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
#include "combobox.h"

#include <qlayout.h>
#include <qmap.h>
#include <qvariant.h>
#include <qpainter.h>
//Added by qt3to4:
#include <Q3HBoxLayout>

#ifdef QT_ONLY
#iinclude <qcombobox.h>
#else
#include <kcombobox.h>
#include <kdebug.h>
#endif

#include "property.h"

using namespace KoProperty;

ComboBox::ComboBox(Property *property, QWidget *parent, const char *name)
 : Widget(property, parent, name)
 , m_setValueEnabled(true)
{
	Q3HBoxLayout *l = new Q3HBoxLayout(this, 0, 0);
#ifdef QT_ONLY
	m_edit = new QComboBox(this);
#else
	m_edit = new KComboBox(this);
#endif
	m_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_edit->setMinimumHeight(5);
	l->addWidget(m_edit);

	// If the boolean-option "editable" is true, the CombBox will
	// be editable. If that#s the case, we return a string on
	// value() and expected a string on setValue() rather then
	// the integer index of the selected item.
	QVariant v = property->option("editable");
	bool editable = ( !v.isNull() && v.canCast(QVariant::Bool) && v.toBool() );
	m_edit->setEditable(editable);
	m_edit->setInsertPolicy(QComboBox::NoInsert);
	m_edit->setMinimumSize(10, 0); // to allow the combo to be resized to a small size
	m_edit->setAutoCompletion(true);
#ifndef QT_ONLY
	m_edit->setContextMenuEnabled(false);
#endif

	if (this->property()->listData()) {
		fillBox();
	}
//not needed for combo	setLeavesTheSpaceForRevertButton(true);

	setFocusWidget(m_edit);
	if(editable)
		connect(m_edit, SIGNAL(textChanged(const QString&)), this, SLOT(slotTextChanged(const QString&)));
	else
		connect(m_edit, SIGNAL(activated(int)), this, SLOT(slotValueChanged(int)));
}

ComboBox::~ComboBox()
{
}

QVariant
ComboBox::value() const
{
	if(m_edit->editable()) {
		return QVariant( m_edit->currentText() );
	}

	if (!property()->listData()) {
		kopropertywarn << "ComboBox::value(): propery listData not available!" << endl;
		return QVariant();
	}

	const int idx = m_edit->currentItem();
	if (idx<0 || idx>=(int)property()->listData()->keys.count())
		return QVariant();
	return QVariant( property()->listData()->keys[idx] );
//	if(property()->listData() && property()->listData()->contains(m_edit->currentText()))
//		return (*(property()->valueList()))[m_edit->currentText()];
//	return QVariant();
}

void
ComboBox::setValue(const QVariant &value, bool emitChange)
{
	if (!m_setValueEnabled)
		return;

	if(m_edit->editable()) {
		m_edit->setCurrentText(value.toString());
	}
	else {
		if (!property()->listData()) {
			kopropertywarn << "ComboBox::value(): propery listData not available!" << endl;
			return;
		}

		int idx = property()->listData()->keys.findIndex( value );
		if (idx>=0 && idx<m_edit->count()) {
			m_edit->setCurrentItem(idx);
		}
		else {
			if (idx<0) {
				kopropertywarn << "ComboBox::setValue(): NO SUCH KEY '" << value.toString() 
					<< "' (property '" << property()->name() << "')" << endl;
			} else {
				QStringList list;
				for (int i=0; i<m_edit->count(); i++)
					list += m_edit->text(i);
				kopropertywarn << "ComboBox::setValue(): NO SUCH INDEX WITHIN COMBOBOX: " << idx 
					<< " count=" << m_edit->count() << " value='" << value.toString() 
					<< "' (property '" << property()->name() << "')\nActual combobox contents: "
					<< list << endl;
			}
			m_edit->setCurrentText(QString::null);
		}
	}

	if(value.isNull())
		return;

//	m_edit->blockSignals(true);
//	m_edit->setCurrentText(keyForValue(value));
//	m_edit->blockSignals(false);
	if (emitChange)
		emit valueChanged(this);
}

void
ComboBox::drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value)
{
	QString txt;
	if(m_edit->editable()) {
		txt = value.toString();
	}
	else {
		if (property()->listData()) {
			const int idx = property()->listData()->keys.findIndex( value );
			if (idx>=0)
				txt = property()->listData()->names[ idx ];
		}
	}

	Widget::drawViewer(p, cg, r, txt); //keyForValue(value));
//	p->eraseRect(r);
//	p->drawText(r, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, keyForValue(value));
}

void
ComboBox::fillBox()
{
	m_edit->clear();
	//m_edit->clearContents();

	if(!property())
		return;
	if (!property()->listData()) {
		kopropertywarn << "ComboBox::fillBox(): propery listData not available!" << endl;
		return;
	}

	m_edit->insertStringList(property()->listData()->names);
#ifndef QT_ONLY
	KCompletion *comp = m_edit->completionObject();
	comp->insertItems(property()->listData()->names);
	comp->setCompletionMode(KGlobalSettings::CompletionShell);
#endif
}

void
ComboBox::setProperty(Property *prop)
{
	const bool b = (property() == prop);
	m_setValueEnabled = false; //setValue() couldn't be called before fillBox()
	Widget::setProperty(prop);
	m_setValueEnabled = true;
	if(!b)
		fillBox();
	if(prop)
		setValue(prop->value(), false); //now the value can be set
}

void
ComboBox::slotValueChanged(int)
{
	emit valueChanged(this);
}

void
ComboBox::slotTextChanged(const QString&)
{
	emit valueChanged(this);
}

void
ComboBox::setReadOnlyInternal(bool readOnly)
{
	setVisibleFlag(!readOnly);
}


/*QString
ComboBox::keyForValue(const QVariant &value)
{
	const QMap<QString, QVariant> *list = property()->valueList();
	Property::ListData *list = property()->listData();

	if (!list)
		return QString::null;
	int idx = listData->keys.findIndex( value );


	QMap<QString, QVariant>::ConstIterator endIt = list->constEnd();
	for(QMap<QString, QVariant>::ConstIterator it = list->constBegin(); it != endIt; ++it) {
		if(it.data() == value)
			return it.key();
	}
	return QString::null;
}*/


#include "combobox.moc"

