/* 
 * This file is part of the KDE project
 * 
 * Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_ID_H_
#define _KIS_ID_H_

#include <qvaluelist.h>
#include <qstring.h>

/**
 * Krita has a large number of extensible resources. Think:
 *
 * - Brushes
 * - Palettes
 * - Patterns
 * - Gradients
 * - Color models
 * - Filters
 * - Composition operations
 * - Paint operations
 * - Tools
 * - Docker tabs
 * 
 * and more...
 *
 * Many of these resources are stored in KisGenericRegistry-based
 * registries. If we store these resources with a descriptive string
 * as a key use the same string in our UI, then our UI will not be
 * localizable, because the identifications of particular resources
 * will be stored in files, and those files need to be exchangeable.
 *
 * So, instead we use and ID class that couples an identification
 * string that will be the same across all languages, an i18n-able
 * string that will be used in comboboxes and that has a fast equality
 * operator to make it well suited for use as key in a registry map.
 *
 * That last bit has not been solved yet.
 *
 */
class KisID {


public:

    KisID() : m_id(QString::null), m_name(QString::null) {}

    KisID(const QString & id, const QString & name = QString::null)
        : m_id(id),
          m_name(name) {};

    QString id() const { return m_id; };
    QString name() const { return m_name; };

    friend inline bool operator==(const KisID &, const KisID &);
    friend inline bool operator!=(const KisID &, const KisID &);
    friend inline bool operator<(const KisID &, const KisID &);
    friend inline bool operator>(const KisID &, const KisID &);

private:

    QString m_id;
    QString m_name;

};

inline bool operator==(const KisID &v1, const KisID &v2)
{
     return v1.m_id == v2.m_id;
}

inline bool operator!=(const KisID &v1, const KisID &v2)
{
    return v1.m_id != v2.m_id;
}


inline bool operator<(const KisID &v1, const KisID &v2)
{
    return v1.m_id < v2.m_id;
}


inline bool operator>(const KisID &v1, const KisID &v2)
{
    return v1.m_id < v2.m_id;
}


typedef QValueList<KisID> KisIDList;

#endif // _KIS_ID_H_
