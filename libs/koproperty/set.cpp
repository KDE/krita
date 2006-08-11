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
#include "utils.h"

#include <qapplication.h>
#include <q3asciidict.h>
#include <QByteArray>

#include <kdebug.h>
#include <klocale.h>

typedef QMap<QByteArray, QList<QByteArray> > StringListMap;
typedef QMapIterator<QByteArray, QStringList> StringListMapIterator;

namespace KoProperty {

//! @internal
static Property Set_nonConstNull;

//! @internal
class SetPrivate
{
	public:
		SetPrivate() :
			dict(101, false), 
			readOnly(false),
			informAboutClearing(0)
		{}
		~SetPrivate(){}

	//dict of properties in form name: property
	Property::Dict dict;
//	PropertyList properties;
	//groups of properties:
	// list of group name: (list of property names)
	StringListMap propertiesOfGroup;
	QList<QByteArray>  groupNames;
	QMap<QByteArray, QString>  groupDescriptions;
	QMap<QByteArray, QString>  groupIcons;
	// map of property: group
	QMap<Property*, QByteArray> groupForProperty;

	bool ownProperty : 1;
	bool readOnly : 1;
//	static Property nonConstNull;
	QByteArray prevSelection;
	QString typeName;

	//! Used in Set::informAboutClearing(Property*) to declare that the property wants 
	//! to be informed that the set has been cleared (all properties are deleted)
	bool* informAboutClearing;

	inline KoProperty::Property& property(const QByteArray &name) const
	{
		KoProperty::Property *p = dict.find(name);
		if (p)
			return *p;
		Set_nonConstNull.setName(0); //to ensure returned property is null
		kopropertywarn << "Set::property(): PROPERTY \"" << name << "\" NOT FOUND" << endl;
		return Set_nonConstNull;
	}
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
	delete iterator;
}

void
Set::Iterator::operator ++()
{
	++(*iterator);
}

Property*
Set::Iterator::operator *() const
{
	return current();
}

QByteArray
Set::Iterator::currentKey() const
{
	if (iterator)
		return iterator->currentKey();

	return QByteArray();
}

Property*
Set::Iterator::current() const
{
	if(iterator)
		return iterator->current();

	return 0;
}

//////////////////////////////////////////////

Set::Set(QObject *parent, const QString &typeName)
: QObject(parent)
{
	setObjectName(typeName.toLatin1());

	d = new SetPrivate();
	d->ownProperty = true;
	d->groupDescriptions.insert("common", i18nc("General properties", "General"));
	d->typeName = typeName;
}


Set::Set(const Set &set)
 : QObject(0 /* implicit sharing the parent is dangerous */)
{
	setObjectName(set.objectName());
	d = new SetPrivate();
	*this = set;
}

Set::Set(bool propertyOwner)
 : QObject(0)
{
	d = new SetPrivate();
	d->ownProperty = propertyOwner;
	d->groupDescriptions.insert("common", i18nc("General properties", "General"));
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
Set::addPropertyInternal(Property *property, QByteArray group, bool updateSortingKey)
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

	Property *p = d->dict.find(property->name());
	if(p) {
		p->addRelatedProperty(property);
	}
	else {
		d->dict.insert(property->name(), property);
		addToGroup(group, property);
	}

	property->addSet(this);
	if (updateSortingKey)
		property->setSortingKey( d->dict.count() );
}

void
Set::addProperty(Property *property, QByteArray group)
{
	addPropertyInternal(property, group, true);
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

	Property *p = d->dict.find(name);
	removeProperty(p);
}

void
Set::clear()
{
	if (d->informAboutClearing)
		*d->informAboutClearing = true;
	d->informAboutClearing = 0;
	aboutToBeCleared();
	d->propertiesOfGroup.clear();
	d->groupNames.clear();
	d->groupForProperty.clear();
	d->groupDescriptions.clear();
	d->groupIcons.clear();
	Property::DictIterator it(d->dict);
	while (it.current())
		removeProperty( it.current() );
}

void
Set::informAboutClearing(bool& cleared)
{
	cleared = false;
	d->informAboutClearing = &cleared;
}

/////////////////////////////////////////////////////

void
Set::addToGroup(const QByteArray &group, Property *property)
{
	if(!property)
		return;

	//do not add the same property to the group twice
	if(d->groupForProperty.contains(property) && (d->groupForProperty[property] == group))
		return;

	if(!d->propertiesOfGroup.contains(group)) { // group doesn't exist
		QList<QByteArray> l;
		l.append(property->name());
		d->propertiesOfGroup.insert(group, l);
		d->groupNames.append(group);
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
	QMap<Property*, QByteArray>::ConstIterator it = d->groupForProperty.find(property);
	if (it==d->groupForProperty.constEnd())
		return;
	QByteArray group = *it;
	QList<QByteArray> propertiesOfGroup = d->propertiesOfGroup[group];
	propertiesOfGroup.removeAt( propertiesOfGroup.indexOf(property->name()) );
	if (propertiesOfGroup.isEmpty()) {
		//remove group as well
		d->propertiesOfGroup.remove( group );
		const int i = d->groupNames.indexOf(group);
		if (i!=-1)
			d->groupNames.removeAt(i);
	}
	d->groupForProperty.remove(property);
}

const QList<QByteArray>&
Set::groupNames() const
{
	return d->groupNames;
}

const QList<QByteArray>&
Set::propertyNamesForGroup(const QByteArray &group) const
{
	return d->propertiesOfGroup[group];
}

void
Set::setGroupDescription(const QByteArray &group, const QString desc)
{
	d->groupDescriptions[group] = desc;
}

QString
Set::groupDescription(const QByteArray &group) const
{
	if(d->groupDescriptions.contains(group))
		return d->groupDescriptions[group];
	return group;
}

void
Set::setGroupIcon(const QByteArray &group, const QString& icon)
{
	d->groupIcons[group] = icon;
}

QString
Set::groupIcon(const QByteArray &group) const
{
	return d->groupIcons[group];
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
Set::contains(const QByteArray &name) const
{
	return d->dict.find(name);
}

Property&
Set::property(const QByteArray &name) const
{
	return d->property(name);
}

Property&
Set::operator[](const QByteArray &name) const
{
	return d->property(name);
}

const Set&
Set::operator= (const Set &set)
{
	if(&set == this)
		return *this;

	clear();

	d->ownProperty = set.d->ownProperty;
	d->prevSelection = set.d->prevSelection;
	d->groupDescriptions = set.d->groupDescriptions;

	// Copy all properties in the list
	for(Property::DictIterator it(set.d->dict); it.current(); ++it) {
		Property *prop = new Property( *it.current() );
		addPropertyInternal(prop, set.d->groupForProperty[ it.current() ], 
			false /*!updateSortingKey, because the key is already set in Property copy ctor.*/ );
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
		QString groupDesc = set->d->groupDescriptions[ group ];
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

	const QList<Property*> *props = prop.related();
	QList<Property*>::ConstIterator it = props->begin();
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

	const QList<Property*> *props = prop.related();
	QList<Property*>::ConstIterator it = props->begin();
	for ( ; it != props->end(); ++it )  {
		( *it )->setValue( prop.value(), false );
	}
}

//////////////////////////////////////////////

QMap<QByteArray, QVariant> KoProperty::propertyValues(const Set& set)
{
	QMap<QByteArray, QVariant> result;
	for (Set::Iterator it(set); it.current(); ++it)
		result.insert( it.currentKey(), it.current()->value() );
	
	return result;
}

#include "set.moc"
