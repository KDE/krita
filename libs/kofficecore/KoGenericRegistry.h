/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KO_GENERIC_REGISTRY_H_
#define _KO_GENERIC_REGISTRY_H_

#include "KoID.h"

#include <map>

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
protected:
    typedef std::map<KoID, T> storageMap;
public:
    KoGenericRegistry() { }
    virtual ~KoGenericRegistry() { }

public:
    /**
     * add an object to the registry
     * @param item the item to add (NOTE: T must have an KoID id() function)
     */
    void add(T item)
    {
        m_storage.insert( typename storageMap::value_type( item->id(), item) );
    }
    /**
     * add an object to the registry
     * @param id the id of the object
     * @param item the item
     */
    void add(KoID id, T item)
    {
        m_storage.insert(typename storageMap::value_type(id, item));
    }
    /**
     * This function remove an item from the registry
     * @return the object which have been remove from the registry and which can be safely delete
     */
    T remove(const KoID& name)
    {
        T p = 0;
        typename storageMap::iterator it = m_storage.find(name);
        if (it != m_storage.end()) {
            m_storage.erase(it);
            p = it->second;
        }
        return p;
    }
    /**
     * This function remove an item from the registry
     * @param id the identifiant of the object
     * @return the object which have been remove from the registry and which can be safely delete
     */
    T remove(const QString& id)
    {
        return remove(KoID(id,""));
    }
    /**
     * This function allow to get an object from its KoID
     * @param name the KoID of the object
     * @return T the object
     */
    T get(const KoID& name) const
    {
        T p = T(0);
        typename storageMap::const_iterator it = m_storage.find(name);
        if (it != m_storage.end()) {
            p = it->second;
        }
        return p;
    }

    /**
     * Get a single entry based on a integer based id.
     * @param id the integer ID as used by Tools and Shapes.
     */
    T get(int id) const
    {
        return get(KoID(QString::number(id), ""));
    }

    /**
     * Get a single entry based on the identifying part of KoID, not the
     * the descriptive part.
     */
    T get(const QString& id) const
    {
        return get(KoID(id, ""));
    }

    /**
     * @param id
     * @return true if there is an object corresponding to id
     */
    bool exists(const KoID& id) const
    {
        typename storageMap::const_iterator it = m_storage.find(id);
        return (it != m_storage.end());
    }

    bool exists(const QString& id) const
    {
        return exists(KoID(id, ""));
    }
    /**
     * This function allow to search a KoID from the name.
     * @param t the name to search
     * @param result The result is filled in this variable
     * @return true if the search has been successfull, false otherwise
     */
    bool search(const QString& t, KoID& result) const
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

    /** This function return a list of all the keys
     */
    QList<KoID> listKeys() const
    {
        QList<KoID> list;
        typename storageMap::const_iterator it = m_storage.begin();
        typename storageMap::const_iterator endit = m_storage.end();
        while( it != endit )
        {
            list.append(it->first);
            ++it;
        }
        return list;
    }

protected:
    KoGenericRegistry(const KoGenericRegistry&) { }
    KoGenericRegistry operator=(const KoGenericRegistry&) { }
    storageMap m_storage;
};

#endif
