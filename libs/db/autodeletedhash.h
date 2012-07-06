/* This file is part of the KDE project
   Copyright (C) 2003-2012 Jarosław Staniek <staniek@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KEXIDB_AUTODELETEDHASH_H
#define KEXIDB_AUTODELETEDHASH_H

namespace KexiDB
{

//! @short Autodeleted hash
template <class Key, class T>
class AutodeletedHash : public QHash<Key, T>
{
public:
    AutodeletedHash(const AutodeletedHash& other) : QHash<Key, T>(other), m_autoDelete(false) {}
    AutodeletedHash(bool autoDelete = true) : QHash<Key, T>(), m_autoDelete(autoDelete) {}
    void setAutoDelete(bool set) {
        m_autoDelete = set;
    }
    bool autoDelete() const {
        return m_autoDelete;
    }
    ~AutodeletedHash() {
        if (m_autoDelete) qDeleteAll(*this);
    }
private:
    bool m_autoDelete;
};
}

#endif // KEXIDB_AUTODELETEDHASH_H
