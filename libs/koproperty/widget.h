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

#ifndef KPROPERTY_PROPERTYWIDGET_H
#define KPROPERTY_PROPERTYWIDGET_H

#include <QWidget>
//Added by qt3to4:
#include <QResizeEvent>
#include <QEvent>
#include "koproperty_global.h"

namespace KoProperty {

class WidgetPrivate;
class Property;

/*! \brief The base class for all item editors used in Editor.
  \author Cedric Pasteur <cedric.pasteur@free.fr>
   \author Alexander Dymo <cloudtemple@mskat.net>
*/
class KOPROPERTY_EXPORT Widget : public QWidget
{
	Q_OBJECT

	public:
		Widget(Property *property, QWidget *parent);
		virtual ~Widget();

		/*! \return the value currently entered in the item editor widget.*/
		virtual QVariant value() const = 0;

		/*! Sets the value shown in the item editor widget. Set emitChange to false
		if you don't want to emit propertyChanged signal.*/
		virtual void setValue(const QVariant &value, bool emitChange=true) = 0;

		/*! \return edited property. */
		virtual Property* property() const;

		/*! Sets the name of edited property.*/
		virtual void setProperty(Property *property);

		/*! Function to draw a property viewer when the item editor isn't shown.*/
		virtual void drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value);

		/*! Reverts the property value to previous setting.*/
		virtual void undo();

		/*! Sets the widget that will receive focus when the Widget is selected. */
		void setFocusWidget(QWidget*focusProxy);

		//! \sa d->leaveTheSpaceForRevertButton description
		bool leavesTheSpaceForRevertButton() const;

		/*! \return true if this editor has borders. 
		 Editors with borders have slightly larger height and width set by property editor widget. */
		bool hasBorders() const;

		/*! \return true if the widget is read-only. 
		 Read-only property widget does not allow to change its property value.
		 The flag is inherited from the underlying property and property set.
		 Editor::setValue() method will still work, however.
		 @see Set::isReadOnly(). */
		bool isReadOnly() const;

		/*! Sets this widget to be read-only. 
		 Disables or enables editing in the appropriate widget(s).
		 @see isReadOnly() */
		void setReadOnly(bool readOnly);

		/*! @internal
		 This flag is checked by Editor when the widget is about to show. */
		bool visibleFlag() const;

	signals:
		void valueChanged(Widget *widget);
		void acceptInput(Widget *widget);
		void rejectInput(Widget *widget);

	protected:
		void setEditor(QWidget* editor);

		/*! Filters some event for main widget, eg Enter or Esc key presses. */
		virtual bool eventFilter(QObject* watched, QEvent* e);

		virtual void resizeEvent(QResizeEvent *e);

		void setLeavesTheSpaceForRevertButton(bool set);
		void setHasBorders(bool set);

		/*! Called by setReadOnly(bool).
		 For implementation: for read-only you should disable editing in the appropriate widget(s). */
		virtual void setReadOnlyInternal(bool readOnly) = 0;

		/*! Used only in setReadOnlyInternal() to make the widget visible or invisible.
		 This flag is checked by Editor when the widget is about to show. 
		 By default widgets are visible. */
		void setVisibleFlag(bool visible);

	protected:
		WidgetPrivate  *d;
};

}

#endif
