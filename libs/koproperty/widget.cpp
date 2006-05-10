/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004 Alexander Dymo <cloudtemple@mskat.net>

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

#include "widget.h"
#include "property.h"
#include "editoritem.h"
#include "editor.h"

#include <qpainter.h>
#include <QVariant>

#ifdef QT_ONLY
#include <q3listview.h>
//Added by qt3to4:
#include <QKeyEvent>
#include <QResizeEvent>
#include <QEvent>
#else
#include <k3listview.h>
#include <kdebug.h>
#endif

using namespace KoProperty;

namespace KoProperty {
class WidgetPrivate
{
	public:
		WidgetPrivate()
		: property(0)
		, editor(0)
		, leaveTheSpaceForRevertButton(false)
		, hasBorders(true)
		, readOnly(false)
		, visibleFlag(true)
		{
		}
		~WidgetPrivate() {}

		Property *property;
		QWidget *editor;
		bool leaveTheSpaceForRevertButton : 1;
		bool hasBorders : 1;
		bool readOnly : 1;
		bool visibleFlag : 1;
};
}

Widget::Widget(Property *property, QWidget *parent, const char *name)
 : QWidget(parent, name)
{
	d = new WidgetPrivate();
	d->property = property;
}

Widget::~Widget()
{
	delete d;
	d = 0;
}

Property*
Widget::property() const
{
	return d ? d->property : 0; //for sanity
}

void
Widget::setProperty(Property *property)
{
	d->property = property;
	if(property)
		setValue(property->value(), false);
	//if(property->type() == ValueFromList)
	//	setValueList(property->valueList());
}

void
Widget::drawViewer(QPainter *p, const QColorGroup &, const QRect &r, const QVariant &value)
{
	p->eraseRect(r);
	QRect rect(r);
	rect.setLeft(rect.left()+KPROPEDITOR_ITEM_MARGIN);
//	if (d->hasBorders)
//		rect.setTop(rect.top()+1); //+1 to have the same vertical position as editor
//	else
//		rect.setHeight(rect.height()-1); //don't place over listviews's border
	p->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, value.toString());
}

void
Widget::undo()
{
	if(d->property)
		d->property->resetValue();
}

bool
Widget::eventFilter(QObject*, QEvent* e)
{
	if(e->type() == QEvent::KeyPress)
	{
		QKeyEvent* ev = static_cast<QKeyEvent*>(e);
		if(ev->key() == Qt::Key_Escape)
		{
			emit rejectInput(this);
			return true;
		}
		else if((ev->key() == Qt::Key_Return) || (ev->key() == Qt::Key_Enter))
		{
			// should apply when autosync == false
			emit acceptInput(this);
			return true;
		}
		else {
			Editor *list = static_cast<KoProperty::Editor*>(parentWidget()->parentWidget());
			if (!list)
				return false; //for sanity
			return list->handleKeyPress(ev);
		}

		/* moved in Editor
		if (item) {
			if(ev->key() == Qt::Key_Up && ev->state() != Qt::ControlButton)
			{
				if(item->itemAbove())
					list->setCurrentItem(item->itemAbove());
				return true;
			}
			else if(ev->key() == Qt::Key_Down && ev->state() != Qt::ControlButton)
			{
				if(item->itemBelow())
					list->setCurrentItem(item->itemBelow());
				return true;
			}
		}*/
	}

	return false;
}

void
Widget::setFocusWidget(QWidget*focusProxy)
{
	if (focusProxy) {
		if (focusProxy->focusPolicy() != Qt::NoFocus)
			setFocusProxy(focusProxy);
		focusProxy->installEventFilter(this);
	}
	else if (this->focusProxy()) {
		this->focusProxy()->removeEventFilter(this);
		setFocusProxy(0);
	}
}

bool
Widget::leavesTheSpaceForRevertButton() const
{
	return d->leaveTheSpaceForRevertButton;
}

void
Widget::setLeavesTheSpaceForRevertButton(bool set)
{
	d->leaveTheSpaceForRevertButton = set;
}

void
Widget::setHasBorders(bool set)
{
	d->hasBorders = set;
}

bool
Widget::hasBorders() const
{
	return d->hasBorders;
}

void
Widget::setEditor(QWidget* editor)
{
	d->editor = editor;
	if (!d->editor)
		return;
	d->editor->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	d->editor->move(0,0);
}

void
Widget::resizeEvent(QResizeEvent *e)
{
	QWidget::resizeEvent(e);
	if (d->editor)
		d->editor->resize(size());
}

bool
Widget::isReadOnly() const
{
	return d->readOnly;
}

void
Widget::setReadOnly(bool readOnly)
{
	d->readOnly = readOnly;
	setReadOnlyInternal(readOnly);
}

bool 
Widget::visibleFlag() const
{
	return d->visibleFlag;
}

void
Widget::setVisibleFlag(bool visible)
{
	d->visibleFlag = visible;
}

#include "widget.moc"
