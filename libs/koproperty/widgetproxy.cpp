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
#include "widgetproxy.h"
#include "property.h"
#include "widget.h"
#include "factory.h"

#include <QLayout>
#include <QVariant>
//Added by qt3to4:
#include <Q3HBoxLayout>

namespace KoProperty {
class WidgetProxyPrivate
{
	public:
		WidgetProxyPrivate()
		: property(0), widget(0), type(Invalid), layout(0)
		{}
		~WidgetProxyPrivate() {}

		Property  *property;
		Widget  *widget;
		PropertyType  type;

		Q3HBoxLayout *layout;
};
}

using namespace KoProperty;

WidgetProxy::WidgetProxy(QWidget *parent, const char *name)
 : QWidget(parent, name)
{
	d = new WidgetProxyPrivate();
	d->property = new Property();
	d->layout = new Q3HBoxLayout(this, 0, 0);
}

WidgetProxy::~WidgetProxy()
{
	delete d->property;
}

void
WidgetProxy::setPropertyType(int propertyType)
{
	d->type = propertyType;
	setWidget();
}

int
WidgetProxy::propertyType() const
{
	return d->type;
}

QVariant
WidgetProxy::value() const
{
	if (m_editor)
		return m_editor->value();
	else
		return QVariant();
}

void
WidgetProxy::setValue(const QVariant &value)
{
	if (d->widget)
		d->widget->setValue(value, false);
}

bool
WidgetProxy::setProperty(const char *name, const QVariant &value)
{
	if( strcmp(name, "value") == 0 ) {
		setPropertyType((int) value.type() );
		setValue(value);
		return true;
	}
	else
		return QWidget::setProperty(name, value);
}

QVariant
WidgetProxy::property(const char *name) const
{
	if( strcmp( name, "value") == 0 )
		return value(  );
	else
		return QWidget::property(name);
}

void
WidgetProxy::setWidget()
{
	if (d->widget)
		delete d->widget;

	p->setType(d->type);
	d->widget = Factory::getInstance()->widgetForProperty(p);

	if (d->widget) {
		d->widget->reparent(this, QPoint(0,0), true);
		d->layout->addWidget(d->widget);
	}
}

#include "widgetproxy.moc"
