/* This file is part of the KDE project
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _KO_GENERIC_REGISTRY_H_
#define _KO_GENERIC_REGISTRY_H_

#include <kdemacros.h>
#include <kdebug.h>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QHash>

/**
 * Base class for registry objects.
 *
 * Registered objects are owned by the registry.
 *
 * Items are mapped by QString as a unique Id.
 *
 * Exemple of use:
 * @code
 * class KoMyClassRegistry : public KoGenericRegistry<MyClass*> {
 * public:
 *   static KoMyClassRegistry * instance();
 * private:
 *  static KoMyClassRegistry* s_instance;
 * };
 *
 * KoMyClassRegistry *KoMyClassRegistry::s_instance = 0;
 * KoMyClassRegistry * KoMyClassRegistry::instance()
 * {
 *    if(s_instance == 0)
 *    {
 *      s_instance = new KoMyClassRegistry;
 *    }
 *    return s_instance;
 * }
 *
 * @endcode
 */
template<typename T>
class KoGenericRegistry
{
public:
    KoGenericRegistry() { }
    virtual ~KoGenericRegistry() { m_hash.clear(); }

public:
    /**
     * Add an object to the registry. If it is a QObject, make sure it isn't in the
     * QObject ownership hierarchy, since the registry itself is responsbile for
     * deleting it.
     *
     * @param item the item to add (NOTE: T must have an QString id() const   function)
     */
    void add(T item) {
        Q_ASSERT( item );
        QString id = item->id();
        if(m_hash.contains(id)) {
            kWarning() << "Registry already contains item" << id;
            m_doubleEntries << value(id);
            remove(id);
        }
        m_hash.insert(id, item);
    }

    /**
     * add an object to the registry
     * @param id the id of the object
     * @param item the item to add
     */
    void add(const QString &id, T item) {
        Q_ASSERT( item );
        if(m_hash.contains(id)) {
            kWarning() << "Registry already contains item" << id;
            m_doubleEntries << value(id);
            remove(id);
        }
        m_hash.insert(id, item);
    }

    /**
     * This function removes an item from the registry
     */
    void remove(const QString &id) {
        m_hash.remove(id);
    }

    /**
     * Retrieve the object from the registry based on the unique
     * identifier string.
     *
     * @param id the id
     */
    T get(const QString& id) const {
        return value(id);
    }

    /**
     * @return if there is an object stored in the registry identified
     * by the id.
     * @param id the unique identifier string
     */
    bool contains(const QString &id) const {
        return m_hash.contains(id);
    }

    /**
     * Retrieve the object from the registry based on the unique identifier string
     * @param id the id
     */
    const T value(const QString &id) const {
        return m_hash.value(id);
    }

    /**
     * @return a list of all keys
     */
    QList<QString> keys() const {
        return m_hash.keys();
    }

    int count() const {
        return m_hash.count();
    }

    QList<T> values() const {
        return m_hash.values();
    }

    QList<T> doubleEntries() const {
        return m_doubleEntries;
    }

private:

    QList<T> m_doubleEntries;

private:

    QHash<QString, T> m_hash;
};

#endif
