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

#ifndef KPROPERTY_SPINBOX_H
#define KPROPERTY_SPINBOX_H

#include <knuminput.h>

#include "../widget.h"
//Added by qt3to4:
#include <QEvent>

namespace KoProperty {

class IntEdit;
class DoubleEdit;

// Int Editor

class IntSpinBox : public KIntSpinBox
{
	Q_OBJECT

	public:
		IntSpinBox(int lower, int upper, int step, int value, int base=10,
			IntEdit *parent=0);
		virtual ~IntSpinBox() {;}

		virtual void setValue(const QVariant &value);

		virtual bool eventFilter(QObject *o, QEvent *e);
		QLineEdit * lineEdit() const { return KIntSpinBox::lineEdit(); }
};

class KOPROPERTY_EXPORT IntEdit : public Widget
{
	Q_OBJECT

	public:
		IntEdit(Property *property, QWidget *parent=0);
		virtual ~IntEdit();

		virtual QVariant value() const;
		virtual void setValue(const QVariant &value, bool emitChange=true);
		virtual void drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value);

	protected:
		virtual void setReadOnlyInternal(bool readOnly);
		void updateSpinWidgets();

	protected slots:
		void slotValueChanged(int value);

	private:
		IntSpinBox  *m_edit;
};

// Double editor

class DoubleSpinBox : public KDoubleSpinBox
{
	Q_OBJECT

	public:
		//! \todo Support setting precision limits, step, etc.
		DoubleSpinBox(double lower, double upper, double step, double value=0,
			int precision=2, DoubleEdit *parent=0);
		virtual ~DoubleSpinBox() {;}

		virtual bool eventFilter(QObject *o, QEvent *e);
		QLineEdit * lineEdit() const { return KDoubleSpinBox::lineEdit(); }

	public slots:
		virtual void setValue( const QVariant& value );
};

class KOPROPERTY_EXPORT DoubleEdit : public Widget
{
	Q_OBJECT

	public:
		DoubleEdit(Property *property, QWidget *parent=0);
		virtual ~DoubleEdit();

		virtual QVariant value() const;
		virtual void setValue(const QVariant &value, bool emitChange=true);
		virtual void drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value);

	protected:
		virtual void setReadOnlyInternal(bool readOnly);
		void updateSpinWidgets();

	protected slots:
		void slotValueChanged(double value);

	private:
		DoubleSpinBox  *m_edit;
};

}

#endif
