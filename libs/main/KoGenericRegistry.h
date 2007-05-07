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
 * Items are mapped by QString as a unique Id.
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
     * This function remove an item from the registry
     */
    void remove (const QString &id) {
        m_hash.remove(id);
    }

    /**
     * Retrieve the object from the registry based on the unique identifier string
     * @param id the id
     */
    KDE_DEPRECATED T get(const QString& id) const
    {
        return value(id);
    }

    /**
     * @return if there is an object stored in the registry identified by the id.
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
     * This function return a list of all the keys in KoID format by using the name() method
     * on the objects stored in the registry.
     */
    QList<KoID> listKeys() const
    {
        QList<KoID> answer;
        foreach(QString key, m_hash.keys())
            answer.append(KoID(key, value(key)->name()));
        return answer;
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

private:
    QHash<QString, T> m_hash;
};

#endif
