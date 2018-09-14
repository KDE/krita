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
#ifndef _KIS_META_DATA_VALUE_H_
#define _KIS_META_DATA_VALUE_H_

#include <QList>
#include <QMap>

#include <kritametadata_export.h>
#include <boost/operators.hpp>

class QVariant;

namespace KisMetaData
{

struct Rational : public boost::equality_comparable<Rational>
{
    explicit Rational(qint32 n = 0, qint32 d = 1) : numerator(n), denominator(d) {}
    qint32 numerator;
    qint32 denominator;
    bool operator==(const Rational& ur) const {
        return numerator == ur.numerator && denominator == ur.denominator;
    }
};

/**
 * Value is build on top of QVariant to extend it to support the various types
 * and extensions through property qualifiers.
 */
class KRITAMETADATA_EXPORT Value
{
    struct Private;
public:
    /// Define the possible value type
    enum ValueType {
        Invalid,
        Variant,
        OrderedArray,
        UnorderedArray,
        AlternativeArray,
        LangArray,
        Structure,
        Rational
    };
public:
    Value();
    Value(const QVariant& value);
    /**
    * @param type is one of OrderedArray, UnorderedArray, AlternativeArray
    * or LangArray
    */
    Value(const QList<Value>& array, ValueType type = OrderedArray);
    Value(const QMap<QString, Value>& structure);
    Value(const KisMetaData::Rational& rational);
    Value(const Value& v);
    Value& operator=(const Value& v);
    ~Value();
public:
    void addPropertyQualifier(const QString& _name, const Value&);
    const QMap<QString, Value>& propertyQualifiers() const;
public:
    /// @return the type of this Value
    ValueType type() const;
    /**
    * @return the value as a double, or null if it's not possible, rationals are evaluated
    */
    double asDouble() const;
    /**
    * @return the value as an integer, or null if it's not possible, rationals are evaluated
    */
    int asInteger() const;
    /**
    * @return the Variant hold by this Value, or an empty QVariant if this Value is not a Variant
    */
    QVariant asVariant() const;
    /**
    * Set this Value to the given variant, or does nothing if this Value is not a Variant.
    * @return true if the value was changed
    */
    bool setVariant(const QVariant& variant);
    bool setStructureVariant(const QString& fieldNAme, const QVariant& variant);
    bool setArrayVariant(int index, const QVariant& variant);
    /**
    * @return the Rational hold by this Value, or a null rational if this Value is not
    * an Rational
    */
    KisMetaData::Rational asRational() const;
    /**
    * @return the array hold by this Value, or an empty array if this Value is not either
    * an OrderedArray, UnorderedArray or AlternativeArray
    */
    QList<KisMetaData::Value> asArray() const;
    /**
    * @return true if this Value is either an OrderedArray, UnorderedArray or AlternativeArray
    */
    bool isArray() const;
    /**
    * @return the structure hold by this Value, or an empty structure if this Value is not a Structure
    */
    QMap<QString, KisMetaData::Value> asStructure() const;
    /**
    * It's a convenient function that build a map from a LangArray using the property
    * qualifier "xml:lang" for the key of the map.
    */
    QMap<QString, KisMetaData::Value> asLangArray() const;
    QString toString() const;
public:
    bool operator==(const Value&) const;
    Value& operator+=(const Value&);
private:
    Private* const d;
};
}


KRITAMETADATA_EXPORT QDebug operator<<(QDebug debug, const KisMetaData::Value &v);

#endif
