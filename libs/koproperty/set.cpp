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

#include "set.h"
#include "property.h"

#include <q3asciidict.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <QByteArray>
//#include <Q3ValueList>

#ifdef QT_ONLY
// \todo
#else
#include <kdebug.h>
#include <klocale.h>
#endif

namespace KoProperty {

//! @internal
static Property Set_nonConstNull;

//! @internal
class SetPrivate
{
	public:
		SetPrivate() : dict(101, false), readOnly(false) {}
		~SetPrivate(){}

	//dict of properties in form name: property
	Property::Dict dict;
//	PropertyList properties;
	//groups of properties:
	// list of group name: (list of property names)
	StringListMap propertiesOfGroup;
	QMap<QByteArray, QString>  groupsDescription;
	// map of property: group
	QMap<Property*, QByteArray> groupForProperty;

	bool ownProperty : 1;
	bool readOnly : 1;
//	static Property nonConstNull;
	QByteArray prevSelection;
	QString typeName;
/*
	bool contains(const QCString &name)
	{
		PropertyList::iterator it = properties.begin();
		for( ; it != properties.end(); ++it )
			if ( ( *it )->name() == name )
				return true;

		return false;
	}

	Property* operator[](const QCString &name)
	{
		PropertyList::iterator it = properties.begin();
		for( ; it != properties.end(); ++it )
			if ( ( *it )->name() == name )
				return ( *it );

		return 0L;
	}

	Property* take(const QCString &name)
	{
		Property *p = 0L;
		PropertyList::iterator it = properties.begin();
		for( ; it != properties.end(); ++it )
			if ( ( *it )->name() == name )
			{
				p = ( *it );
				properties.remove( it );
			}
		return p;
	}
*/
};
}

using namespace KoProperty;

//Set::Iterator class
Set::Iterator::Iterator(const Set &set)
{
	iterator = new Property::DictIterator(set.d->dict);
}

Set::Iterator::~Iterator()
{
}

void
Set::Iterator::operator ++()
{
	++(*iterator);
}

Property*
Set::Iterator::operator *()
{
	return current();
}

QByteArray
Set::Iterator::currentKey()
{
	if (iterator)
		return iterator->currentKey();

	return QByteArray();
}

Property*
Set::Iterator::current()
{
	if(iterator)
		return iterator->current();

	return 0;
}

 //////////////////////////////////////////////

Set::Set(QObject *parent, const QString &typeName)
: QObject(parent, typeName.latin1())
{
	d = new SetPrivate();
	d->ownProperty = true;
	d->groupsDescription.insert("common", i18nc("General properties", "General"));
	d->typeName = typeName;
}


Set::Set(const Set &l)
 : QObject(l.parent(), l.name())
{
	d = new SetPrivate();
	*this = l;
}

Set::Set(bool propertyOwner)
 : QObject(0, 0)
{
	d = new SetPrivate();
	d->ownProperty = propertyOwner;
	d->groupsDescription.insert("common", i18nc("General properties", "General"));
}

Set::~Set()
{
	emit aboutToBeCleared();
	emit aboutToBeDeleted();
	clear();
	delete d;
}

/////////////////////////////////////////////////////

void
Set::addProperty(Property *property, QByteArray group)
{
	if (group.isEmpty())
		group = "common";
	if (property == 0) {
		kopropertywarn << "Set::addProperty(): property == 0" << endl; 
		return;
	}
	if (property->name().isEmpty()) {
		kopropertywarn << "Set::addProperty(): COULD NOT ADD NULL PROPERTY" << endl; 
		return;
	}

	if(d->dict.find(property->name())) {
		Property *p = d->dict[property->name()];
		p->addRelatedProperty(property);
	}
	else {
		d->dict.insert(property->name(), property);
		addToGroup(group, property);
	}

	property->addSet(this);
	property->setSortingKey( d->dict.count() );
}

void
Set::removeProperty(Property *property)
{
	if(!property)
		return;

	Property *p = d->dict.take(property->name());
	removeFromGroup(p);
	if(d->ownProperty) {
		emit aboutToDeleteProperty(*this, *p);
		delete p;
	}
}

void
Set::removeProperty(const QByteArray &name)
{
	if(name.isNull())
		return;

	Property *p = d->dict.take(name);
	removeProperty(p);
}

void
Set::clear()
{
	aboutToBeCleared();
	Property::DictIterator it(d->dict);
	while (it.current())
		removeProperty( it.current() );
}

/////////////////////////////////////////////////////

void
Set::addToGroup(const QByteArray &group, Property *property)
{
	if(!property)
		return;

	//do not add same property to the group twice
	if(d->groupForProperty.contains(property) && (d->groupForProperty[property] == group))
		return;

	if(!d->propertiesOfGroup.contains(group)) { // group doesn't exist
		Q3ValueList<QByteArray> l;
		l.append(property->name());
		d->propertiesOfGroup.insert(group, l);
	}
	else {
		d->propertiesOfGroup[group].append(property->name());
	}
	d->groupForProperty.insert(property, group);
}

void
Set::removeFromGroup(Property *property)
{
	if(!property)
		return;
	QByteArray group = d->groupForProperty[property];
	d->propertiesOfGroup[group].remove(property->name());
	d->groupForProperty.remove(property);
}

const StringListMap&
Set::groups()
{
	return d->propertiesOfGroup;
}

void
Set::setGroupDescription(const QByteArray &group, const QString desc)
{
	d->groupsDescription[group] = desc;
}

QString
Set::groupDescription(const QByteArray &group)
{
	if(d->groupsDescription.contains(group))
		return d->groupsDescription[group];
	return group;
}

/////////////////////////////////////////////////////

uint
Set::count() const
{
	return d->dict.count();
}

bool
Set::isEmpty() const
{
	return d->dict.isEmpty();
}

bool
Set::isReadOnly() const
{
	return d->readOnly;
}

void
Set::setReadOnly(bool readOnly)
{
	d->readOnly = readOnly;
}

bool
Set::contains(const QByteArray &name)
{
	return d->dict.find(name);
}

Property&
Set::property(const QByteArray &name)
{
	Property *p = d->dict[name];
	if (p)
		return *p;
//		p = new Property();
//		//addProperty(p); // maybe just return a null property
//	}
	Set_nonConstNull.setName(0); //to ensure returned property is null
	kopropertywarn << "Set::property(): PROPERTY \"" << name << "\" NOT FOUND" << endl;
	return Set_nonConstNull;
}

Property&
Set::operator[](const QByteArray &name)
{
	return property(name);
}

const Set&
Set::operator= (const Set &l)
{
	if(&l == this)
		return *this;

	d->dict.clear();
	d->groupForProperty.clear();

	d->ownProperty = l.d->ownProperty;
	d->prevSelection = l.d->prevSelection;
	d->groupsDescription = l.d->groupsDescription;
	d->propertiesOfGroup = l.d->propertiesOfGroup;

	// Copy all properties in the list
	for(Property::DictIterator it(l.d->dict); it.current(); ++it) {
		Property *prop = new Property( *it.current() );
		addProperty(prop, l.d->groupForProperty[ it.current() ] );
	}

	return *this;
}

void
Set::changeProperty(const QByteArray &property, const QVariant &value)
{
	Property *p = d->dict[property];
	if(p)
		p->setValue(value);
}

/////////////////////////////////////////////////////

void
Set::debug()
{
	//kopropertydbg << "List: typeName='" << m_typeName << "'" << endl;
	if(d->dict.isEmpty()) {
		kopropertydbg << "<EMPTY>" << endl;
		return;
	}
	kopropertydbg << d->dict.count() << " properties:" << endl;

	for(Property::DictIterator it(d->dict); it.current(); ++it)
		it.current()->debug();
}

QByteArray
Set::prevSelection() const
{
	return d->prevSelection;
}

void
Set::setPrevSelection(const QByteArray &prevSelection)
{
	d->prevSelection = prevSelection;
}

QString
Set::typeName() const
{
	return d->typeName;
}

/////////////////////////////////////////////////////

Buffer::Buffer()
	:Set(false)
{
	connect( this, SIGNAL( propertyChanged( KoProperty::Set&, KoProperty::Property& ) ),
	         this, SLOT(intersectedChanged( KoProperty::Set&, KoProperty::Property& ) ) );

	connect( this, SIGNAL( propertyReset( KoProperty::Set&, KoProperty::Property& ) ),
	         this, SLOT(intersectedReset( KoProperty::Set&, KoProperty::Property& ) ) );
}

Buffer::Buffer(const Set *set)
	:Set(false)
{
	connect( this, SIGNAL( propertyChanged( KoProperty::Set&, KoProperty::Property& ) ),
	         this, SLOT(intersectedChanged( KoProperty::Set&, KoProperty::Property& ) ) );

	connect( this, SIGNAL( propertyReset( KoProperty::Set&, KoProperty::Property& ) ),
	         this, SLOT(intersectedReset( KoProperty::Set&, KoProperty::Property& ) ) );

	initialSet( set );
}

void Buffer::initialSet(const Set *set)
{
	//deep copy of set
	for(Property::DictIterator it(set->d->dict); it.current(); ++it) {
		Property *prop = new Property( *it.current() );
		QByteArray group = set->d->groupForProperty[it.current()];
		QString groupDesc = set->d->groupsDescription[ group ];
		setGroupDescription( group, groupDesc );
		addProperty( prop, group );
		prop->addRelatedProperty( it.current() );
	}
}

void Buffer::intersect(const Set *set)
{
	if ( d->dict.isEmpty() )
	{
		initialSet( set );
		return;
	}

	 for(Property::DictIterator it(d->dict); it.current(); ++it) {
		const char* key = it.current()->name();
		if ( Property *property =  set->d->dict[ key ] )
		{
				blockSignals( true );
				it.current()->resetValue();
				it.current()->addRelatedProperty( property );
				blockSignals( false );
		}
		else
			removeProperty( key );
	}
}

void Buffer::intersectedChanged(KoProperty::Set& set, KoProperty::Property& prop)
{
	Q_UNUSED(set);
	QByteArray propertyName = prop.name();
	if ( !contains( propertyName ) )
		return;

	const Q3ValueList<Property*> *props = prop.related();
	Q3ValueList<Property*>::const_iterator it = props->begin();
	for ( ; it != props->end(); ++it ) {
		( *it )->setValue( prop.value(), false );
	}
}

void Buffer::intersectedReset(KoProperty::Set& set, KoProperty::Property& prop)
{
	Q_UNUSED(set);
	QByteArray propertyName = prop.name();
	if ( !contains( propertyName ) )
		return;

	const Q3ValueList<Property*> *props = prop.related();
	Q3ValueList<Property*>::const_iterator it = props->begin();
	for ( ; it != props->end(); ++it )  {
		( *it )->setValue( prop.value(), false );
	}
}

#include "set.moc"
