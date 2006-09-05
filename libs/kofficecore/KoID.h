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


/**
 * A KoID is a combination of a user-visible string and a string that uniquely
 * identifies a given resource across languages.
 */
class KoID {
public:
    KoID() : m_id(), m_name() {}

    KoID(const QString & id, const QString & name = QString())
        : m_id(id),
          m_name(name) {};

    QString id() const { return m_id; };
    QString name() const { return m_name; };

    friend inline bool operator==(const KoID &, const KoID &);
    friend inline bool operator!=(const KoID &, const KoID &);
    friend inline bool operator<(const KoID &, const KoID &);
    friend inline bool operator>(const KoID &, const KoID &);

private:

    QString m_id;
    QString m_name;

};

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
    return v1.m_id < v2.m_id;
}

#endif
