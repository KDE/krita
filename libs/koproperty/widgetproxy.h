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

#ifndef KPROPERTY_PROPERTYWIDGETPROXY_H
#define KPROPERTY_PROPERTYWIDGETPROXY_H

#include <qwidget.h>
#include "koproperty_global.h"

class QVariant;

namespace KoProperty {

class WidgetProxyPrivate;

/*! \brief
  \author Cedric Pasteur <cedric.pasteur@free.fr>
   \author Alexander Dymo <cloudtemple@mskat.net>
*/
class KOPROPERTY_EXPORT WidgetProxy : public QWidget
{
	Q_OBJECT

	public:
		WidgetProxy(QWidget *parent, const char *name=0);
		WidgetProxy();

		void setPropertyType(int propertyType);
		int propertyType() const;

		QVariant value() const;
		void setValue(const QVariant &value);

		virtual bool setProperty( const char *name, const QVariant &value);
		virtual QVariant property( const char *name) const;

	protected:
		void setWidget();

	private:
		WidgetProxyPrivate    *d;
};

}

#endif
