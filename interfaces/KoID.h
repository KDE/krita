/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 * Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef _KO_ID_H_
#define _KO_ID_H_

#include <QString>
#include <QMetaType>
#include <QDebug>

#include <klocalizedstring.h>

/**
 * A KoID is a combination of a user-visible string and a string that uniquely
 * identifies a given resource across languages.
 */
class KoID
{
public:
    KoID()
        : m_id()
        , m_name()
    {}

    /**
     * Construct a KoID with the given id, and name, id is the untranslated
     * official name of the id, name should be translatable as it will be used
     * in the UI.
     *
     * @code
     * KoID("id", i18n("name"))
     * @endcode
     */
    explicit KoID(const QString &id, const QString &name = QString())
        : m_id(id)
        , m_name(name)
    {}

    /**
     * Use this constructore for static KoID. as KoID("id", ki18n("name"));
     * the name will be translated the first time it is needed. This is
     * important because static objects are constructed before translations
     * are initialized.
     */
    explicit KoID(const QString &id, const KLocalizedString &name)
        : m_id(id)
        , m_localizedString(name)
    {}

    KoID(const KoID &rhs)
    {
        m_id = rhs.m_id;
        m_name = rhs.name();
    }

    KoID &operator=(const KoID &rhs)
    {
        if (this != &rhs) {
            m_id = rhs.m_id;
            m_name = rhs.name();
        }
        return *this;
    }

    QString id() const
    {
        return m_id;
    }

    QString name() const
    {
        if (m_name.isEmpty() && !m_localizedString.isEmpty()) {
            m_name = m_localizedString.toString();
        }
        return m_name;
    }

    friend inline bool operator==(const KoID &, const KoID &);
    friend inline bool operator!=(const KoID &, const KoID &);
    friend inline bool operator<(const KoID &, const KoID &);
    friend inline bool operator>(const KoID &, const KoID &);

    static bool compareNames(const KoID &id1, const KoID &id2)
    {
        return id1.name() < id2.name();
    }



private:

    QString m_id;
    mutable QString m_name;
    KLocalizedString m_localizedString;

};

Q_DECLARE_METATYPE(KoID)

inline bool operator==(const KoID &v1, const KoID &v2)
{
    return v1.m_id == v2.m_id;
}

inline bool operator!=(const KoID &v1, const KoID &v2)
{
    return v1.m_id != v2.m_id;
}

inline bool operator<(const KoID &v1, const KoID &v2)
{
    return v1.m_id < v2.m_id;
}

inline bool operator>(const KoID &v1, const KoID &v2)
{
    return v1.m_id > v2.m_id;
}

inline QDebug operator<<(QDebug dbg, const KoID &id)
{
    dbg.nospace() << id.name() << " (" << id.id() << " )";

    return dbg.space();
}

#endif
