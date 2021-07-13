/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "kis_exiv2.h"

#include <QDateTime>

#include "kis_iptc_io.h"
#include "kis_exif_io.h"
#include "kis_xmp_io.h"

#include <kis_meta_data_value.h>
#include <kis_debug.h>

// ---- Generic conversion functions ---- //

// Convert an exiv value to a KisMetaData value
KisMetaData::Value exivValueToKMDValue(const Exiv2::Value::AutoPtr value, bool forceSeq, KisMetaData::Value::ValueType arrayType)
{
    switch (value->typeId()) {
    case Exiv2::signedByte:
    case Exiv2::invalidTypeId:
    case Exiv2::lastTypeId:
    case Exiv2::directory:
        dbgMetaData << "Invalid value :" << value->typeId() << " value =" << value->toString().c_str();
        return KisMetaData::Value();
    case Exiv2::undefined: {
        dbgMetaData << "Undefined value :" << value->typeId() << " value =" << value->toString().c_str();
        QByteArray array(value->count() , 0);
        value->copy((Exiv2::byte*)array.data(), Exiv2::invalidByteOrder);
        return KisMetaData::Value(QString(array.toBase64()));
    }
    case Exiv2::unsignedByte:
    case Exiv2::unsignedShort:
    case Exiv2::unsignedLong:
    case Exiv2::signedShort:
    case Exiv2::signedLong: {
        if (value->count() == 1 && !forceSeq) {
            return KisMetaData::Value((int)value->toLong());
        } else {
            QList<KisMetaData::Value> array;
            for (int i = 0; i < value->count(); i++)
                array.push_back(KisMetaData::Value((int)value->toLong(i)));
            return KisMetaData::Value(array, arrayType);
        }
    }
    case Exiv2::asciiString:
    case Exiv2::string:
    case Exiv2::comment: // look at kexiv2 for the problem about decoding correctly that tag
        return KisMetaData::Value(value->toString().c_str());
    case Exiv2::unsignedRational:
        if(value->size() < 2)
        {
            dbgMetaData << "Invalid size :" << value->size() << " value =" << value->toString().c_str();
            return KisMetaData::Value();
        }
        return KisMetaData::Value(KisMetaData::Rational(value->toRational().first , value->toRational().second));
    case Exiv2::signedRational:
        if(value->size() < 2)
        {
            dbgMetaData << "Invalid size :" << value->size() << " value =" << value->toString().c_str();
            return KisMetaData::Value();
        }
        return KisMetaData::Value(KisMetaData::Rational(value->toRational().first , value->toRational().second));
    case Exiv2::date:
    case Exiv2::time:
        return KisMetaData::Value(QDateTime::fromString(value->toString().c_str(), Qt::ISODate));
    case Exiv2::xmpText:
    case Exiv2::xmpAlt:
    case Exiv2::xmpBag:
    case Exiv2::xmpSeq:
    case Exiv2::langAlt:
    default: {
        dbgMetaData << "Unknown type id :" << value->typeId() << " value =" << value->toString().c_str();
        //Q_ASSERT(false); // This point must never be reached !
        return KisMetaData::Value();
    }
    }
    dbgMetaData << "Unknown type id :" << value->typeId() << " value =" << value->toString().c_str();
    //Q_ASSERT(false); // This point must never be reached !
    return KisMetaData::Value();
}


// Convert a QtVariant to an Exiv value
Exiv2::Value* variantToExivValue(const QVariant& variant, Exiv2::TypeId type)
{
    switch (type) {
    case Exiv2::undefined: {
        QByteArray arr = QByteArray::fromBase64(variant.toString().toLatin1());
        return new Exiv2::DataValue((Exiv2::byte*)arr.data(), arr.size());
    }
    case Exiv2::unsignedByte:
        return new Exiv2::ValueType<uint16_t>(variant.toUInt(0));
    case Exiv2::unsignedShort:
        return new Exiv2::ValueType<uint16_t>(variant.toUInt(0));
    case Exiv2::unsignedLong:
        return new Exiv2::ValueType<uint32_t>(variant.toUInt(0));
    case Exiv2::signedShort:
        return new Exiv2::ValueType<int16_t>(variant.toInt(0));
    case Exiv2::signedLong:
        return new Exiv2::ValueType<int32_t>(variant.toInt(0));
    case Exiv2::date: {
        QDate date = variant.toDate();
        return new Exiv2::DateValue(date.year(), date.month(), date.day());
    }
    case Exiv2::asciiString:
        if (variant.type() == QVariant::DateTime) {
            return new Exiv2::AsciiValue(qPrintable(QLocale::c().toString(variant.toDateTime(), QStringLiteral("yyyy:MM:dd hh:mm:ss"))));
        } else
            return new Exiv2::AsciiValue(qPrintable(variant.toString()));
    case Exiv2::string: {
        if (variant.type() == QVariant::DateTime) {
            return new Exiv2::StringValue(qPrintable(QLocale::c().toString(variant.toDateTime(), QStringLiteral("yyyy:MM:dd hh:mm:ss"))));
        } else
            return new Exiv2::StringValue(qPrintable(variant.toString()));
    }
    case Exiv2::comment:
        return new Exiv2::CommentValue(qPrintable(variant.toString()));
    default:
        dbgMetaData << "Unhandled type:" << type;
        //Q_ASSERT(false);
        return 0;
    }
}

template<typename _TYPE_>
Exiv2::Value* arrayToExivValue(const KisMetaData::Value& value)
{
    Exiv2::ValueType<_TYPE_>* ev = new Exiv2::ValueType<_TYPE_>();
    for (int i = 0; i < value.asArray().size(); ++i) {
        ev->value_.push_back(qvariant_cast<_TYPE_>(value.asArray()[i].asVariant()));
    }
    return ev;
}

// Convert a KisMetaData to an Exiv value
Exiv2::Value* kmdValueToExivValue(const KisMetaData::Value& value, Exiv2::TypeId type)
{
    switch (value.type()) {
    case KisMetaData::Value::Invalid:
        return &*Exiv2::Value::create(Exiv2::invalidTypeId);
    case KisMetaData::Value::Variant: {
        return variantToExivValue(value.asVariant(), type);
    }
    case KisMetaData::Value::Rational:
        //Q_ASSERT(type == Exiv2::signedRational || type == Exiv2::unsignedRational);
        if (type == Exiv2::signedRational) {
            return new Exiv2::ValueType<Exiv2::Rational>(Exiv2::Rational(value.asRational().numerator, value.asRational().denominator));
        } else {
            return new Exiv2::ValueType<Exiv2::URational>(Exiv2::URational(value.asRational().numerator, value.asRational().denominator));
        }
    case KisMetaData::Value::OrderedArray:
        Q_FALLTHROUGH();
    case KisMetaData::Value::UnorderedArray:
        Q_FALLTHROUGH();
    case KisMetaData::Value::AlternativeArray: {
        switch (type) {
        case Exiv2::unsignedByte:
            return arrayToExivValue<uint16_t>(value);
        case Exiv2::unsignedShort:
            return arrayToExivValue<uint16_t>(value);
        case Exiv2::unsignedLong:
            return arrayToExivValue<uint32_t>(value);
        case Exiv2::signedShort:
            return arrayToExivValue<int16_t>(value);
        case Exiv2::signedLong:
            return arrayToExivValue<int32_t>(value);
        case Exiv2::string: {
            Exiv2::StringValue* ev = new Exiv2::StringValue();
            for (int i = 0; i < value.asArray().size(); ++i) {
                ev->value_ += qvariant_cast<QString>(value.asArray()[i].asVariant()).toLatin1().constData();
                if (i != value.asArray().size() - 1) ev->value_ += ',';
            }
            return ev;
        }
            break;
        default:
            dbgMetaData << type << " " << value;
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(0 && "Unknown alternative array type", 0);
            break;
        }
        break;
    }
    default:
        dbgMetaData << type << " " << value;
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(0 && "Unknown array type", 0);
        break;
    }
    return 0;
}

Exiv2::Value* kmdValueToExivXmpValue(const KisMetaData::Value& value)
{
    //Q_ASSERT(value.type() != KisMetaData::Value::Structure);
    switch (value.type()) {
    case KisMetaData::Value::Invalid:
        return new Exiv2::DataValue(Exiv2::invalidTypeId);
    case KisMetaData::Value::Variant: {
        QVariant var = value.asVariant();
        if (var.type() == QVariant::Bool) {
            if (var.toBool()) {
                return new Exiv2::XmpTextValue("True");
            } else {
                return new Exiv2::XmpTextValue("False");
            }
        } else {
            //Q_ASSERT(var.canConvert(QVariant::String));
            return new Exiv2::XmpTextValue(var.toString().toLatin1().constData());
        }
    }
    case KisMetaData::Value::Rational: {
        QString rat = "%1 / %2";
        rat = rat.arg(value.asRational().numerator);
        rat = rat.arg(value.asRational().denominator);
        return new Exiv2::XmpTextValue(rat.toLatin1().constData());
    }
    case KisMetaData::Value::AlternativeArray:
    case KisMetaData::Value::OrderedArray:
    case KisMetaData::Value::UnorderedArray: {
        Exiv2::XmpArrayValue* arrV = new Exiv2::XmpArrayValue;
        switch (value.type()) {
        case KisMetaData::Value::OrderedArray:
            arrV->setXmpArrayType(Exiv2::XmpValue::xaSeq);
            break;
        case KisMetaData::Value::UnorderedArray:
            arrV->setXmpArrayType(Exiv2::XmpValue::xaBag);
            break;
        case KisMetaData::Value::AlternativeArray:
            arrV->setXmpArrayType(Exiv2::XmpValue::xaAlt);
            break;
        default:
            // Cannot happen
            ;
        }
        Q_FOREACH (const KisMetaData::Value& v, value.asArray()) {
            Exiv2::Value* ev = kmdValueToExivXmpValue(v);
            if (ev) {
                arrV->read(ev->toString());
                delete ev;
            }
        }
        return arrV;
    }
    case KisMetaData::Value::LangArray: {
        Exiv2::Value* arrV = new Exiv2::LangAltValue;
        QMap<QString, KisMetaData::Value> langArray = value.asLangArray();
        for (QMap<QString, KisMetaData::Value>::iterator it = langArray.begin();
             it != langArray.end(); ++it) {
            QString exivVal;
            if (it.key() != "x-default") {
                exivVal = "lang=" + it.key() + ' ';
            }
            //Q_ASSERT(it.value().type() == KisMetaData::Value::Variant);
            QVariant var = it.value().asVariant();
            //Q_ASSERT(var.type() == QVariant::String);
            exivVal += var.toString();
            arrV->read(exivVal.toLatin1().constData());
        }
        return arrV;
    }
    case KisMetaData::Value::Structure:
    default: {
        warnKrita << "KisExiv2: Unhandled value type";
        return 0;
    }

    }
    warnKrita << "KisExiv2: Unhandled value type";
    return 0;
}

void KisExiv2::initialize()
{
    // XXX_EXIV: make the exiv io backends real plugins

    KisMetaData::IOBackendRegistry* ioreg = KisMetaData::IOBackendRegistry::instance();
    ioreg->add(new KisIptcIO);
    ioreg->add(new KisExifIO);
    ioreg->add(new KisXMPIO);
}
