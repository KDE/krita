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

#include "Set.h"
#include "Property.h"
#include "Utils.h"

#include <QApplication>
#include <QByteArray>

#include <kdebug.h>
#include <klocale.h>

typedef QMap<QByteArray, QList<QByteArray> > ByteArrayListMap;

namespace KoProperty
{

//! @internal
class SetPrivate
{
public:
    SetPrivate(KoProperty::Set *set) :
            q(set),
            readOnly(false),
            informAboutClearing(0),
            m_visiblePropertiesCount(0)
    {}

    ~SetPrivate() {}

    inline uint visiblePropertiesCount() const { return m_visiblePropertiesCount; }

    inline Property* property(const QByteArray &name) const {
        return hash.value(name.toLower());
    }

    inline Property& propertyOrNull(const QByteArray &name) const
    {
        Property *p = property(name);
        if (p)
            return *p;
        nonConstNull.setName(0); //to ensure returned property is null
        kWarning() << "PROPERTY" << name << "NOT FOUND";
        return nonConstNull;
    }

    void addProperty(Property *property, QByteArray group/*, bool updateSortingKey*/)
    {
        if (!property) {
            kWarning() << "property == 0";
            return;
        }
        if (property->isNull()) {
            kWarning() << "COULD NOT ADD NULL PROPERTY";
            return;
        }
        if (group.isEmpty())
            group = "common";

        Property *p = this->property(property->name());
        if (p) {
            q->addRelatedProperty(p, property);
        }
        else {
            list.append(property);
            hash.insert(property->name().toLower(), property);
            if (property->isVisible()) {
                m_visiblePropertiesCount++;
            }
            q->addToGroup(group, property);
        }

        property->addSet(q);
#if 0
        if (updateSortingKey)
            property->setSortingKey(count());
#endif
    }

    void removeProperty(Property *property)
    {
        if (!property)
            return;

        if (!list.removeOne(property)) {
            kDebug() << "Set does not contain property" << property;
            return;
        }
        Property *p = hash.take(property->name());
        q->removeFromGroup(p);
        if (p->isVisible()) {
            m_visiblePropertiesCount--;
        }
        if (ownProperty) {
            emit q->aboutToDeleteProperty(*q, *p);
            delete p;
        }
    }

    void clear() {
        if (informAboutClearing)
            *informAboutClearing = true;
        informAboutClearing = 0;
        emit q->aboutToBeCleared();
        m_visiblePropertiesCount = 0;
        propertiesOfGroup.clear();
        groupNames.clear();
        groupForProperties.clear();
        groupDescriptions.clear();
        groupIcons.clear();
        qDeleteAll(list);
        list.clear();
        hash.clear();
    }

    inline int count() const { return list.count(); }

    inline bool isEmpty() const { return list.isEmpty(); }

    inline QByteArray groupForProperty(Property *property) const {
        return groupForProperties.value(property);
    }

    inline void addPropertyToGroup(Property *property, const QByteArray &groupLower) {
        groupForProperties.insert(property, groupLower);
    }

    inline void removePropertyFromGroup(Property *property) {
        groupForProperties.remove(property);
    }

    // Copy all attributes except complex ones
    void copyAttributesFrom(const SetPrivate& other)
    {
        Set *origSet = q;
        *this = other;
        q = origSet;
        // do not copy too deeply
        list.clear();
        hash.clear();
        groupForProperties.clear();
        m_visiblePropertiesCount = 0;
    }

    // Copy all properties from the other set
    void copyPropertiesFrom(
        const QList<Property*>::ConstIterator& constBegin,
        const QList<Property*>::ConstIterator& constEnd, const Set & set)
    {
        for (QList<Property*>::ConstIterator it(constBegin); it!=constEnd; ++it) {
            Property *prop = new Property(*(*it));
            addProperty(prop, set.groupForProperty( *it )
#if 0
                        ,
                        false /* don't updateSortingKey, because the key is already 
                                 set in Property copy ctor.*/
#endif
                       );
        }
    }

    inline QList<Property*>::ConstIterator listConstIterator() const {
        return list.constBegin();
    }

    inline QList<Property*>::ConstIterator listConstEnd() const {
        return list.constEnd();
    }

    Set *q;

    //groups of properties:
    // list of group name: (list of property names)
    ByteArrayListMap propertiesOfGroup;
    QList<QByteArray>  groupNames;
    QHash<QByteArray, QString>  groupDescriptions;
    QHash<QByteArray, QString>  groupIcons;
    // map of property: group

    bool ownProperty;
    bool readOnly;
    QByteArray prevSelection;
    QString typeName;

    //! Used in Set::informAboutClearing(Property*) to declare that the property wants
    //! to be informed that the set has been cleared (all properties are deleted)
    bool* informAboutClearing;

    mutable Property nonConstNull;

private:
    //! a list of properties, preserving their order, owner of Property objects
    QList<Property*> list;
    //! a hash of properties in form name -> property
    QHash<QByteArray, Property*> hash;
    QHash<Property*, QByteArray> groupForProperties;
    uint m_visiblePropertiesCount; //! cache for optimization, 
                                   //! used by @ref bool Set::hasVisibleProperties()
};

}

using namespace KoProperty;

//////////////////////////////////////////////

Set::PropertySelector::PropertySelector()
{
}

Set::PropertySelector::~PropertySelector()
{
}

//////////////////////////////////////////////

typedef QPair<Property*, QString> Iterator_PropertyAndString;

bool Iterator_propertyAndStringLessThan(
    const Iterator_PropertyAndString &n1, const Iterator_PropertyAndString &n2)
{
    return QString::compare(n1.second, n2.second, Qt::CaseInsensitive) < 0;
}

//////////////////////////////////////////////

Set::Iterator::Iterator(const Set &set)
    : m_set(&set)
    , m_iterator( set.d->listConstIterator() )
    , m_end( set.d->listConstEnd() )
    , m_selector( 0 )
    , m_order(Set::InsertionOrder)
{
}

Set::Iterator::Iterator(const Set &set, const PropertySelector& selector)
    : m_set(&set)
    , m_iterator( set.d->listConstIterator() )
    , m_end( set.d->listConstEnd() )
    , m_selector( selector.clone() )
    , m_order(Set::InsertionOrder)
{
    skipNotAcceptable();
}

Set::Iterator::~Iterator()
{
    delete m_selector;
}

void Set::Iterator::skipNotAcceptable()
{
    if (!m_selector)
        return;
    //kDebug() << "FROM:" << *current();
    if (current() && !(*m_selector)( *current() )) {
        // skip first items that not are acceptable by the selector
        ++(*this);
    }
    //kDebug() << "TO:" << *current();
}

void Set::Iterator::setOrder(Set::Order order)
{
    if (m_order == order)
        return;
    m_order = order;
    switch (m_order) {
    case Set::AlphabeticalOrder:
    case Set::AlphabeticalByName:
    {
        QList<Iterator_PropertyAndString> propertiesAndStrings;
        m_iterator = m_set->d->listConstIterator();
        m_end = m_set->d->listConstEnd();
        for (; m_iterator!=m_end; ++m_iterator) {
            Property *prop = *m_iterator;
            QString captionOrName;
            if (m_order == Set::AlphabeticalOrder) {
                captionOrName = prop->caption();
            }
            if (captionOrName.isEmpty()) {
                captionOrName = prop->name();
            }
            propertiesAndStrings.append( qMakePair(prop, captionOrName) );
        }
        qSort(propertiesAndStrings.begin(), propertiesAndStrings.end(), 
            Iterator_propertyAndStringLessThan);
        m_sorted.clear();
        foreach (const Iterator_PropertyAndString& propertyAndString, propertiesAndStrings) {
            m_sorted.append(propertyAndString.first);
        }
        // restart the iterator
        m_iterator = m_sorted.constBegin();
        m_end = m_sorted.constEnd();
        break;
    }
    default:
        m_sorted.clear();
        // restart the iterator
        m_iterator = m_set->d->listConstIterator();
        m_end = m_set->d->listConstEnd();
    }
    skipNotAcceptable();
}

void
Set::Iterator::operator ++()
{
    while (true) {
        ++m_iterator;
        if (!m_selector)
            return;
        // selector exists
        if (!current()) // end encountered
            return;
        if ((*m_selector)( *current() ))
            return;
    }
}

//////////////////////////////////////////////

Set::Set(QObject *parent, const QString &typeName)
        : QObject(parent)
        , d(new SetPrivate(this))
{
    setObjectName(typeName.toLower().toLatin1());

    d->ownProperty = true;
    d->groupDescriptions.insert("common", i18nc("General properties", "General"));
    d->typeName = typeName.toLower();
}


Set::Set(const Set &set)
        : QObject(0 /* implicit sharing the parent is dangerous */)
        , d(new SetPrivate(this))
{
    setObjectName(set.objectName());
    *this = set;
}

Set::Set(bool propertyOwner)
        : QObject(0)
        , d(new SetPrivate(this))
{
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
Set::addProperty(Property *property, QByteArray group)
{
    d->addProperty(property, group);
}

void
Set::removeProperty(Property *property)
{
    d->removeProperty(property);
}

void
Set::removeProperty(const QByteArray &name)
{
    Property *p = d->property(name);
    removeProperty(p);
}

void
Set::clear()
{
    d->clear();
}

void
Set::informAboutClearing(bool& cleared)
{
    cleared = false;
    d->informAboutClearing = &cleared;
}

/////////////////////////////////////////////////////

QByteArray Set::groupForProperty(Property *property) const
{
    return d->groupForProperty(property);
}

void
Set::addToGroup(const QByteArray &group, Property *property)
{
    if (!property || group.isEmpty())
        return;

    //do not add the same property to the group twice
    const QByteArray groupLower(group.toLower());
    if (d->groupForProperty(property) == groupLower) {
        kWarning() << "Group" << group << "already contains property" << property->name();
        return;
    }
    QList<QByteArray>& propertiesOfGroup = d->propertiesOfGroup[ groupLower ];
    if (propertiesOfGroup.isEmpty()) {
        d->groupNames.append(groupLower);
    }
    propertiesOfGroup.append(property->name());
    d->addPropertyToGroup(property, groupLower);
}

void
Set::removeFromGroup(Property *property)
{
    if (!property)
        return;
    const QByteArray group( d->groupForProperty(property) );
    if (group.isEmpty())
        return;
    QList<QByteArray>& propertiesOfGroup = d->propertiesOfGroup[ group ];
    propertiesOfGroup.removeAt( propertiesOfGroup.indexOf(property->name()) );
    if (propertiesOfGroup.isEmpty()) {
        //remove group as well
        d->propertiesOfGroup.remove(group);
        const int i = d->groupNames.indexOf(group);
        if (i != -1)
            d->groupNames.removeAt(i);
    }
    d->removePropertyFromGroup(property);
}

const QList<QByteArray>
Set::groupNames() const
{
    return d->groupNames;
}

const QList<QByteArray>
Set::propertyNamesForGroup(const QByteArray &group) const
{
    return d->propertiesOfGroup.value(group);
}

void
Set::setGroupDescription(const QByteArray &group, const QString desc)
{
    d->groupDescriptions.insert(group.toLower(), desc);
}

QString
Set::groupDescription(const QByteArray &group) const
{
    const QString result( d->groupDescriptions.value(group.toLower()) );
    if (!result.isEmpty())
        return result;
    return group;
}

void
Set::setGroupIcon(const QByteArray &group, const QString& icon)
{
    d->groupIcons.insert(group.toLower(), icon);
}

QString
Set::groupIcon(const QByteArray &group) const
{
    return d->groupIcons.value(group);
}


/////////////////////////////////////////////////////

uint
Set::count() const
{
    return d->count();
}

uint Set::count(const PropertySelector& selector) const
{
    uint result = 0;
    for (Set::Iterator it(*this, selector); it.current(); ++it, result++)
        ;
    return result;
}

bool
Set::isEmpty() const
{
    return d->isEmpty();
}

bool Set::hasVisibleProperties() const
{
    return d->visiblePropertiesCount() > 0;
}

bool Set::hasProperties(const PropertySelector& selector) const
{
    Set::Iterator it(*this, selector);
    return it.current();
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
    return d->property(name);
}

Property&
Set::property(const QByteArray &name) const
{
    return d->propertyOrNull(name);
}

Property&
Set::operator[](const QByteArray &name) const
{
    return d->propertyOrNull(name);
}

const Set&
Set::operator= (const Set & set)
{
    if (&set == this)
        return *this;

    clear();
    d->copyAttributesFrom(*set.d);
    d->copyPropertiesFrom(set.d->listConstIterator(), set.d->listConstEnd(), set);
    return *this;
}

QVariant Set::propertyValue(const QByteArray &name, const QVariant& defaultValue) const
{
    const Property *p = d->property(name);
    return p ? p->value() : defaultValue;
}

void
Set::changeProperty(const QByteArray &property, const QVariant &value)
{
    Property *p = d->property(property);
    if (p)
        p->setValue(value);
}

void Set::debug() const
{
    kDebug(30007) << *this;
}

QDebug KoProperty::operator<<(QDebug dbg, const Set &set)
{
    dbg.nospace() << "KoProperty::Set(";
    if (!set.typeName().isEmpty()) {
        dbg.nospace() << "TYPENAME=" << set.typeName();
        if (set.isEmpty()) {
            dbg.space() << "<EMPTY>)";
            return dbg.space();
        }
    }
    if (set.isEmpty()) {
        dbg.space() << "<EMPTY>)";
        return dbg.space();
    }
    dbg.nospace() << " PROPERTIES(" << set.count() << "):\n";

    Set::Iterator it(set);
    it.setOrder(Set::AlphabeticalByName);
    bool first = true;
    for ( ; it.current(); ++it) {
        if (first) {
            first = false;
        }
        else {
            dbg.nospace() << "\n";
        }
        dbg.nospace() << *it.current();
    }
    dbg.nospace() << "\n)";
    return dbg.space();
}

QByteArray
Set::previousSelection() const
{
    return d->prevSelection;
}

void
Set::setPreviousSelection(const QByteArray &prevSelection)
{
    d->prevSelection = prevSelection;
}

QString
Set::typeName() const
{
    return d->typeName;
}

void Set::addRelatedProperty(Property *p1, Property *p2) const
{
    p1->addRelatedProperty(p2);
}

/////////////////////////////////////////////////////

Buffer::Buffer()
        : Set(false)
{
    connect(this, SIGNAL(propertyChanged(KoProperty::Set&, KoProperty::Property&)),
            this, SLOT(intersectedChanged(KoProperty::Set&, KoProperty::Property&)));

    connect(this, SIGNAL(propertyReset(KoProperty::Set&, KoProperty::Property&)),
            this, SLOT(intersectedReset(KoProperty::Set&, KoProperty::Property&)));
}

Buffer::Buffer(const Set& set)
        : Set(false)
{
    connect(this, SIGNAL(propertyChanged(KoProperty::Set&, KoProperty::Property&)),
            this, SLOT(intersectedChanged(KoProperty::Set&, KoProperty::Property&)));

    connect(this, SIGNAL(propertyReset(KoProperty::Set&, KoProperty::Property&)),
            this, SLOT(intersectedReset(KoProperty::Set&, KoProperty::Property&)));

    init(set);
}

void Buffer::init(const Set& set)
{
    //deep copy of set
    const QList<Property*>::ConstIterator itEnd(set.d->listConstEnd());
    for (QList<Property*>::ConstIterator it(set.d->listConstIterator()); 
        it!=itEnd; ++it)
    {
        Property *prop = new Property(*(*it));
        QByteArray group = set.groupForProperty(*it);
        QString groupDesc = set.groupDescription( group );
        setGroupDescription(group, groupDesc);
        addProperty(prop, group);
        prop->addRelatedProperty(*it);
    }
}

void Buffer::intersect(const Set& set)
{
    if (isEmpty()) {
        init(set);
        return;
    }

    const QList<Property*>::ConstIterator itEnd(set.d->listConstEnd());
    for (QList<Property*>::ConstIterator it(set.d->listConstIterator()); 
        it!=itEnd; ++it)
    {
        const QByteArray key( (*it)->name() );
        Property *property = set.d->property( key );
        if (property) {
            blockSignals(true);
            (*it)->resetValue();
            (*it)->addRelatedProperty(property);
            blockSignals(false);
        } else {
            removeProperty(key);
        }
    }
}

void Buffer::intersectedChanged(Set& set, Property& prop)
{
    Q_UNUSED(set);
    if (!contains(prop.name()))
        return;

    const QList<Property*> *props = prop.related();
    for (QList<Property*>::ConstIterator it = props->constBegin(); it != props->constEnd(); ++it) {
        (*it)->setValue(prop.value(), false);
    }
}

void Buffer::intersectedReset(Set& set, Property& prop)
{
    Q_UNUSED(set);
    if (!contains(prop.name()))
        return;

    const QList<Property*> *props = prop.related();
    for (QList<Property*>::ConstIterator it = props->constBegin(); it != props->constEnd(); ++it)  {
        (*it)->setValue(prop.value(), false);
    }
}

//////////////////////////////////////////////

QMap<QByteArray, QVariant> KoProperty::propertyValues(const Set& set)
{
    QMap<QByteArray, QVariant> result;
    for (Set::Iterator it(set); it.current(); ++it) {
        result.insert(it.current()->name(), it.current()->value());
    }
    return result;
}

#include "Set.moc"
