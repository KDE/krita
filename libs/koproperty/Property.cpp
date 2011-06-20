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

#include "Property.h"
#include "Property_p.h"
//#include "customproperty.h"
#include "Set.h"
#include "Factory.h"

#include <kdebug.h>

#include <QPointer>
#include <QByteArray>
#include <QStringList>
#include <QSizePolicy>

namespace KoProperty
{

//! @internal
class PropertyPrivate
{
public:
    PropertyPrivate()
            : caption(0), listData(0), changed(false), storable(true),
            readOnly(false), visible(true),
            autosync(-1), composed(0), useComposedProperty(true),
            sets(0), parent(0), children(0), relatedProperties(0)
//            sortingKey(0)
    {
    }

    inline void setCaptionForDisplaying(const QString& captionForDisplaying) {
        delete caption;
        if (captionForDisplaying.simplified() != captionForDisplaying) {
            if (captionForDisplaying.isEmpty()) {
                caption = 0;
            }
            else {
               caption = new QString(captionForDisplaying.simplified());
            }
        }
        else {
            caption = 0;
        }
        this->captionForDisplaying = captionForDisplaying;
    }

    ~PropertyPrivate() {
        delete caption;
        caption = 0;
        delete listData;
        if (children) {
            qDeleteAll(*children);
            delete children;
        }
        delete relatedProperties;
        delete composed;
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
// QMap<QString, QVariant> *valueList;
    QString icon;

    bool changed;
    bool storable;
    bool readOnly;
    bool visible;
    int autosync;
    QMap<QByteArray, QVariant> options;

    ComposedPropertyInterface *composed;
    //! Flag used to allow composed property to use setValue() without causing recursion
    bool useComposedProperty;

    //! Used when a single set is assigned for the property
    QPointer<Set> set;
    //! Used when multiple sets are assigned for the property
    QList< QPointer<Set> > *sets;
// QValueList<Set*>  sets;

    Property  *parent;
    QList<Property*>  *children;
    //! list of properties with the same name (when intersecting buffers)
    QList<Property*>  *relatedProperties;

//    int sortingKey;
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

Property::ListData::ListData(const QList<QVariant> keys_, const QStringList& names_)
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
    for (QStringList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it) {
        keys.append(*it);
    }
}

QStringList Property::ListData::keysAsStringList() const
{
    QStringList result;
    for (QList<QVariant>::ConstIterator it = keys.constBegin(); it != keys.constEnd(); ++it) {
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
        : d(new PropertyPrivate())
{
    d->name = name;
    d->setCaptionForDisplaying(caption);
    d->description = description;

    if (type == (int)Auto)
        d->type = value.type();
    else
        d->type = type;

    d->composed = FactoryManager::self()->createComposedProperty(this);

    if (parent)
        parent->addChild(this);
    setValue(value, false);
}

Property::Property(const QByteArray &name, const QStringList &keys, const QStringList &strings,
                   const QVariant &value, const QString &caption, const QString &description,
                   int type, Property* parent)
        : d(new PropertyPrivate())
{
    d->name = name;
    d->setCaptionForDisplaying(caption);
    d->description = description;
    d->type = type;
    setListData(keys, strings);

    d->composed = FactoryManager::self()->createComposedProperty(this);

    if (parent)
        parent->addChild(this);
    setValue(value, false);
}

Property::Property(const QByteArray &name, ListData* listData,
                   const QVariant &value, const QString &caption, const QString &description,
                   int type, Property* parent)
        : d(new PropertyPrivate())
{
    d->name = name;
    d->setCaptionForDisplaying(caption);
    d->description = description;
    d->type = type;
    d->listData = listData;

    d->composed = FactoryManager::self()->createComposedProperty(this);

    if (parent)
        parent->addChild(this);
    setValue(value, false);
}

Property::Property()
        : d(new PropertyPrivate())
{
}

Property::Property(const Property &prop)
        : d(new PropertyPrivate())
{
    *this = prop;
}

Property::~Property()
{
    delete d;
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
//??    if (d->custom && d->custom->handleValue())
//??        return d->custom->value();
    return d->value;
}

QVariant
Property::oldValue() const
{
    /* if(d->oldValue.isNull())
        return value();
      else*/
    return d->oldValue;
}

void
Property::childValueChanged(Property *child, const QVariant &value, bool rememberOldValue)
{
    if (!d->composed)
        return;
    d->composed->childValueChangedInternal(child, value, rememberOldValue);
}

//! @return true if @a currentValue and @a value are compatible
static bool compatibleTypes(const QVariant& currentValue, const QVariant &value)
{
    if (currentValue.isNull() || value.isNull())
        return true;
    const QVariant::Type t = currentValue.type();
    const QVariant::Type newt = value.type();
    if (t == newt)
        return true;
    if (   (t == QVariant::Int && newt == QVariant::UInt)
        || (t == QVariant::UInt && newt == QVariant::Int)
        || (t == QVariant::ByteArray && newt == QVariant::String)
        || (t == QVariant::String && newt == QVariant::ByteArray)
        || (t == QVariant::ULongLong && newt == QVariant::LongLong)
        || (t == QVariant::LongLong && newt == QVariant::ULongLong))
    {
        return true;
    }
    return false;
}

void Property::setValue(const QVariant &value, bool rememberOldValue, bool useComposedProperty)
{
    if (d->name.isEmpty()) {
        kWarning() << "COULD NOT SET value to a null property";
        return;
    }
    QVariant currentValue = this->value();
    if (!compatibleTypes(currentValue, value)) {
        kWarning() << "INCOMPATIBLE TYPES! old=" << currentValue << "new=" << value;
    }

    //1. Check if the value should be changed
    bool ch;
    const QVariant::Type t = currentValue.type();
    const QVariant::Type newt = value.type();
    if (   t == QVariant::DateTime
        || t == QVariant::Time)
    {
        //for date and datetime types: compare with strings, because there
        //can be miliseconds difference
        ch = (currentValue.toString() != value.toString());
    }
    else if (t == QVariant::String || t == QVariant::ByteArray) {
        //property is changed for string type,
        //if one of value is empty and other isn't..
        ch = ((currentValue.toString().isEmpty() != value.toString().isEmpty())
              //..or both are not empty and values differ
              || (!currentValue.toString().isEmpty() && !value.toString().isEmpty() && currentValue != value));
    }
    else if (t == QVariant::Double) {
        const double factor = 1.0 / option("step", KOPROPERTY_DEFAULT_DOUBLE_VALUE_STEP).toDouble();
        kDebug()
            << "double compared:" << currentValue.toDouble() << value.toDouble() 
            << ":" << static_cast<qlonglong>(currentValue.toDouble() * factor) << static_cast<qlonglong>(value.toDouble() * factor);
        ch = static_cast<qlonglong>(currentValue.toDouble() * factor) != static_cast<qlonglong>(value.toDouble() * factor);
    } else if (t == QVariant::Invalid && newt == QVariant::Invalid) {
        ch = false;
    } else if (t == QVariant::SizePolicy) {
        ch = (currentValue.value<QSizePolicy>() != value.value<QSizePolicy>());
    }
    else {
        ch = (currentValue != value);
    }

    if (!ch)
        return;

    //2. Then change it, and store old value if necessary
    if (rememberOldValue) {
        if (!d->changed)
            d->oldValue = currentValue;
        d->changed = true;
    }
    else {
        d->oldValue = QVariant(); // clear old value
        d->changed = false;
    }
    if (d->parent) {
        d->parent->childValueChanged(this, value, rememberOldValue);
    }

    QVariant prevValue;
    if (d->composed && useComposedProperty) {
        prevValue = currentValue; //???
        d->composed->setChildValueChangedEnabled(false);
        d->composed->setValue(this, value, rememberOldValue);
        d->composed->setChildValueChangedEnabled(true);
//        prevValue = d->composed->value();
    }
    else {
        prevValue = currentValue;
    }

//    if (!d->composed || !useComposedProperty)// || !composed->handleValue())
    d->value = value;

    if (!d->parent) { // emit only if parent has not done it
        emitPropertyChanged(); // called as last step in this method!
    }
}

void
Property::resetValue()
{
    d->changed = false;
    bool cleared = false;
    if (d->set)
        d->set->informAboutClearing(cleared); //inform me about possibly clearing the property sets
    setValue(oldValue(), false);
    if (cleared)
        return; //property set has been cleared: no further actions make sense as 'this' is dead

    // maybe parent  prop is also unchanged now
    if (d->parent && d->parent->value() == d->parent->oldValue())
        d->parent->d->changed = false;

    if (d->sets) {
        foreach (QPointer<Set> set, *d->sets) {
            if (!set.isNull()) //may be destroyed in the meantime
                emit set->propertyReset(*set, *this);
        }
    } else if (d->set) {
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
// if(!d->valueList)
//  d->valueList = new QMap<QString, QVariant>();
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

// if(!d->valueList)
//  d->valueList = new QMap<QString, QVariant>();
// *(d->valueList) = createValueListFromStringLists(keys, values);
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

void
Property::clearModifiedFlag()
{
    d->changed = false;
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
Property::option(const char* name, const QVariant& defaultValue) const
{
    if (d->options.contains(name))
        return d->options[name];
    return defaultValue;
}

bool
Property::hasOptions() const
{
    return !d->options.isEmpty();
}

/////////////////////////////////////////////////////////////////

const Property&
Property::operator= (const QVariant & val)
{
    setValue(val);
    return *this;
}

const Property&
Property::operator= (const Property & property)
{
    if (&property == this)
        return *this;

    delete d->listData;
    d->listData = 0;
    delete d->children;
    d->children = 0;
    delete d->relatedProperties;
    d->relatedProperties = 0;
    delete d->composed;
    d->composed = 0;

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

    if (property.d->listData) {
        d->listData = new ListData(*property.d->listData); //QMap<QString, QVariant>(*(property.d->valueList));
    }
    if (property.d->composed) {
        delete d->composed;
        d->composed = FactoryManager::self()->createComposedProperty(this);
        // updates all children value, using ComposedPropertyInterface
        setValue(property.value());
    } else {
        d->value = property.d->value;
        if (property.d->children) {
            // no ComposedPropertyInterface (should never happen), simply copy all children
            d->children = new QList<Property*>();
            QList<Property*>::ConstIterator endIt = property.d->children->constEnd();
            for (QList<Property*>::ConstIterator it = property.d->children->constBegin(); it != endIt; ++it) {
                Property *child = new Property(*(*it));
                addChild(child);
            }
        }
    }

    if (property.d->relatedProperties) {
        d->relatedProperties = new QList<Property*>(*(property.d->relatedProperties));
    }

    // update these later because they may have been changed when creating children
    d->oldValue = property.d->oldValue;
    d->changed = property.d->changed;
//    d->sortingKey = property.d->sortingKey;

    return *this;
}

bool
Property::operator ==(const Property &prop) const
{
    return ((d->name == prop.d->name) && (value() == prop.value()));
}

/////////////////////////////////////////////////////////////////

const QList<Property*>*
Property::children() const
{
    return d->children;
}

Property*
Property::child(const QByteArray &name)
{
    QList<Property*>::ConstIterator endIt = d->children->constEnd();
    for (QList<Property*>::ConstIterator it = d->children->constBegin(); it != endIt; ++it) {
        if ((*it)->name() == name)
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

    if (!d->children || qFind(d->children->begin(), d->children->end(), prop) == d->children->end()) { // not in our list
        if (!d->children)
            d->children = new QList<Property*>();
        d->children->append(prop);
//        prop->setSortingKey(d->children->count());
        prop->d->parent = this;
    } else {
        kWarning() << "property" << name()
                << ": child property" << prop->name() << "already added";
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
    if ((Set*)d->set == set || d->sets->contains(set))
        return;
    if (!d->sets) {
        d->sets = new QList< QPointer<Set> >;
    }

    d->sets->append(QPointer<Set>(set));
}

const QList<Property*>*
Property::related() const
{
    return d->relatedProperties;
}

void
Property::addRelatedProperty(Property *property)
{
    if (!d->relatedProperties)
        d->relatedProperties = new QList<Property*>();

    QList<Property*>::iterator it = qFind(d->relatedProperties->begin(), d->relatedProperties->end(), property);
    if (it == d->relatedProperties->end()) // not in our list
        d->relatedProperties->append(property);
}

ComposedPropertyInterface* Property::composedProperty() const
{
    return d->composed;
}

void
Property::setComposedProperty(ComposedPropertyInterface *prop)
{
    if (d->composed == prop)
        return;
    delete d->composed;
    d->composed = prop;
}

#if 0
int Property::sortingKey() const
{
    return d->sortingKey;
}

void Property::setSortingKey(int key)
{
    d->sortingKey = key;
}
#endif

void Property::emitPropertyChanged()
{
    QList< QPointer<Set> > *sets = 0;
    if (d->sets) {
        sets = d->sets;
    }
    else if (d->parent) {
        sets = d->parent->d->sets;
    }
    if (sets) {
        foreach (QPointer<Set> s, *sets) {
            if (!s.isNull()) { //may be destroyed in the meantime
                emit s->propertyChangedInternal(*s, *this);
                emit s->propertyChanged(*s, *this);
            }
        }
    }
    else {
        QPointer<Set> set;
        set = d->set;
        if (d->set) {
            set = d->set;
        }
        else if (d->parent) {
            set = d->parent->d->set;
        }
        if (!set.isNull()) {
            //if the slot connect with that signal may call set->clear() - that's
            //the case e.g. at kexi/plugins/{macros|scripting}/* -  this Property
            //may got destroyed ( see Set::removeProperty(Property*) ) while we are
            //still on it. So, if we try to access ourself/this once the signal
            //got emitted we may end in a very hard to reproduce crash. So, the
            //emit should happen as last step in this method!
            emit set->propertyChangedInternal(*set, *this);
            emit set->propertyChanged(*set, *this);
        }
    }
}

const QMap<QByteArray, QVariant>& Property::options() const
{
    return d->options;
}

/////////////////////////////////////////////////////////////////

void Property::debug() const
{
    kDebug(30007) << *this;
}

KOPROPERTY_EXPORT QDebug KoProperty::operator<<(QDebug dbg, const Property &p)
{
    dbg.nospace() << "KoProperty::Property("
        << "NAME=" << p.name();
    if (!p.caption().isEmpty()) {
        dbg.nospace() << " CAPTION=" << p.caption();
    }
    if (!p.description().isEmpty()) {
        dbg.nospace() << " DESC=" << p.description();
    }
    dbg.nospace() << " TYPE=" << p.type();
    if (p.value().isValid()) {
        dbg.nospace() << " VALUE=" << p.value();
    }
    else {
        dbg.nospace() << " VALUE=<INVALID>";
    }
    if (p.oldValue().isValid()) {
        dbg.nospace() << " OLDVALUE=" << p.oldValue();
    }
    if (p.isModified()) {
        dbg.nospace() << " MODIFIED";
    }
    if (!p.isVisible()) {
        dbg.nospace() << " HIDDEN";
    }

//! @todo children...

    if (p.hasOptions()) {
        dbg.nospace() << " OPTIONS(" << p.options().count() << "): [";
        QList<QByteArray> optionKeys( p.options().keys() );
        qSort(optionKeys);
        bool first = true;
        foreach (const QByteArray& key, optionKeys) {
            if (first) {
                first = false;
            }
            else {
                dbg.space() << ",";
            }
            dbg.nospace() << key << ":" << p.option(key);
        }
        dbg.nospace() << "]";
    }

    dbg.nospace() << ")";
    return dbg.space();
}
