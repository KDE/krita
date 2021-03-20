/* This file is part of the KDE project
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _KO_GENERIC_REGISTRY_H_
#define _KO_GENERIC_REGISTRY_H_

#include <QList>
#include <QString>
#include <QHash>

#include "kis_assert.h"

/**
 * Base class for registry objects.
 *
 * Registered objects are owned by the registry.
 *
 * Items are mapped by QString as a unique Id.
 *
 * Example of use:
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
    virtual ~KoGenericRegistry()
    {
        m_hash.clear();
    }

public:
    /**
     * Add an object to the registry. If it is a QObject, make sure it isn't in the
     * QObject ownership hierarchy, since the registry itself is responsbile for
     * deleting it.
     *
     * @param item the item to add (NOTE: T must have an QString id() const   function)
     */
    void add(T item)
    {
        KIS_SAFE_ASSERT_RECOVER_RETURN(item);

        const QString id = item->id();
        KIS_SAFE_ASSERT_RECOVER_NOOP(!m_aliases.contains(id));

        if (m_hash.contains(id)) {
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
    void add(const QString &id, T item)
    {
        KIS_SAFE_ASSERT_RECOVER_RETURN(item);
        KIS_SAFE_ASSERT_RECOVER_NOOP(!m_aliases.contains(id));

        if (m_hash.contains(id)) {
            m_doubleEntries << value(id);
            remove(id);
        }
        m_hash.insert(id, item);
    }

    /**
     * This function removes an item from the registry
     */
    void remove(const QString &id)
    {
        m_hash.remove(id);
    }

    void addAlias(const QString &alias, const QString &id)
    {
        KIS_SAFE_ASSERT_RECOVER_NOOP(!m_hash.contains(alias));
        m_aliases[alias] = id;
    }

    void removeAlias(const QString &alias)
    {
        m_aliases.remove(alias);
    }

    /**
     * Retrieve the object from the registry based on the unique
     * identifier string.
     *
     * @param id the id
     */
    T get(const QString &id) const
    {
        return value(id);
    }

    /**
     * @return if there is an object stored in the registry identified
     * by the id.
     * @param id the unique identifier string
     */
    bool contains(const QString &id) const
    {
        bool result = m_hash.contains(id);

        if (!result && m_aliases.contains(id)) {
            result = m_hash.contains(m_aliases.value(id));
        }

        return result;
    }

    /**
     * Retrieve the object from the registry based on the unique identifier string
     * @param id the id
     */
    const T value(const QString &id) const
    {
        T result = m_hash.value(id);

        if (!result && m_aliases.contains(id)) {
            result = m_hash.value(m_aliases.value(id));
        }

        return result;
    }

    /**
     * @return a list of all keys
     */
    QList<QString> keys() const
    {
        return m_hash.keys();
    }

    int count() const
    {
        return m_hash.count();
    }

    QList<T> values() const
    {
        return m_hash.values();
    }

    QList<T> doubleEntries() const
    {
        return m_doubleEntries;
    }

private:

    QList<T> m_doubleEntries;

private:

    QHash<QString, T> m_hash;
    QHash<QString, QString> m_aliases;
};

#endif
