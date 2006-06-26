/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004 Alexander Dymo <cloudtemple@mskat.net>
   Copyright (C) 2004-2006 Jaroslaw Staniek <js@iidea.pl>

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

#include "property.h"
#include "customproperty.h"
#include "set.h"
#include "factory.h"

#ifndef QT_ONLY
#include <kdebug.h>
#endif

#include <QObject>
#include <q3ptrdict.h>
#include <q3asciidict.h>
#include <QPointer>
//Added by qt3to4:
#include <Q3ValueList>
#include <QByteArray>
#include <QStringList>
namespace KoProperty {

QT_STATIC_CONST_IMPL Property Property::null;

//! @internal
class PropertyPrivate
{
	public:
		PropertyPrivate()
		: caption(0), listData(0), changed(false), storable(true), 
		 readOnly(false), visible(true),
		 autosync(-1), custom(0), useCustomProperty(true),
		 sets(0), parent(0), children(0), relatedProperties(0),
		 sortingKey(0)
		{
		}

		inline void setCaptionForDisplaying(const QString& captionForDisplaying)
		{
			delete caption;
			if (captionForDisplaying.simplified()!=captionForDisplaying)
				caption = new QString(captionForDisplaying.simplified());
			else
				caption = 0;
			this->captionForDisplaying = captionForDisplaying;
		}

		~PropertyPrivate()
		{
			delete caption;
			caption = 0;
			delete listData;
			delete children;
			delete relatedProperties;
			delete custom;
			delete sets;
		}

	int type;
	QByteArray name;
	QString captionForDisplaying;
	QString* caption;
	QString description;
	QVariant value;
	QVariant oldValue;
	/*! The string-to-value correspondence list of the property.*/
	Property::ListData* listData;
//	QMap<QString, QVariant> *valueList;
	QString icon;

	bool changed : 1;
	bool storable : 1;
	bool readOnly : 1;
	bool visible : 1;
	int autosync;
	QMap<QByteArray, QVariant> options;

	CustomProperty *custom;
	//! Flag used to allow CustomProperty to use setValue()
	bool useCustomProperty;

	//! Used when a single set is assigned for the property
	QPointer<Set> set;
	//! Used when multiple sets are assigned for the property
	Q3PtrDict< QPointer<Set> > *sets;
//	QValueList<Set*>  sets;

	Property  *parent;
	Q3ValueList<Property*>  *children;
	//! list of properties with the same name (when intersecting buffers)
	Q3ValueList<Property*>  *relatedProperties;

	int sortingKey;
};
}

using namespace KoProperty;

/////////////////////////////////////////////////////////////////

Property::ListData::ListData(const QStringList& keys_, const QStringList& names_)
 : names(names_)
// , fixed(true)
{
	setKeysAsStringList(keys_);
}

Property::ListData::ListData(const Q3ValueList<QVariant> keys_, const QStringList& names_)
 : keys(keys_), names(names_)
// , fixed(true)
{
}

Property::ListData::ListData()
// : fixed(true)
{
}

Property::ListData::~ListData()
{
}

void Property::ListData::setKeysAsStringList(const QStringList& list)
{
	keys.clear();
	for (QStringList::ConstIterator it = list.constBegin(); it!=list.constEnd(); ++it) {
		keys.append(*it);
	}
}

QStringList Property::ListData::keysAsStringList() const
{
	QStringList result;
	for (Q3ValueList<QVariant>::ConstIterator it = keys.constBegin(); it!=keys.constEnd(); ++it) {
		result.append((*it).toString());
	}
	return result;
}

/////////////////////////////////////////////////////////////////

/*
KOPROPERTY_EXPORT QMap<QString, QVariant>
KoProperty::createValueListFromStringLists(const QStringList &keys, const QStringList &values)
{
	QMap<QString, QVariant> map;
	if(keys.count() != values.count())
		return map;

	QStringList::ConstIterator valueIt = values.begin();
	QStringList::ConstIterator endIt = keys.constEnd();
	for(QStringList::ConstIterator it = keys.begin(); it != endIt; ++it, ++valueIt)
		map.insert( *it, *valueIt);

	return map;
}
*/


Property::Property(const QByteArray &name, const QVariant &value,
	const QString &caption, const QString &description,
	int type, Property* parent)
 : d( new PropertyPrivate() )
{
	d->name = name;
	d->setCaptionForDisplaying(caption);
	d->description = description;

	if(type == Auto)
		d->type = value.type();
	else
		d->type = type;

	d->custom = FactoryManager::self()->createCustomProperty(this);

	if (parent)
		parent->addChild(this);
	setValue(value, false);
}

Property::Property(const QByteArray &name, const QStringList &keys, const QStringList &strings,
	const QVariant &value, const QString &caption, const QString &description, 
	int type, Property* parent)
 : d( new PropertyPrivate() )
{
	d->name = name;
	d->setCaptionForDisplaying(caption);
	d->description = description;
	d->type = type;
	setListData(keys, strings);

	d->custom = FactoryManager::self()->createCustomProperty(this);

	if (parent)
		parent->addChild(this);
	setValue(value, false);
}

Property::Property(const QByteArray &name, ListData* listData, 
	const QVariant &value, const QString &caption, const QString &description, 
	int type, Property* parent)
 : d( new PropertyPrivate() )
{
	d->name = name;
	d->setCaptionForDisplaying(caption);
	d->description = description;
	d->type = type;
	d->listData = listData;

	d->custom = FactoryManager::self()->createCustomProperty(this);

	if (parent)
		parent->addChild(this);
	setValue(value, false);
}

Property::Property()
 : d( new PropertyPrivate() )
{
}

Property::Property(const Property &prop)
 : d( new PropertyPrivate() )
{
	*this = prop;
}

Property::~Property()
{
	delete d;
	d = 0;
}

QByteArray
Property::name() const
{
	return d->name;
}

void
Property::setName(const QByteArray &name)
{
	d->name = name;
}

QString
Property::caption() const
{
	return d->caption ? *d->caption : d->captionForDisplaying;
}

QString
Property::captionForDisplaying() const
{
	return d->captionForDisplaying;
}

void
Property::setCaption(const QString &caption)
{
	d->setCaptionForDisplaying(caption);
}

QString
Property::description() const
{
	return d->description;
}

void
Property::setDescription(const QString &desc)
{
	d->description = desc;
}

int
Property::type() const
{
	return d->type;
}

void
Property::setType(int type)
{
	d->type = type;
}

QString
Property::icon() const
{
	return d->icon;
}

void
Property::setIcon(const QString &icon)
{
	d->icon = icon;
}

QVariant
Property::value() const
{
	if(d->custom && d->custom->handleValue())
		return d->custom->value();
	return d->value;
}

QVariant
Property::oldValue() const
{
	if(d->oldValue.isNull())
		return value();
	else
		return d->oldValue;
}

void
Property::setValue(const QVariant &value, bool rememberOldValue, bool useCustomProperty)
{
	if (d->name.isEmpty()) {
		kopropertywarn << "Property::setValue(): COULD NOT SET value to a null property" << endl;
		return;
	}
	QVariant currentValue = this->value();
	const QVariant::Type t = currentValue.type();
	const QVariant::Type newt = value.type();
//	kopropertydbg << d->name << " : setValue('" << value.toString() << "' type=" << type() << ")" << endl;
	if (t != newt && !currentValue.isNull() && !value.isNull()
		 && !( (t==QVariant::Int && newt==QVariant::UInt)
			   || (t==QVariant::UInt && newt==QVariant::Int)
			   || (t==QVariant::CString && newt==QVariant::String)
			   || (t==QVariant::String && newt==QVariant::CString)
		 )) {
		kopropertywarn << "Property::setValue(): INCOMPAT TYPES! " << currentValue.typeName() 
			<< " and " << value.typeName() << endl;
	}

	//1. Check if the value should be changed
	bool ch;
	if (t == QVariant::DateTime
		|| t == QVariant::Time) {
		//for date and datetime types: compare with strings, because there
		//can be miliseconds difference
		ch = (currentValue.toString() != value.toString());
	}
	else if (t == QVariant::String || t==QVariant::CString) {
		//property is changed for string type,
		//if one of value is empty and other isn't..
		ch = ( (currentValue.toString().isEmpty() != value.toString().isEmpty())
		//..or both are not empty and values differ
			|| (!currentValue.toString().isEmpty() && !value.toString().isEmpty() && currentValue != value) );
	}
	else
		ch = (currentValue != value);

	if (!ch)
		return;

	//2. Then change it, and store old value if necessary
	if(rememberOldValue) {
		if(!d->changed)
			d->oldValue = currentValue;
		d->changed = true;
	}
	else {
		d->oldValue = QVariant(); // clear old value
		d->changed = false;
	}
	QVariant prevValue;
	if(d->custom && useCustomProperty) {
		d->custom->setValue(value, rememberOldValue);
		prevValue = d->custom->value();
	}
	else
		prevValue = currentValue;

	if (!d->custom || !useCustomProperty || !d->custom->handleValue())
		d->value = value;

	emitPropertyChanged(); // called as last step in this method!
}

void
Property::resetValue()
{
	d->changed = false;
	setValue(oldValue(), false);
	// maybe parent  prop is also unchanged now
	if(d->parent && d->parent->value() == d->parent->oldValue())
		d->parent->d->changed = false;

	if (d->sets) {
		for (Q3PtrDictIterator< QPointer<Set> > it(*d->sets); it.current(); ++it) {
			if (it.current()) //may be destroyed in the meantime
				emit (*it.current())->propertyReset(**it.current(), *this);
		}
	}
	else if (d->set) {
		emit d->set->propertyReset(*d->set, *this);
	}
}

//const QMap<QString, QVariant>*
Property::ListData*
Property::listData() const
{
	return d->listData;
}

void
Property::setListData(ListData* list) //const QMap<QString, QVariant> &list)
{
//	if(!d->valueList)
//		d->valueList = new QMap<QString, QVariant>();
	if (list == d->listData)
		return;
	delete d->listData;
	d->listData = list;
}

void
Property::setListData(const QStringList &keys, const QStringList &names)
{
	ListData* list = new ListData(keys, names);
	setListData(list);

//	if(!d->valueList)
//		d->valueList = new QMap<QString, QVariant>();
//	*(d->valueList) = createValueListFromStringLists(keys, values);
}

////////////////////////////////////////////////////////////////

bool
Property::isNull() const
{
	return d->name.isEmpty();
}

bool
Property::isModified() const
{
	return d->changed;
}

bool
Property::isReadOnly() const
{
	return d->readOnly;
}

void
Property::setReadOnly(bool readOnly)
{
	d->readOnly = readOnly;
}

bool
Property::isVisible() const
{
	return d->visible;
}

void
Property::setVisible(bool visible)
{
	d->visible = visible;
}

int
Property::autoSync() const
{
	return d->autosync;
}

void
Property::setAutoSync(int sync)
{
	d->autosync = sync;
}

bool
Property::isStorable() const
{
	return d->storable;
}

void
Property::setStorable(bool storable)
{
	d->storable = storable;
}

void
Property::setOption(const char* name, const QVariant& val)
{
	d->options[name] = val;
}

QVariant
Property::option(const char* name) const
{
	if (d->options.contains(name))
		return d->options[name];
	return QVariant();
}

bool
Property::hasOptions() const
{
	return !d->options.isEmpty();
}

/////////////////////////////////////////////////////////////////

Property::operator bool () const
{
	return !isNull();
}

const Property&
Property::operator= (const QVariant& val)
{
	setValue(val);
	return *this;
}

const Property&
Property::operator= (const Property &property)
{
	if(&property == this)
		return *this;

	if(d->listData) {
		delete d->listData;
		d->listData = 0;
	}
	if(d->children) {
		delete d->children;
		d->children = 0;
	}
	if(d->relatedProperties) {
		delete d->relatedProperties;
		d->relatedProperties = 0;
	}
	if(d->custom) {
		delete d->custom;
		d->custom = 0;
	}

	d->name = property.d->name;
	d->setCaptionForDisplaying(property.captionForDisplaying());
	d->description = property.d->description;
	d->type = property.d->type;

	d->icon = property.d->icon;
	d->autosync = property.d->autosync;
	d->visible = property.d->visible;
	d->storable = property.d->storable;
	d->readOnly = property.d->readOnly;
	d->options = property.d->options;

	if(property.d->listData) {
		d->listData = new ListData(*property.d->listData); //QMap<QString, QVariant>(*(property.d->valueList));
	}
	if(property.d->custom) {
		d->custom = FactoryManager::self()->createCustomProperty(this);
		// updates all children value, using CustomProperty
		setValue(property.value());
	}
	else {
		d->value = property.d->value;
		if(property.d->children) {
			// no CustomProperty (should never happen), simply copy all children
			d->children = new Q3ValueList<Property*>();
			Q3ValueList<Property*>::ConstIterator endIt = property.d->children->constEnd();
			for(Q3ValueList<Property*>::ConstIterator it = property.d->children->constBegin(); it != endIt; ++it) {
				Property *child = new Property( *(*it) );
				addChild(child);
			}
		}
	}

	if(property.d->relatedProperties) {
		d->relatedProperties = new Q3ValueList<Property*>( *(property.d->relatedProperties));
	}

	// update these later because they may have been changed when creating children
	d->oldValue = property.d->oldValue;
	d->changed = property.d->changed;
	d->sortingKey = property.d->sortingKey;

	return *this;
}

bool
Property::operator ==(const Property &prop) const
{
	return ((d->name == prop.d->name) && (value() == prop.value()));
}

/////////////////////////////////////////////////////////////////

const Q3ValueList<Property*>*
Property::children() const
{
	return d->children;
}

Property*
Property::child(const QByteArray &name)
{
	Q3ValueList<Property*>::ConstIterator endIt = d->children->constEnd();
	for(Q3ValueList<Property*>::ConstIterator it = d->children->constBegin(); it != endIt; ++it) {
		if((*it)->name() == name)
			return *it;
	}
	return 0;
}

Property*
Property::parent() const
{
	return d->parent;
}

void
Property::addChild(Property *prop)
{
	if (!prop)
		return;

	if(!d->children || qFind( d->children->begin(), d->children->end(), prop) == d->children->end()) { // not in our list
		if(!d->children)
			d->children = new Q3ValueList<Property*>();
		d->children->append(prop);
		prop->setSortingKey(d->children->count());
		prop->d->parent = this;
	}
	else {
		kopropertywarn << "Property::addChild(): property \"" << name() 
			<< "\": child property \"" << prop->name() << "\" already added" << endl;
		return;
	}
}

void
Property::addSet(Set *set)
{
	if (!set)
		return;

	if (!d->set) {//simple case
		d->set = set;
		return;
	}
	if ((Set*)d->set==set)
		return;
	QPointer<Set> *pset = d->sets ? d->sets->find(set) : 0;
	if (pset && (Set*)*pset == set)
		return;
	if (!d->sets) {
		d->sets = new Q3PtrDict< QPointer<Set> >( 101 );
		d->sets->setAutoDelete(true);
	}

	d->sets->replace(set, new QPointer<Set>( set ));

//	QValueList<Set*>::iterator it = qFind( d->sets.begin(), d->sets.end(), set);
//	if(it == d->sets.end()) // not in our list
//		d->sets.append(set);
}

const Q3ValueList<Property*>*
Property::related() const
{
	return d->relatedProperties;
}

void
Property::addRelatedProperty(Property *property)
{
	if(!d->relatedProperties)
		d->relatedProperties = new Q3ValueList<Property*>();

	Q3ValueList<Property*>::iterator it = qFind( d->relatedProperties->begin(), d->relatedProperties->end(), property);
	if(it == d->relatedProperties->end()) // not in our list
		d->relatedProperties->append(property);
}

CustomProperty*
Property::customProperty() const
{
	return d->custom;
}

void
Property::setCustomProperty(CustomProperty *prop)
{
	d->custom = prop;
}

int Property::sortingKey() const
{
	return d->sortingKey;
}

void Property::setSortingKey(int key)
{
	d->sortingKey = key;
}

void Property::emitPropertyChanged()
{
	if (d->sets) {
		for (Q3PtrDictIterator< QPointer<Set> > it(*d->sets); it.current(); ++it) {
			if (it.current()) {//may be destroyed in the meantime
				emit (*it.current())->propertyChangedInternal(**it.current(), *this);
				emit (*it.current())->propertyChanged(**it.current(), *this);
			}
		}
	}
	else if (d->set) {
		//if the slot connect with that signal may call set->clear() - that's
		//the case e.g. at kexi/plugins/{macros|scripting}/* -  this Property
		//may got destroyed ( see Set::removeProperty(Property*) ) while we are
		//still on it. So, if we try to access ourself/this once the signal
		//got emitted we may end in a very hard to reproduce crash. So, the
		//emit should happen as last step in this method!
		emit d->set->propertyChangedInternal(*d->set, *this);
		emit d->set->propertyChanged(*d->set, *this);
	}
}

/////////////////////////////////////////////////////////////////

void
Property::debug()
{
	QString dbg = "Property( name='" + QString(d->name) + "' desc='" + d->description
		+ "' val=" + (value().isValid() ? value().toString() : "<INVALID>");
	if (!d->oldValue.isValid())
		dbg += (", oldVal='" + d->oldValue.toString() + "'");
	dbg += (QString(d->changed ? " " : " un") + "changed");
	dbg += (d->visible ? " visible" : " hidden");
	dbg+=" )";

	kopropertydbg << dbg << endl;
}
