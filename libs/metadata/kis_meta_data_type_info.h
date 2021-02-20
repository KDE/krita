/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _KIS_META_DATA_TYPE_INFO_H_
#define _KIS_META_DATA_TYPE_INFO_H_

#include <QList>
#include <QString>
#include <kritametadata_export.h>

namespace KisMetaData
{
class Parser;
class Schema;
class Value;
class KRITAMETADATA_EXPORT TypeInfo
{
public:
    enum PropertyType {
        BooleanType,
        IntegerType,
        DateType,
        TextType,
        OrderedArrayType,
        UnorderedArrayType,
        AlternativeArrayType,
        LangArrayType,
        StructureType,
        RationalType,
        GPSCoordinateType,
        OpenedChoice,
        ClosedChoice
    };
    class KRITAMETADATA_EXPORT Choice
    {
    public:
        Choice(const Value&, const QString& hint);
        Choice(const Choice&);
        Choice& operator=(const Choice&);
        ~Choice();
    public:
        const Value& value() const;
        const QString& hint() const;
    private:
        struct Private;
        Private* const d;
    };
private:
    TypeInfo(PropertyType _propertiesType);
    /**
     * Create a \ref TypeInfo for a
     */
    TypeInfo(PropertyType _propertiesType, const TypeInfo* _embedded);
    /**
     * Create a \ref TypeInfo for a choice (either open or closed).
     * @param _propertiesType either OpenedChoice or ClosedChoice
     */
    TypeInfo(PropertyType _propertiesType, const TypeInfo* _embedded, const QList< Choice >&);
    /**
     * Create a \ref TypeInfo for a structure.
     */
    TypeInfo(Schema* _structureSchema, const QString& name);
    ~TypeInfo();
public:
    PropertyType propertyType() const;
    const TypeInfo* embeddedPropertyType() const;
    const QList< Choice >& choices() const;
    Schema* structureSchema() const;
    const QString& structureName() const;
    const Parser* parser() const;
    /**
     * @return true if @p value has a type that is correct for this \ref TypeInfo
     */
    bool hasCorrectType(const Value& value) const;
    /**
     * @return true if @p value has a value acceptable for this \ref TypeInfo
     */
    bool hasCorrectValue(const Value& value) const;
public:
    struct Private;
private:
    Private* const d;
};
}

#endif
