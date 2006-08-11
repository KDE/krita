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

#include "factory.h"
#include "property.h"
#include "customproperty.h"
#include "booledit.h"
#include "combobox.h"
#include "coloredit.h"
#include "cursoredit.h"
#include "dateedit.h"
#include "datetimeedit.h"
#include "dummywidget.h"
#include "fontedit.h"
#include "linestyleedit.h"
#include "pixmapedit.h"
#include "pointedit.h"
#include "rectedit.h"
#include "sizeedit.h"
#include "sizepolicyedit.h"
#include "spinbox.h"
#include "stringlistedit.h"
#include "stringedit.h"
#include "symbolcombo.h"
#include "timeedit.h"
#include "urledit.h"

#include <kdebug.h>

#include <QHash>

static KStaticDeleter<KoProperty::FactoryManager> m_managerDeleter;
static KoProperty::FactoryManager* m_manager = 0;

namespace KoProperty {

CustomPropertyFactory::CustomPropertyFactory(QObject *parent)
 : QObject(parent)
{
}

CustomPropertyFactory::~CustomPropertyFactory()
{
}


//! @internal
class FactoryManagerPrivate
{
	public:
		FactoryManagerPrivate() {}
		~FactoryManagerPrivate() {}

		//registered widgets for property types
		QHash<int, CustomPropertyFactory*> registeredWidgets;
		QHash<int, CustomPropertyFactory*> registeredCustomProperties;
};
}

using namespace KoProperty;

FactoryManager::FactoryManager()
: QObject(0)
{
	setObjectName("KoProperty::FactoryManager");
	d = new FactoryManagerPrivate();
}

FactoryManager::~FactoryManager()
{
	delete d;
}

FactoryManager*
FactoryManager::self()
{
	if(!m_manager)
		m_managerDeleter.setObject( m_manager, new FactoryManager() );
	return m_manager;
}

///////////////////  Functions related to widgets /////////////////////////////////////

void
FactoryManager::registerFactoryForEditor(int editorType, CustomPropertyFactory *widgetFactory)
{
	if(!widgetFactory)
		return;
	if(d->registeredWidgets.contains(editorType))
		kopropertywarn << "FactoryManager::registerFactoryForEditor(): "
		"Overriding already registered custom widget type \"" << editorType << "\"" << endl;
	d->registeredWidgets.insert(editorType, widgetFactory);
}

void
FactoryManager::registerFactoryForEditors(const QList<int> &editorTypes, CustomPropertyFactory *factory)
{
	QList<int>::ConstIterator endIt = editorTypes.constEnd();
	for(QList<int>::ConstIterator it = editorTypes.constBegin(); it != endIt; ++it)
		registerFactoryForEditor(*it, factory);
}

CustomPropertyFactory *
FactoryManager::factoryForEditorType(int type)
{
	return d->registeredWidgets[type];
}

Widget*
FactoryManager::createWidgetForProperty(Property *property)
{
	if(!property)
		return 0;

	const int type = property->type();

	CustomPropertyFactory *factory = d->registeredWidgets[type];
	if (factory)
		return factory->createCustomWidget(property);

	//handle combobox-based widgets:
	if (type==Cursor)
		return new CursorEdit(property);

	if (property->listData()) {
		return new ComboBox(property);
	}

	//handle other widget types:
	switch(type)
	{
		// Default QVariant types
		case String:
		case CString:
			return new StringEdit(property);
		case Rect_X:
		case Rect_Y:
		case Rect_Width:
		case Rect_Height:
		case Point_X:
		case Point_Y:
		case Size_Width:
		case Size_Height:
		case SizePolicy_HorStretch:
		case SizePolicy_VerStretch:
		case Integer:
			return new IntEdit(property);
		case Double:
			return new DoubleEdit(property);
		case Boolean: {
			//boolean editors can optionally accept 3rd state:
			QVariant thirdState = property ? property->option("3rdState") : QVariant();
			if (thirdState.toString().isEmpty())
				return new BoolEdit(property);
			else
				return new ThreeStateBoolEdit(property);
		}
		case Date:
			return new DateEdit(property);
		case Time:
			return new TimeEdit(property);
		case DateTime:
			return new DateTimeEdit(property);
		case StringList:
			return new StringListEdit(property);
		case Color:
			return new ColorButton(property);
		case Font:
			return new FontEdit(property);
		case Pixmap:
			return new PixmapEdit(property);

		// Other default types
		case Symbol:
			return new SymbolCombo(property);
		//case FontName:
		//	return new FontCombo(property);
		case FileURL:
		case DirectoryURL:
			return new URLEdit(property);
		case LineStyle:
			return new LineStyleEdit(property);

		// Composed types
		case Size:
			return new SizeEdit(property);
		case Point:
			return new PointEdit(property);
		case Rect:
			return new RectEdit(property);
		case SizePolicy:
			return new SizePolicyEdit(property);

		case List:
		case Map:
		default:
			kopropertywarn << "No editor for property " << property->name() << " of type " << property->type() << endl;
			return new DummyWidget(property);
	}
}

///////////////////  Functions related to custom properties /////////////////////////////////////

void
FactoryManager::registerFactoryForProperty(int propertyType, CustomPropertyFactory *factory)
{
	if(!factory)
		return;
	if(d->registeredCustomProperties.contains(propertyType))
		kopropertywarn << "FactoryManager::registerFactoryForProperty(): "
		"Overriding already registered custom property type \"" << propertyType << "\"" << endl;
	
	delete d->registeredCustomProperties[ propertyType ];
	d->registeredCustomProperties.insert(propertyType, factory);
}

void
FactoryManager::registerFactoryForProperties(const QList<int> &propertyTypes, 
	CustomPropertyFactory *factory)
{
	QList<int>::ConstIterator endIt = propertyTypes.constEnd();
	for(QList<int>::ConstIterator it = propertyTypes.constBegin(); it != endIt; ++it)
		registerFactoryForProperty(*it, factory);
}

CustomProperty*
FactoryManager::createCustomProperty(Property *parent)
{
	const int type = parent->type();
	CustomPropertyFactory *factory = d->registeredWidgets[type];
	if (factory)
		return factory->createCustomProperty(parent);

	switch(type) {
		case Size: case Size_Width: case Size_Height:
			return new SizeCustomProperty(parent);
		case Point: case Point_X: case Point_Y:
			return new PointCustomProperty(parent);
		case Rect: case Rect_X: case Rect_Y: case Rect_Width: case Rect_Height:
			return new RectCustomProperty(parent);
		case SizePolicy: case SizePolicy_HorStretch: case SizePolicy_VerStretch:
		case SizePolicy_HorData: case SizePolicy_VerData:
			return new SizePolicyCustomProperty(parent);
		default:
			return 0;
	}
}

