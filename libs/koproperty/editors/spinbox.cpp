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

#include "spinbox.h"

#include "property.h"

#include <QLayout>
#include <qobject.h>
#include <QVariant>
#include <qpainter.h>
//Added by qt3to4:
#include <QKeyEvent>
#include <QEvent>

#include <kglobal.h>
#include <klocale.h>

#ifdef QT_ONLY
//! \todo
#else
#include <QLineEdit>
#endif

using namespace KoProperty;

IntSpinBox::IntSpinBox(int lower, int upper, int step, int value, int base, IntEdit *parent, const char *name)
: KIntSpinBox(lower, upper, step, value, parent, base)
{
	setObjectName(name);
	lineEdit()->setAlignment(Qt::AlignLeft);
	installEventFilter(lineEdit());
	installEventFilter(this);
	QObjectList spinwidgets = queryList( "QAbstractSpinBox", 0, false, true );
	QAbstractSpinBox* spin = static_cast<QAbstractSpinBox*>(spinwidgets.first());
	if (spin)
		spin->installEventFilter(this);
}

bool
IntSpinBox::eventFilter(QObject *o, QEvent *e)
{
	if(o == lineEdit())
	{
		if(e->type() == QEvent::KeyPress)
		{
			QKeyEvent* ev = static_cast<QKeyEvent*>(e);
			if((ev->key()==Qt::Key_Up || ev->key()==Qt::Key_Down) && ev->state() !=Qt::ControlModifier)
			{
				parentWidget()->eventFilter(o, e);
				return true;
			}
		}
	}
	if ((o == lineEdit() || o == this || o->parent() == this)
		&& e->type() == QEvent::Wheel && static_cast<IntEdit*>(parentWidget())->isReadOnly())
	{
		return true; //avoid value changes for read-only widget
	}

	return KIntSpinBox::eventFilter(o, e);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IntEdit::IntEdit(Property *property, QWidget *parent, const char *name)
 : Widget(property, parent, name)
{
	QVariant minVal( property ? property->option("min") : 0 );
	QVariant maxVal( property ? property->option("max") : QVariant() );
	QVariant minValueText( property ? property->option("minValueText") : QVariant() );
	if (minVal.isNull())
		minVal = 0;
	if (maxVal.isNull())
		maxVal = INT_MAX;

	m_edit = new IntSpinBox(minVal.toInt(), maxVal.toInt(), 1, 0, 10, this);
	if (!minValueText.isNull())
		m_edit->setSpecialValueText(minValueText.toString());
	m_edit->setMinimumHeight(5);
	setEditor(m_edit);

	setLeavesTheSpaceForRevertButton(true);
	setFocusWidget(m_edit);
	connect(m_edit, SIGNAL(valueChanged(int)), this, SLOT(slotValueChanged(int)));
}

IntEdit::~IntEdit()
{}

QVariant
IntEdit::value() const
{
	//return m_edit->cleanText().toInt();  adymo: why cleanText()
	return m_edit->value();
}

void
IntEdit::setValue(const QVariant &value, bool emitChange)
{
	m_edit->blockSignals(true);
	m_edit->setValue(value.toInt());
	updateSpinWidgets();
	m_edit->blockSignals(false);
	if (emitChange)
		emit valueChanged(this);
}

void
IntEdit::drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value)
{
	QString valueText = value.toString();
	if (property() && property()->hasOptions()) {
		//replace min value with minValueText if defined
		QVariant minValue( property()->option("min") );
		QVariant minValueText( property()->option("minValueText") );
		if (!minValue.isNull() && !minValueText.isNull() && minValue.toInt() == value.toInt()) {
			valueText = minValueText.toString();
		}
	}

	Widget::drawViewer(p, cg, r, valueText);
//	p->eraseRect(r);
//	p->drawText(r, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, valueText);
}

void
IntEdit::slotValueChanged(int)
{
	emit valueChanged(this);
}

void
IntEdit::updateSpinWidgets()
{
	QObjectList spinwidgets = queryList( "QAbstractSpinBox", 0, false, true );
	QAbstractSpinBox* spin = static_cast<QAbstractSpinBox*>(spinwidgets.first());
	if (spin) {
		spin->setReadOnly(isReadOnly());
	}
}

void
IntEdit::setReadOnlyInternal(bool readOnly)
{
	//disable editor and spin widget
	m_edit->lineEdit()->setReadOnly(readOnly);
	updateSpinWidgets();
	if (readOnly)
		setLeavesTheSpaceForRevertButton(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DoubleSpinBox::DoubleSpinBox (double lower, double upper, double step, double value, int precision, DoubleEdit *parent)
: KDoubleSpinBox(lower, upper, step, value, parent, precision)
{
	lineEdit()->setAlignment(Qt::AlignLeft);
	installEventFilter(lineEdit());
	installEventFilter(this);
	QObjectList spinwidgets = queryList( "QAbstractSpinBox", 0, false, true );
	QAbstractSpinBox* spin = static_cast<QAbstractSpinBox*>(spinwidgets.first());
	if (spin)
		spin->installEventFilter(this);
}

bool
DoubleSpinBox::eventFilter(QObject *o, QEvent *e)
{
	if(o == lineEdit())
	{
		if(e->type() == QEvent::KeyPress)
		{
			QKeyEvent* ev = static_cast<QKeyEvent*>(e);
			if((ev->key()==Qt::Key_Up || ev->key()==Qt::Key_Down) && ev->state()!=Qt::ControlButton)
			{
				parentWidget()->eventFilter(o, e);
				return true;
			}
		}
	}
	if ((o == lineEdit() || o == this || o->parent() == this)
		&& e->type() == QEvent::Wheel && static_cast<IntEdit*>(parentWidget())->isReadOnly())
	{
		return true; //avoid value changes for read-only widget
	}

	return KDoubleSpinBox::eventFilter(o, e);
}


void DoubleSpinBox::setValue ( double value )
{
	if (static_cast<IntEdit*>(parentWidget())->isReadOnly())
		return;
	KDoubleSpinBox::setValue(value);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DoubleEdit::DoubleEdit(Property *property, QWidget *parent, const char *name)
 : Widget(property, parent, name)
{
	QVariant minVal( property ? property->option("min") : 0 );
	QVariant maxVal( property ? property->option("max") : QVariant() );
	QVariant step( property ? property->option("step") : QVariant());
	QVariant precision( property ? property->option("precision") : QVariant());
	QVariant minValueText( property ? property->option("minValueText") : QVariant() );
	if (minVal.isNull())
		minVal = 0;
	if (maxVal.isNull())
		maxVal = (double)(INT_MAX/100);
	if(step.isNull())
		step = 0.1;
	if(precision.isNull())
		precision = 2;

	m_edit = new DoubleSpinBox(minVal.toDouble(), maxVal.toDouble(), step.toDouble(),
		 0, precision.toInt(), this);
	if (!minValueText.isNull())
		m_edit->setSpecialValueText(minValueText.toString());
	m_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_edit->setMinimumHeight(5);
	setEditor(m_edit);

	setLeavesTheSpaceForRevertButton(true);
	setFocusWidget(m_edit);
	connect(m_edit, SIGNAL(valueChanged(double)), this, SLOT(slotValueChanged(double)));
}

DoubleEdit::~DoubleEdit()
{}

QVariant
DoubleEdit::value() const
{
	//return m_edit->cleanText().toInt();  adymo: why cleanText()
	return m_edit->value();
}

void
DoubleEdit::setValue(const QVariant &value, bool emitChange)
{
	m_edit->blockSignals(true);
	m_edit->setValue(value.toDouble());
	updateSpinWidgets();
	m_edit->blockSignals(false);
	if (emitChange)
		emit valueChanged(this);
}

void
DoubleEdit::drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value)
{
	QString valueText;
	if (property() && property()->hasOptions()) {
		//replace min value with minValueText if defined
		QVariant minValue( property()->option("min") );
		QVariant minValueText( property()->option("minValueText") );
		if (!minValue.isNull() && !minValueText.isNull() && minValue.toString().toDouble() == value.toString().toDouble()) {
			valueText = minValueText.toString();
		}
	}
	if (valueText.isEmpty())
		valueText = QString(value.toString()).replace('.', KGlobal::locale()->decimalSymbol());

	Widget::drawViewer(p, cg, r, valueText);
//	p->eraseRect(r);
//	p->drawText(r, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, valueText);
}

void
DoubleEdit::slotValueChanged(double)
{
	emit valueChanged(this);
}

void
DoubleEdit::updateSpinWidgets()
{
	QObjectList spinwidgets = queryList( "QAbstractSpinBox", 0, false, true );
	QAbstractSpinBox* spin = static_cast<QAbstractSpinBox*>(spinwidgets.first());
	if (spin) {
		spin->setReadOnly(isReadOnly());
	}
}

void
DoubleEdit::setReadOnlyInternal(bool readOnly)
{
	//disable editor and spin widget
	m_edit->lineEdit()->setReadOnly(readOnly);
	updateSpinWidgets();
	if (readOnly)
		setLeavesTheSpaceForRevertButton(false);
}

#include "spinbox.moc"
