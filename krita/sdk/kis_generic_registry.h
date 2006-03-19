/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_GENERIC_REGISTRY_H_
#define _KIS_GENERIC_REGISTRY_H_

#include <map>

#include <qstring.h>
#include <kdebug.h>

#include <kis_id.h>

/**
 * Base class for registry objects in Krita. Krita registries
 * contain resources such as filters, tools or colorspaces.
 *
 * Items are mapped by KisID. A KisID is the combination of 
 * a non-localized string that can be used in files and a
 * user-visible, translated string that can be used in the 
 * user interface.
 */
template<typename _T>
class KisGenericRegistry {
protected:
    typedef std::map<KisID, _T> storageMap;
public:
    KisGenericRegistry() { };
    virtual ~KisGenericRegistry() { };
public:

    /**
     * add an object to the registry
     * @param item the item to add (NOTE: _T must have an KisID id() function)
     */
    void add(_T item)
    {
        m_storage.insert( typename storageMap::value_type( item->id(), item) );
    }
    /**
     * add an object to the registry
     * @param id the id of the object
     * @param item the item
     */
    void add(KisID id, _T item)
    {
        m_storage.insert(typename storageMap::value_type(id, item));
    }
    /**
     * This function remove an item from the registry
     * @return the object which have been remove from the registry and which can be safely delete
     */
    _T remove(const KisID& name)
    {
        _T p = 0;
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
    _T remove(const QString& id)
    {
        return remove(KisID(id,""));
    }
    /**
     * This function allow to get an object from its KisID
     * @param name the KisID of the object
     * @return _T the object
     */
    _T get(const KisID& name) const
    {
        _T p = 0;
        typename storageMap::const_iterator it = m_storage.find(name);
        if (it != m_storage.end()) {
            p = it->second;
        }
        return p;
    }

    /**
     * Get a single entry based on the identifying part of KisID, not the
     * the descriptive part.
     */
    _T get(const QString& id) const
    {
        return get(KisID(id, ""));
    }

    /**
     * @param id
     * @return true if there is an object corresponding to id
     */
    bool exists(const KisID& id) const
    {
        typename storageMap::const_iterator it = m_storage.find(id);
        return (it != m_storage.end());
    }

    bool exists(const QString& id) const
    {
        return exists(KisID(id, ""));
    }
    /**
     * This function allow to search a KisID from the name.
     * @param t the name to search
     * @param result The result is filled in this variable
     * @return true if the search has been successfull, false otherwise
     */
    bool search(const QString& t, KisID& result) const
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
    KisIDList listKeys() const
    {
        KisIDList list;
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
    KisGenericRegistry(const KisGenericRegistry&) { };
    KisGenericRegistry operator=(const KisGenericRegistry&) { };
    storageMap m_storage;
};

#endif
