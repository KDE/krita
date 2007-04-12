/* This file is part of the KDE project
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _KO_GENERIC_REGISTRY_H_
#define _KO_GENERIC_REGISTRY_H_

#include "KoID.h"

#include <kdemacros.h>
#include <QList>
#include <QString>
#include <QHash>

/**
 * Base class for registry objects.
 *
 * Items are mapped by KoID. A KoID is the combination of
 * a non-localized string that can be used in files and a
 * user-visible, translated string that can be used in the
 * user interface.
 */
template<typename T>
class KoGenericRegistry {
public:
    KoGenericRegistry() { }
    virtual ~KoGenericRegistry() { }

public:
    /**
     * add an object to the registry
     * @param item the item to add (NOTE: T must have an QString id() const   function)
     */
    void add(T item)
    {
        m_hash.insert(item->id(), item);
    }

    /**
     * add an object to the registry
     * @param id the id of the object
     * @param item the item to add
     */
    void add(const QString &id, T item)
    {
        m_hash.insert(id, item);
    }

    /**
     * add an object to the registry
     * @param id the id of the object
     * @param item the item
     */
    KDE_DEPRECATED void add(KoID id, T item)
    {
        m_hash.insert(id.id(), item);
    }

    /**
     * This function remove an item from the registry
     * @return the object which have been remove from the registry and which can be safely delete
     */
    KDE_DEPRECATED void remove(const KoID& name)
    {
        m_hash.remove(name.id());
    }

    void remove (const QString &id) {
        m_hash.remove(id);
    }

    /**
     * This function allow to get an object from its KoID
     * @param name the KoID of the object
     * @return T the object
     */
    KDE_DEPRECATED T get(const KoID& name) const
    {
        return m_hash.value(name.id());
    }

    /**
     * Get a single entry based on the identifying part of KoID, not the
     * the descriptive part.
     */
    KDE_DEPRECATED T get(const QString& id) const
    {
        return value(id);
    }

    /**
     * @param id
     * @return true if there is an object corresponding to id
     */
    KDE_DEPRECATED bool exists(const KoID& id) const
    {
        return m_hash.contains(id.id());
    }

    KDE_DEPRECATED bool exists(const QString& id) const
    {
        return contains(id);
    }

    bool contains(const QString &id) const {
        return m_hash.contains(id);
    }

    const T value(const QString &id) const {
        return m_hash.value(id);
    }

#if 0
    /**
     * This function allow to search a KoID from the name.
     * @param t the name to search
     * @param result The result is filled in this variable
     * @return true if the search has been successful, false otherwise
     */
    KDE_DEPRECATED bool search(const QString& t, KoID& result) const
    {
        for(typename storageMap::const_iterator it = m_storage.begin();
            it != m_storage.end(); ++it)
        {
            if(it->first.name() == t)
            {
                result = it->first;
                return true;
            }
        }
        return false;
    }
#endif

    /** This function return a list of all the keys in KoID format
     */
    QList<KoID> listKeys() const
    {
        QList<KoID> answer;
        foreach(QString key, m_hash.keys())
            answer.append(KoID(key, value(key)->name()));
        return answer;
    }

    QList<QString> keys() const {
        return m_hash.keys();
    }

private:
    QHash<QString, T> m_hash;
};

#endif
