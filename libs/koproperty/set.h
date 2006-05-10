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

#ifndef KPROPERTY_SET_H
#define KPROPERTY_SET_H

#include "koproperty_global.h"
#include <qobject.h>
#include <q3asciidict.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <QByteArray>
#include <QStringList>
#include <QMap>
namespace KoProperty {

class Property;
class SetPrivate;

typedef QMap<QByteArray, Q3ValueList<QByteArray> > StringListMap ;
typedef QMapIterator<QByteArray, QStringList> StringListMapIterator;

/*! \brief Lists holding properties in groups

   \author Cedric Pasteur <cedric.pasteur@free.fr>
   \author Alexander Dymo <cloudtemple@mskat.net>
   \author Jaroslaw Staniek <js@iidea.pl>
 */
class KOPROPERTY_EXPORT Set : public QObject
{
	Q_OBJECT

	public:
		/*! \brief A class to iterate over a SetPrivate
		It behaves as a QDictIterator. To use it:
		\code  Set::Iterator it(set);
		       for(; it.current(); ++it) { .... }
		\endcode
		  \author Cedric Pasteur <cedric.pasteur@free.fr>
		  \author Alexander Dymo <cloudtemple@mskat.net> */
		class KOPROPERTY_EXPORT Iterator {
			public:
				Iterator(const Set &set);
				~Iterator();

				void operator ++();
				Property*  operator *();

				QByteArray  currentKey();
				Property*  current();

			private:
				Q3AsciiDictIterator<Property> *iterator;
				friend class Set;
		};

		Set(QObject *parent=0, const QString &typeName=QString::null);

		/*! Constreucts a deep copy of \a set. */
		Set(const Set& set);

		~Set();

		/*! Adds the property to the set, in the group. You can use any group name, except "common"
		  (which is already used for basic group). */
		void addProperty(Property *property, QByteArray group = "common");

		/*! Removes property from the set. Emits aboutToDeleteProperty before removing.*/
		void removeProperty(Property *property);

		/*! Removes property with the given name from the set.
		Emits aboutToDeleteProperty() before removing.*/
		void removeProperty(const QByteArray &name);

		/*! Removes all properties from the property set and destroys them. */
		virtual void clear();

		/*! Returns the number of items in the set. */
		uint count() const;

		/*! Returns TRUE if the set is empty, i.e. count() == 0; otherwise it returns FALSE. */
		bool isEmpty() const;

		/*! \return true if the set is read-only. 
		 In read-only property set, 
		 no property can be modified regardless of read-only flag of any 
		 property (see Property::isReadOnly()). On the other hand, if Property::isReadOnly()
		 is true of a property and Set::isReadOnly() is false, the property is still read-only.
		 Read-only property set prevents editing in the property editor. 
		 By default the set is read-write. */
		bool isReadOnly() const;

		/*! Sets this set to be read-only. 
		 @see isReadOnly */
		void setReadOnly(bool readOnly);

		/*! \return true if the set contains property names \a name. */
		bool contains(const QByteArray &name);

		/*! \return property named with \a name. If no such property is found,
		 null property (Property::null) is returned. */
		Property&  property( const QByteArray &name);

		/*! Accesses a property by it's name. 
		Property reference is returned, so all property modifications are allowed.
		If there is no such property, null property is returned, 
		so it's good practice to use contains() is you're unsure if the property exists. 
		For example, to set a value of a property, use:
		/code
		Set set;
		...
		if (!set.contains("myProperty")) {
			dosomething;
		}
		set["myProperty"].setValue("My Value");
		/endcode
		\return \ref Property with given name. */
		Property&  operator[](const QByteArray &name);

		/*! Creates a deep copy of \a set and assigns it to this property set. */
		const Set& operator= (const Set &set);

		/*! Change the value of property whose key is \a property to \a value.
		By default, it only calls Property::setValue(). */
		void changeProperty(const QByteArray &property, const QVariant &value);

		/*! Sets the i18n'ed string that will be shown in Editor to represent this group. */
		void setGroupDescription(const QByteArray &group, const QString desc);

		QString groupDescription(const QByteArray &group);

		const StringListMap& groups();

		/*! Used by property editor to preserve previous selection when this set is assigned again. */
		QByteArray prevSelection() const;

		void setPrevSelection(const QByteArray& prevSelection);

		/*! A name of this property set type, that is usable when
		 we want to know if two property set objects have the same type.
		 This avoids e.g. reloading of all Editor's contents.
		 Also, this allows to know if two property set objects are compatible
		 by their property sets.
		 For comparing purposes, type names are case insensitive.*/
		QString typeName() const;

		void debug();

	protected:
		/*! Constructs a set which owns or does not own it's properties.*/
		Set(bool propertyOwner);

		/*! Adds property to a group.*/
		void addToGroup(const QByteArray &group, Property *property);

		/*! Removes property from a group.*/
		void removeFromGroup(Property *property);

	signals:
		/*! Emitted when the value of the property is changed.*/
		void propertyChanged(KoProperty::Set& set, KoProperty::Property& property);

		/*! Emitted when the value of the property is reset.*/
		void propertyReset(KoProperty::Set& set, KoProperty::Property& property);

		/*! Emitted when property is about to be deleted.*/
		void aboutToDeleteProperty(KoProperty::Set& set, KoProperty::Property& property);

		/*! Emitted when property set object is about to be cleared (using clear()). 
		 This signal is also emmited from destructor before emitting aboutToBeDeleted(). */
		void aboutToBeCleared();

		/*! Emitted when property set object is about to be deleted.*/
		void aboutToBeDeleted();

	protected:
		SetPrivate *d;

	friend class Iterator;
	friend class Property;
	friend class Buffer;
};

/*! \brief
	\todo find a better name to show it's a set that doesn't own property
   \author Cedric Pasteur <cedric.pasteur@free.fr>
   \author Alexander Dymo <cloudtemple@mskat.net>
   \author Adam Treat <treat@kde.org>
 */
class KOPROPERTY_EXPORT Buffer : public Set
{
	Q_OBJECT

	public:
		Buffer();
		Buffer(const Set *set);

		/*! Intersects with other Set.*/
		virtual void intersect(const Set *set);

	protected slots:
		void intersectedChanged(KoProperty::Set& set, KoProperty::Property& prop);
		void intersectedReset(KoProperty::Set& set, KoProperty::Property& prop);

	private:
		void initialSet(const Set *set);
};

}

#endif
