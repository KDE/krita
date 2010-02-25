/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004 Alexander Dymo <cloudtemple@mskat.net>
   Copyright (C) 2004-2009 Jarosław Staniek <staniek@kde.org>

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

#include <QtCore/QObject>
#include <QtCore/QHash>
#include "Property.h"

namespace KoProperty
{

class Property;
class SetPrivate;

/*! \brief Set of properties

   \author Cedric Pasteur <cedric.pasteur@free.fr>
   \author Alexander Dymo <cloudtemple@mskat.net>
   \author Jarosław Staniek <staniek@kde.org>
 */
class KOPROPERTY_EXPORT Set : public QObject
{
    Q_OBJECT

public:
    //! Ordering options for properties
    /*! @see Set::Iterator::setOrder() */
    enum Order {
        InsertionOrder,    //!< insertion order
        AlphabeticalOrder, //!< alphabetical order (case-insensitively by captions)
        AlphabeticalByName //!< alphabetical order (case-insensitively by name)
    };

    //! An interface for functor selecting properties.
    /*! Used in Iterator. */
    class KOPROPERTY_EXPORT PropertySelector
    {
    public:
        PropertySelector();
        virtual ~PropertySelector();
        
        //! An operator implementing the functor.
        virtual bool operator()(const Property& prop) const = 0;

        //! Creates a deep copy of the selector. 
        //! Required for proper usage of the selector.
        virtual PropertySelector* clone() const = 0;
    };

    //! A class to iterate over a Set.
    /*! It behaves like a QList::ConstIterator.
     Usage:
     @code  for (Set::Iterator it(set); it.current(); ++it) { .... }
     @endcode
     Usage with selector:
     @code  for (Set::Iterator it(set, MySelector()); it.current(); ++it) { .... }
     @endcode */
    class KOPROPERTY_EXPORT Iterator
    {
    public:
        //! Creates iterator for @a set set of properties.
        /*!             The properties are sorted by insertion order by default.
            Use setOrder(Iterator::Alphabetical) to have alphabetical order. */
        Iterator(const Set &set);

        //! Creates iterator for @a set set of properties.
        /*! @a selector functor is used to iterate only 
            over specified properties. 
            The properties are sorted by insertion order by default.
            Use setOrder(Iterator::Alphabetical) to have alphabetical order. */
        Iterator(const Set &set, const PropertySelector& selector);

        ~Iterator();

        //! Sets order for properties. Restarts the iterator.
        void setOrder(Set::Order order);

        //! @return order for properties.
        Set::Order order() const { return m_order; }

        void operator ++();
        Property* operator *() const {
            return current();
        }
        Property* current() const {
            return m_iterator==m_end ? 0 : *m_iterator;
        }

        friend class Set;
    private:
        const Set *m_set;
        QList<Property*>::ConstIterator m_iterator;
        QList<Property*>::ConstIterator m_end;
        PropertySelector *m_selector;
        Set::Order m_order;
        QList<Property*> m_sorted; //!< for sorted order
    };

    //! Constructs a new Set object.
    //! @see typeName()
    explicit Set(QObject *parent = 0, const QString &typeName = QString());

    /*! Constructs a deep copy of \a set.
     The new object will not have a QObject parent even if \a set has such parent. */
    explicit Set(const Set& set);

    virtual ~Set();

    /*! Adds the property to the set, in the group. You can use any group name, except "common"
      (which is already used for basic group). */
    void addProperty(Property *property, QByteArray group = "common");

    /*! Removes property from the set. Emits aboutToDeleteProperty before removing.*/
    void removeProperty(Property *property);

    /*! Removes property with the given name from the set.
    Emits aboutToDeleteProperty() before removing.*/
    void removeProperty(const QByteArray &name);

    /*! Removes all properties from the property set and destroys them. */
    void clear();

    /*! @return the number of top-level properties in the set. */
    uint count() const;

    /*! @return the number of top-level properties in the set 
                matching criteria defined by @a selector. */
    uint count(const PropertySelector& selector) const;

    /*! @return true if the set is empty, i.e. count() is 0; otherwise returns false. */
    bool isEmpty() const;

    /*! @return true if the set is contains visible properties. */
    bool hasVisibleProperties() const;

    /*! @return true if the set is contains properties
                matching criteria defined by @a selector. */
    bool hasProperties(const PropertySelector& selector) const;

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
    bool contains(const QByteArray &name) const;

    /*! \return property named with \a name. If no such property is found,
     null property (Property::null) is returned. */
    Property& property(const QByteArray &name) const;

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
    @return \ref Property with given name. 
    @see changeProperty(const QByteArray &, const QVariant &)
    @see changePropertyIfExists(const QByteArray &, const QVariant &)
    */
    Property& operator[](const QByteArray &name) const;

    /*! @return value for property named with @a name. 
     If no such property is found, default value @a defaultValue is returned. */
    QVariant propertyValue(const QByteArray &name, const QVariant& defaultValue = QVariant()) const {
        const Property& p( property(name) );
        return p.isNull() ? defaultValue : p.value();
    }

    /*! Creates a deep copy of \a set and assigns it to this property set. */
    const Set& operator= (const Set &set);

    /*! Change the value of property whose key is \a property to \a value. 
    @see void changePropertyIfExists(const QByteArray &, const QVariant &) */
    void changeProperty(const QByteArray &property, const QVariant &value);

    /*! Change the value of property whose key is \a property to \a value
     only if it exists in the set.
     @see void changeProperty(const QByteArray &, const QVariant &) */
    void changePropertyIfExists(const QByteArray &property, const QVariant &value) {
        if (contains(property))
            changeProperty(property, value);
    }

    /*! Sets the i18n'ed string that will be shown in Editor to represent
     \a group. */
    void setGroupDescription(const QByteArray &group, const QString desc);

    /*! \return the i18n'ed description string for \a group that will
     be shown in Editor to represent \a group. If there is no special
     description set for the group, \a group is just returned. */
    QString groupDescription(const QByteArray &group) const;

    /*! Sets the icon name \a icon to be displayed for \a group. */
    void setGroupIcon(const QByteArray &group, const QString& icon);

    /*! \return the icons name for \a group. */
    QString groupIcon(const QByteArray &group) const;

    /*! \return a list of all group names. The order of items is undefined. */
    const QList<QByteArray> groupNames() const;

    /*! \return a list of all property names for group @ group. 
     The order of items is undefined. */
    const QList<QByteArray> propertyNamesForGroup(const QByteArray &group) const;

    /*! \return a list of all property names for group @ group. 
     The order of items is undefined. */
    const QHash<Property*, QByteArray> groupsNamesForProperties() const;

    /*! Used by property editor to preserve previous selection when this set
     is assigned again. */
    QByteArray previousSelection() const;

    void setPreviousSelection(const QByteArray& prevSelection);

    /*! An optional name of this property set's type, that is usable when
     we want to know if two property set objects have the same type.
     This avoids e.g. reloading of all Editor's contents.
     Also, informs whether two property set objects are compatible.
     For comparing purposes, type names are lowercase for case insensitive comparisons.*/
    QString typeName() const;

    /*! Prints debug output for this set. */
    void debug() const;

protected:
    /*! Constructs a set which owns or does not own it's properties.*/
    Set(bool propertyOwner);

    /*! @return group name for property @a property */
    QByteArray groupForProperty(Property *property) const;

    /*! Adds property to a group.*/
    void addToGroup(const QByteArray &group, Property *property);

    /*! Removes property from a group.*/
    void removeFromGroup(Property *property);

    /*! Adds the property to the set, in the group. You can use any group name, except "common"
      (which is already used for basic group). 
      @internal */
    void addPropertyInternal(Property *property, QByteArray group);

    /*! @internal used to declare that \a property wants to be informed
     that the set has been cleared (all properties are deleted) */
    void informAboutClearing(bool& cleared);

    /*! Helper for Private class. */
    void addRelatedProperty(Property *p1, Property *p2) const;

signals:
    /*! Emitted when the value of the property is changed.*/
    void propertyChanged(KoProperty::Set& set, KoProperty::Property& property);

    /*! @internal Exists to be sure that we emitted it before propertyChanged(),
     so Editor object can handle this. */
    void propertyChangedInternal(KoProperty::Set& set, KoProperty::Property& property);

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
    friend class SetPrivate;
    SetPrivate * const d;

    friend class Iterator;
    friend class Property;
    friend class Buffer;
};

//! kDebug() stream operator. Writes this set to the debug output in a nicely formatted way.
KOPROPERTY_EXPORT QDebug operator<<(QDebug dbg, const Set &set);

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
    Buffer(const KoProperty::Set& set);

    /*! Intersects with other Set.*/
    virtual void intersect(const KoProperty::Set& set);

protected slots:
    void intersectedChanged(KoProperty::Set& set, KoProperty::Property& prop);
    void intersectedReset(KoProperty::Set& set, KoProperty::Property& prop);

private:
    void init(const KoProperty::Set& set);
};

//! @return property values for set @a set
KOPROPERTY_EXPORT QHash<QByteArray, QVariant> propertyValues(const Set& set);

}

#endif
