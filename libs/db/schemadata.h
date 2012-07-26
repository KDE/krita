/* This file is part of the KDE project
   Copyright (C) 2003 Jarosław Staniek <staniek@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KEXIDB_SCHEMADATA_H
#define KEXIDB_SCHEMADATA_H

#include "global.h"
#include "field.h"

namespace KexiDB
{

/*! Container class that stores common kexi object schema's properties like
 id, name, caption, help text.
 By kexi object we mean in-db storable object like table schema or query schema.
*/

class CALLIGRADB_EXPORT SchemaData
{
public:
    SchemaData(int obj_type = KexiDB::UnknownObjectType);
    virtual ~SchemaData();

    int type() const {
        return m_type;
    }
    int id() const {
        return m_id;
    }
    QString name() const {
        return m_name;
    }
    /*! The same as name(). Added to avoid conflict with QObject::name() */
    QString objectName() const {
        return m_name;
    }
    void setName(const QString& n) {
        m_name = n;
    }
    QString caption() const {
        return m_caption;
    }
    void setCaption(const QString& c) {
        m_caption = c;
    }
    QString captionOrName() const {
        return m_caption.isEmpty() ? m_name : m_caption;
    }
    QString description() const {
        return m_desc;
    }
    void setDescription(const QString& desc) {
        m_desc = desc;
    }

    /*! \return debug string useful for debugging */
    virtual QString schemaDataDebugString() const;

    /*! \return true if this is schema of native database object,
     like, for example like, native table. This flag
     is set when object schema (currently -- database table)
     is not retrieved using kexi__* schema storage system,
     but just based on the information about native table.

     By native object we mean the one that has no additional
     data like caption, description, etc. properties (no kexidb extensions).

     Native objects schemas are used mostly for representing
     kexi system (kexi__*) tables in memory for later reference;
     see Connection::tableNames().

     By default (on allocation) SchemaData objects are not native.
    */
    virtual bool isNative() const {
        return m_native;
    }

    /* Sets native flag */
    virtual void setNative(bool set) {
        m_native = set;
    }

protected:
    //! Clears all properties except 'type'.
    void clear();

    int m_type;
    int m_id;
    QString m_name;
    QString m_caption;
    QString m_desc;
    bool m_native : 1;

    friend class Connection;
};

} //namespace KexiDB

#endif
