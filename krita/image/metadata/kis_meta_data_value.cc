/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_meta_data_value.h"
#include <QDate>
#include <QPoint>
#include <QPointF>
#include <QRegExp>
#include <QVariant>
#include <QStringList>

#include <kis_debug.h>

using namespace KisMetaData;

struct Value::Private {
    Private() : propertyQualifier(0) {}
    union {
        QVariant* variant;
        QList<Value>* array;
        QMap<QString, Value>* structure;
        KisMetaData::SignedRational* signedRational;
        KisMetaData::UnsignedRational* unsignedRational;
    } value;
    ValueType type;
    Value* propertyQualifier;
};

Value::Value() : d(new Private)
{
    d->type = Invalid;
}


Value::Value(const QVariant& variant) : d(new Private)
{
    d->type = Value::Variant;
    d->value.variant = new QVariant(variant);
}

Value::Value(const QList<Value>& array, ValueType type) : d(new Private)
{
    Q_ASSERT(type == OrderedArray || type == UnorderedArray || type == AlternativeArray || type == LangArray);
    d->value.array = new QList<Value>(array);
    d->type = type; // TODO: I am hesitating about LangArray to keep them as array or convert them to maps
}

Value::Value(const QMap<QString, Value>& structure) : d(new Private)
{
    d->type = Structure;
    d->value.structure = new QMap<QString, Value>(structure);
}

Value::Value(const KisMetaData::SignedRational& signedRational) : d(new Private)
{
    d->type = Value::SignedRational;
    d->value.signedRational = new KisMetaData::SignedRational(signedRational);
}
Value::Value(const KisMetaData::UnsignedRational& unsignedRational) : d(new Private)
{
    d->type = Value::UnsignedRational;
    d->value.unsignedRational = new KisMetaData::UnsignedRational(unsignedRational);
}


Value::Value(const Value& v) : d(new Private)
{
    d->type = Invalid;
    *this = v;
}

Value& Value::operator=(const Value & v)
{
    Q_ASSERT(d->type == Invalid || d->type == v.d->type);
    d->type = v.d->type;
    switch (d->type) {
    case Invalid:
        break;
    case Variant:
        d->value.variant = new QVariant(*v.d->value.variant);
        break;
    case OrderedArray:
    case UnorderedArray:
    case AlternativeArray:
    case LangArray:
        d->value.array = new QList<Value>(*v.d->value.array);
        break;
    case Structure:
        d->value.structure = new QMap<QString, Value>(*v.d->value.structure);
        break;
    case SignedRational:
        d->value.signedRational = new KisMetaData::SignedRational(*v.d->value.signedRational);
    case UnsignedRational:
        d->value.unsignedRational = new KisMetaData::UnsignedRational(*v.d->value.unsignedRational);
    }
    delete d->propertyQualifier;
    if (v.d->propertyQualifier) {
        d->propertyQualifier = new Value(*v.d->propertyQualifier);
    } else {
        d->propertyQualifier = 0;
    }
    return *this;
}


Value::~Value()
{
    delete d;
}

Value::ValueType Value::type() const
{
    return d->type;
}

double Value::asDouble() const
{
    switch (type()) {
    case Variant:
        return d->value.variant->toDouble(0);
    case UnsignedRational:
        return d->value.unsignedRational->numerator / (double)d->value.unsignedRational->denominator;
    case SignedRational:
        return d->value.signedRational->numerator / (double)d->value.signedRational->denominator;
    default:
        return 0.0;
    }
    return 0.0;
}

int Value::asInteger() const
{
    switch (type()) {
    case Variant:
        return d->value.variant->toInt(0);
    case UnsignedRational:
        return d->value.unsignedRational->numerator / d->value.unsignedRational->denominator;
    case SignedRational:
        return d->value.signedRational->numerator / d->value.signedRational->denominator;
    default:
        return 0;
    }
    return 0;
}

QVariant Value::asVariant() const
{
    switch (type()) {
    case Variant:
        return *d->value.variant;
    case UnsignedRational:
        return QVariant(QString("%1 / %2").arg(d->value.unsignedRational->numerator).arg(d->value.unsignedRational->denominator));
    case SignedRational:
        return QVariant(QString("%1 / %2").arg(d->value.signedRational->numerator).arg(d->value.signedRational->denominator));
    default: break;
    }
    return QVariant();
}

bool Value::setVariant(const QVariant& variant)
{
    switch (type()) {
    case KisMetaData::Value::Invalid:
        *this = KisMetaData::Value(variant);
        return true;
    case UnsignedRational:
    case SignedRational: {
        QRegExp rx("([^\\/]*)\\/([^\\/]*)");
        rx.indexIn(variant.toString());
    }
    case KisMetaData::Value::Variant: {
        if (d->value.variant->type() == variant.type()) {
            *d->value.variant = variant;
            return true;
        }
    }
    return true;
    default:
        break;
    }
    return false;
}

KisMetaData::UnsignedRational Value::asUnsignedRational() const
{
    if (d->type == UnsignedRational) {
        return *d->value.unsignedRational;
    }
    return KisMetaData::UnsignedRational();
}

KisMetaData::SignedRational Value::asSignedRational() const
{
    if (d->type == SignedRational) {
        return *d->value.signedRational;
    }
    return KisMetaData::SignedRational();
}

QList<Value> Value::asArray() const
{
    if (isArray()) {
        return *d->value.array;
    }
    return QList<Value>();
}


bool Value::isArray() const
{
    return type() == OrderedArray || type() == UnorderedArray || type() == AlternativeArray;
}

QMap<QString, KisMetaData::Value> Value::asStructure() const
{
    if (type() == Structure) {
        return *d->value.structure;
    }
    return QMap<QString, KisMetaData::Value>();
}

QMap<QString, KisMetaData::Value>* Value::asStructure()
{
    if (type() == Structure) {
        return d->value.structure;
    }
    return 0;
}

QDebug operator<<(QDebug dbg, const Value &v)
{
    switch (v.type()) {
    case Value::Invalid:
        dbg.nospace() << "invalid value";
        break;
    case Value::Variant:
        dbg.nospace() << "Variant: " << v.asVariant();
        break;
    case Value::OrderedArray:
    case Value::UnorderedArray:
    case Value::AlternativeArray:
    case Value::LangArray:
        dbg.nospace() << "Array: " << v.asArray();
        break;
    case Value::Structure:
        dbg.nospace() << "Structure: " << v.asStructure();
        break;
    case Value::SignedRational:
        dbg.nospace() << "Signed rational: " << v.asSignedRational().numerator << " / " << v.asSignedRational().denominator;
        break;
    case Value::UnsignedRational:
        dbg.nospace() << "Unsigend rational: " << v.asUnsignedRational().numerator << " / " << v.asUnsignedRational().denominator;
        break;
    }
    return dbg.space();
}

bool Value::operator==(const Value& rhs) const
{
    if (d->type != rhs.d->type) return false;
    switch (d->type) {
    case Value::Invalid:
        return true;
    case Value::Variant:
        return asVariant() == rhs.asVariant();
    case Value::OrderedArray:
    case Value::UnorderedArray:
    case Value::AlternativeArray:
    case Value::LangArray:
        return asArray() == rhs.asArray();
    case Value::Structure:
        return asStructure() == rhs.asStructure();
    case Value::SignedRational:
        return asSignedRational() == rhs.asSignedRational();
    case Value::UnsignedRational:
        return asUnsignedRational() == rhs.asUnsignedRational();
    }
    return false;
}

Value& Value::operator+=(const Value & v)
{
    switch (d->type) {
    case Value::Invalid:
        Q_ASSERT(v.type() == Value::Invalid);
        break;
    case Value::Variant:
        Q_ASSERT(v.type() == Value::Variant);
        {
            QVariant v1 = d->value.variant;
            QVariant v2 = v.d->value.variant;
            Q_ASSERT(v2.canConvert(v1.type()));
            switch (v1.type()) {
            default:
                break;
            case QVariant::Date: {
                QDate date;
                date.fromJulianDay(v1.toDate().toJulianDay()
                                   + v2.toDate().toJulianDay());
                *d->value.variant = date;
            }
            break;
            case QVariant::DateTime: {
                QDateTime dt;
                dt.fromTime_t(
                    v1.toDateTime().toTime_t()
                    + v2.toDateTime().toTime_t());
                *d->value.variant = dt;
            }
            break;
            case QVariant::Double:
                *d->value.variant = v1.toDouble() + v2.toDouble();
                break;
            case QVariant::Int:
                *d->value.variant = v1.toInt() + v2.toInt();
                break;
            case QVariant::List:
                *d->value.variant = v1.toList() + v2.toList();
                break;
            case QVariant::LongLong:
                *d->value.variant = v1.toLongLong() + v2.toLongLong();
                break;
            case QVariant::Point:
                *d->value.variant = v1.toPoint() + v2.toPoint();
                break;
            case QVariant::PointF:
                *d->value.variant = v1.toPointF() + v2.toPointF();
                break;
            case QVariant::String:
                *d->value.variant = v1.toString() + v2.toString();
                break;
            case QVariant::StringList:
                *d->value.variant = v1.toStringList() + v2.toStringList();
                break;
            case QVariant::Time: {
                QTime t1 = v1.toTime();
                QTime t2 = v2.toTime();
                int h = t1.hour() + t2.hour();
                int m = t1.minute() + t2.minute();
                int s = t1.second() + t2.second();
                int ms = t1.msec() + t2.msec();
                if (ms > 999) {
                    ms -= 999; s++;
                }
                if (s > 60) {
                    s -= 60; m++;
                }
                if (m > 60) {
                    m -= 60; h++;
                }
                if (h > 24) {
                    h -= 24;
                }
                *d->value.variant = QTime(h, m, s, ms);
            }
            break;
            case QVariant::UInt:
                *d->value.variant = v1.toUInt() + v2.toUInt();
                break;
            case QVariant::ULongLong:
                *d->value.variant = v1.toULongLong() + v2.toULongLong();
                break;
            }

        }
        break;
    case Value::OrderedArray:
    case Value::UnorderedArray:
    case Value::AlternativeArray: {
        if (v.isArray()) {
            *(d->value.array) += *(v.d->value.array);
        } else {
            d->value.array->append(v);
        }
    }
    break;
    case Value::LangArray: {
        Q_ASSERT(v.type() == Value::LangArray);
    }
    break;
    case Value::Structure: {
        Q_ASSERT(v.type() == Value::Structure);
        break;
    }
    case Value::SignedRational: {
        Q_ASSERT(v.type() == Value::SignedRational);
        d->value.signedRational->numerator =
            (d->value.signedRational->numerator
             * v.d->value.signedRational->denominator)
            + (v.d->value.signedRational->numerator
               * d->value.signedRational->denominator);
        d->value.signedRational->denominator *= v.d->value.signedRational->denominator;
        break;
    }
    case Value::UnsignedRational: {
        Q_ASSERT(v.type() == Value::UnsignedRational);
        d->value.unsignedRational->numerator =
            (d->value.unsignedRational->numerator
             * v.d->value.unsignedRational->denominator)
            + (v.d->value.unsignedRational->numerator
               * d->value.unsignedRational->denominator);
        d->value.unsignedRational->denominator *=
            v.d->value.unsignedRational->denominator;
        break;
    }
    break;
    }
    return *this;
}
