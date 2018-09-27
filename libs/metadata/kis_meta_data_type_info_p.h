/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include "kis_meta_data_type_info.h"

#include <QHash>

struct KRITAMETADATA_EXPORT KisMetaData::TypeInfo::Private {
    Private() : embeddedTypeInfo(0), structureSchema(0), parser(0) {}
    PropertyType propertyType;
    const TypeInfo* embeddedTypeInfo;
    QList< Choice> choices;
    Schema* structureSchema;
    QString structureName;
    const Parser* parser;
private:
    static QHash< const TypeInfo*, const TypeInfo*> orderedArrays;
    static QHash< const TypeInfo*, const TypeInfo*> unorderedArrays;
    static QHash< const TypeInfo*, const TypeInfo*> alternativeArrays;
public:
    static const TypeInfo* Boolean;
    static const TypeInfo* Integer;
    static const TypeInfo* Date;
    static const TypeInfo* Text;
    static const TypeInfo* Rational;
    static const TypeInfo* GPSCoordinate;
    static const TypeInfo* orderedArray(const TypeInfo*);
    static const TypeInfo* unorderedArray(const TypeInfo*);
    static const TypeInfo* alternativeArray(const TypeInfo*);
    static const TypeInfo* createChoice(PropertyType _propertiesType, const TypeInfo* _embedded, const QList< Choice >&);
    static const TypeInfo* createStructure(Schema* _structureSchema, const QString& name);
    static const TypeInfo* LangArray;
};
