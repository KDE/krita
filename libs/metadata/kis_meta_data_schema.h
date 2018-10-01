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

#ifndef _KIS_META_DATA_SCHEMA_H_
#define _KIS_META_DATA_SCHEMA_H_

#include <kritametadata_export.h>
#include <kis_debug.h>

class QString;

namespace KisMetaData
{

class SchemaRegistry;
class TypeInfo;

class KRITAMETADATA_EXPORT Schema
{
    friend class SchemaRegistry;

public:

    virtual ~Schema();

    static const QString TIFFSchemaUri;
    static const QString EXIFSchemaUri;
    static const QString DublinCoreSchemaUri;
    static const QString XMPSchemaUri;
    static const QString XMPRightsSchemaUri;
    static const QString XMPMediaManagementUri;
    static const QString MakerNoteSchemaUri;
    static const QString IPTCSchemaUri;
    static const QString PhotoshopSchemaUri;
private:
    Schema();
    Schema(const QString & _uri, const QString & _ns);
public:
    /**
     * @return the \ref TypeInfo associated with a given a property ( @p _propertyName ).
     */
    const TypeInfo* propertyType(const QString& _propertyName) const;
    /**
     * @return the \ref TypeInfo describing a given structure of that scheam
     */
    const TypeInfo* structure(const QString& _structureName) const;
public:
    QString uri() const;
    QString prefix() const;
    QString generateQualifiedName(const QString &) const;
private:
    struct Private;
    Private* const d;
};

}

KRITAMETADATA_EXPORT QDebug operator<<(QDebug debug, const KisMetaData::Schema &c);

#endif
