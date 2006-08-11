/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004 Alexander Dymo <cloudtemple@mskat.net>
   Copyright (C) 2006 Jaroslaw Staniek <js@iidea.pl>

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

#ifndef KPROPERTY_BOOLEDIT_H
#define KPROPERTY_BOOLEDIT_H

#include "../widget.h"
#include "combobox.h"
#include <QPixmap>
#include <QResizeEvent>
#include <QEvent>

class QToolButton;

namespace KoProperty {

class KOPROPERTY_EXPORT BoolEdit : public Widget
{
	Q_OBJECT

	public:
		BoolEdit(Property *property, QWidget *parent=0);
		virtual ~BoolEdit();

		virtual QVariant value() const;
		virtual void setValue(const QVariant &value, bool emitChange=true);

		virtual void drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value);

	protected slots:
		void  slotValueChanged(bool state);

	protected:
		virtual void setReadOnlyInternal(bool readOnly);
		void setState(bool state);
		virtual void resizeEvent(QResizeEvent *ev);
		virtual bool eventFilter(QObject* watched, QEvent* e);

	private:
		QToolButton *m_toggle;
		QPixmap m_yesIcon, m_noIcon; //!< icons for m_toggle
};

class KOPROPERTY_EXPORT ThreeStateBoolEdit : public ComboBox
{
	Q_OBJECT

	public:
		ThreeStateBoolEdit(Property *property, QWidget *parent=0);
		virtual ~ThreeStateBoolEdit();

		virtual QVariant value() const;
		virtual void setValue(const QVariant &value, bool emitChange=true);

		virtual void setProperty(Property *property);
		virtual void drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value);
		QPixmap m_yesIcon, m_noIcon; //!< icons for m_toggle
};

}

#endif
