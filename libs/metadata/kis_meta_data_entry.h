/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#ifndef _KIS_META_DATA_ENTRY_H_
#define _KIS_META_DATA_ENTRY_H_

#include <kritametadata_export.h>
#include <kis_debug.h>

class QString;

namespace KisMetaData
{
class Value;
class Store;
class Schema;
/**
 * Represent a metadata entry, a name and a value (\ref KisMetaData::Value).
 */
class KRITAMETADATA_EXPORT Entry
{
    struct Private;
    friend class Store;
public:
    /**
     * Create an invalid entry
     */
    Entry();
    /**
     * Create a new entry.
     * @param name
     * @param namespacePrefix
     * @param value
     */
    Entry(const KisMetaData::Schema* schema, QString name, const KisMetaData::Value& value);
    Entry(const Entry&);
    ~Entry();
    /**
     * @return the name of this entry
     */
    QString name() const;
    /**
     * @return the namespace of this entry
     */
    const KisMetaData::Schema* schema() const;
    /**
     * @return the qualified name of this entry, which is the concatenation of the
     * namespace and of the name
     */
    QString qualifiedName() const;
    /**
     * @return the value of this entry
     */
    const KisMetaData::Value& value() const;
    /**
     * @return the value of this entry
     */
    KisMetaData::Value& value();
    /**
     * @return true if this entry is valid
     */
    bool isValid() const;
    /**
     * @return true if the name in argument is valid entry name.
     */
    static bool isValidName(const QString& _name);
    /**
     * Affect the content of entry to this entry if entry is valid
     */
    Entry& operator=(const Entry& entry);
    bool operator==(const Entry&) const;
private:
    void setSchema(const KisMetaData::Schema* schema);
private:
    Private* const d;
};
}

KRITAMETADATA_EXPORT QDebug operator<<(QDebug debug, const KisMetaData::Entry &c);

#endif
