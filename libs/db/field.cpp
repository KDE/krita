/* This file is part of the KDE project
   Copyright (C) 2002 Lucijan Busch <lucijan@gmx.at>
   Copyright (C) 2002 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2003-2012 Jarosław Staniek <staniek@kde.org>

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

#include "field.h"
#include "connection.h"
#include "driver.h"
#include "expression.h"
#include "utils.h"

// we use here i18n() but this depends on kde libs: TODO: add #ifdefs
#include <kdebug.h>
#include <klocale.h>

#include <QDateTime>

#include <assert.h>

#include "calligradb_global.h"

using namespace KexiDB;

Field::FieldTypeNames Field::m_typeNames;
Field::FieldTypeGroupNames Field::m_typeGroupNames;

//! @todo make this configurable
static uint m_defaultMaxLength = 0; // unlimited

Field::Field()
{
    init();
    setConstraints(NoConstraints);
}


Field::Field(TableSchema *tableSchema)
{
    init();
    m_parent = tableSchema;
    m_order = tableSchema->fieldCount();
    setConstraints(NoConstraints);
}

Field::Field(QuerySchema *querySchema, BaseExpr* expr)
{
    init();
    m_parent = querySchema;
    m_order = querySchema->fieldCount();
    setConstraints(NoConstraints);
    if (expr)
        setExpression(expr);
}

Field::Field(const QString& name, Type ctype,
             uint cconst, uint options, uint maxLength, uint precision,
             QVariant defaultValue, const QString& caption, const QString& description,
             uint width)
        : m_parent(0)
        , m_name(name.toLower())
        , m_precision(precision)
        , m_visibleDecimalPlaces(-1)
        , m_options(options)
        , m_defaultValue(defaultValue)
        , m_order(-1)
        , m_caption(caption)
        , m_desc(description)
        , m_width(width)
        , m_expr(0)
        , m_customProperties(0)
        , m_type(ctype)
{
    setMaxLength(maxLength);
    setConstraints(cconst);
}

/*! Copy constructor. */
Field::Field(const Field& f)
{
    (*this) = f;
    if (f.m_customProperties)
        m_customProperties = new CustomPropertiesMap(f.customProperties());

    if (f.m_expr) {//deep copy the expression
//TODO  m_expr = new BaseExpr(*f.m_expr);

//  m_expr->m_field = this;
    } else
        m_expr = 0;
}

Field::~Field()
{
    //kDebug() << this << debugString();
    delete m_expr;
    delete m_customProperties;
}

Field* Field::copy() const
{
    return new Field(*this);
}

void Field::init()
{
    m_parent = 0;
    m_name = "";
    m_type = InvalidType;
    m_precision = 0;
    m_visibleDecimalPlaces = -1;
    m_options = NoOptions;
    m_defaultValue = QVariant(QString());
    m_order = -1;
    m_width = 0;
    m_expr = 0;
    m_customProperties = 0;
    setMaxLength(0); // do not move this line up!
    setMaxLengthStrategy(DefinedMaxLength); // do not move this line up!
}

Field::Type Field::type() const
{
    if (m_expr)
        return m_expr->type();
    return m_type;
}

QVariant::Type Field::variantType(uint type)
{
    switch (type) {
    case Byte:
    case ShortInteger:
    case Integer:
    case BigInteger:
        return QVariant::Int;
    case Boolean:
        return QVariant::Bool;
    case Date:
        return QVariant::Date;
    case DateTime:
        return QVariant::DateTime;
    case Time:
        return QVariant::Time;
    case Float:
    case Double:
        return QVariant::Double;
    case Text:
    case LongText:
        return QVariant::String;
    case BLOB:
        return QVariant::ByteArray;
    default:
        return QVariant::Invalid;
    }

    return QVariant::Invalid;
}

QString Field::typeName(uint type)
{
    m_typeNames.init();
    return m_typeNames.value(type, QString::number(type));
}

QStringList Field::typeNames()
{
    m_typeNames.init();
    return m_typeNames.names;
}

QString Field::typeString(uint type)
{
    m_typeNames.init();
    return (type <= LastType) ? m_typeNames.at((int)LastType + 1 + type) : QString("Type%1").arg(type);
}

QString Field::typeGroupName(uint typeGroup)
{
    m_typeGroupNames.init();
    return (typeGroup <= LastTypeGroup) ? m_typeGroupNames.at(typeGroup) : typeGroupString(typeGroup);
}

QStringList Field::typeGroupNames()
{
    m_typeGroupNames.init();
    return m_typeGroupNames.names;
}

QString Field::typeGroupString(uint typeGroup)
{
    m_typeGroupNames.init();
    return m_typeGroupNames.value((int)LastTypeGroup + 1 + typeGroup, QString("TypeGroup%1").arg(typeGroup));
}

Field::Type Field::typeForString(const QString& typeString)
{
    m_typeNames.init();
    return m_typeNames.str2num.value(typeString.toLower(), InvalidType);
}

Field::TypeGroup Field::typeGroupForString(const QString& typeGroupString)
{
    m_typeGroupNames.init();
    return m_typeGroupNames.str2num.value(typeGroupString.toLower(), InvalidGroup);
}

bool Field::isIntegerType(uint type)
{
    switch (type) {
    case Field::Byte:
    case Field::ShortInteger:
    case Field::Integer:
    case Field::BigInteger:
        return true;
    default:;
    }
    return false;
}

bool Field::isNumericType(uint type)
{
    switch (type) {
    case Field::Byte:
    case Field::ShortInteger:
    case Field::Integer:
    case Field::BigInteger:
    case Field::Float:
    case Field::Double:
        return true;
    default:;
    }
    return false;
}

bool Field::isFPNumericType(uint type)
{
    return type == Field::Float || type == Field::Double;
}

bool Field::isDateTimeType(uint type)
{
    switch (type) {
    case Field::Date:
    case Field::DateTime:
    case Field::Time:
        return true;
    default:;
    }
    return false;
}

bool Field::isTextType(uint type)
{
    switch (type) {
    case Field::Text:
    case Field::LongText:
        return true;
    default:;
    }
    return false;
}

bool Field::hasEmptyProperty(uint type)
{
    return Field::isTextType(type) || type == BLOB;
}

bool Field::isAutoIncrementAllowed(uint type)
{
    return Field::isIntegerType(type);
}

Field::TypeGroup Field::typeGroup(uint type)
{
    if (Field::isTextType(type))
        return TextGroup;
    else if (Field::isIntegerType(type))
        return IntegerGroup;
    else if (Field::isFPNumericType(type))
        return FloatGroup;
    else if (type == Boolean)
        return BooleanGroup;
    else if (Field::isDateTimeType(type))
        return DateTimeGroup;
    else if (type == BLOB)
        return BLOBGroup;

    return InvalidGroup; //unknown
}

TableSchema*
Field::table() const
{
    return dynamic_cast<TableSchema*>(m_parent);
}

void
Field::setTable(TableSchema *tableSchema)
{
    m_parent = tableSchema;
}

QuerySchema*
Field::query() const
{
    return dynamic_cast<QuerySchema*>(m_parent);
}

void
Field::setQuery(QuerySchema *querySchema)
{
    m_parent = querySchema;
}

void
Field::setName(const QString& n)
{
    m_name = n.toLower();
}

void
Field::setType(Type t)
{
    if (m_expr) {
        KexiDBWarn << QString("Field::setType(%1)").arg(t)
        << " could not set type because the field has expression assigned!";
        return;
    }
    m_type = t;
}

void
Field::setConstraints(uint c)
{
    m_constraints = c;
    //pkey must be unique notnull
    if (isPrimaryKey()) {
        setPrimaryKey(true);
    }
    if (isIndexed()) {
        setIndexed(true);
    }
    if (isAutoIncrement() && !isAutoIncrementAllowed()) {
        setAutoIncrement(false);
    }
}

uint Field::defaultMaxLength()
{
    return m_defaultMaxLength;
}

void Field::setDefaultMaxLength(uint maxLength)
{
    m_defaultMaxLength = maxLength;
}

Field::MaxLengthStrategy Field::maxLengthStrategy() const
{
    return m_maxLengthStrategy;
}

void Field::setMaxLengthStrategy(MaxLengthStrategy strategy)
{
    m_maxLengthStrategy = strategy;
}

uint Field::maxLength() const
{
    return m_maxLength;
}

void
Field::setMaxLength(uint maxLength)
{
    m_maxLength = maxLength;
    m_maxLengthStrategy = DefinedMaxLength;
}

void
Field::setPrecision(uint p)
{
    if (!isFPNumericType())
        return;
    m_precision = p;
}

void
Field::setScale(uint s)
{
    if (!isFPNumericType())
        return;
    m_maxLength = s;
}

void
Field::setVisibleDecimalPlaces(int p)
{
    if (!KexiDB::supportsVisibleDecimalPlacesProperty(type()))
        return;
    m_visibleDecimalPlaces = p < 0 ? -1 : p;
}

void
Field::setUnsigned(bool u)
{
    m_options |= Unsigned;
    m_options ^= (!u * Unsigned);
}

void
Field::setDefaultValue(const QVariant& def)
{
    m_defaultValue = def;
}

bool
Field::setDefaultValue(const QByteArray& def)
{
    if (def.isNull()) {
        m_defaultValue = QVariant();
        return true;
    }

    bool ok;
    switch (type()) {
    case Byte: {
        unsigned int v = def.toUInt(&ok);
        if (!ok || v > 255)
            m_defaultValue = QVariant();
        else
            m_defaultValue = QVariant(v);
        break;
    }
    case ShortInteger: {
        int v = def.toInt(&ok);
        if (!ok || (!(m_options & Unsigned) && (v < -32768 || v > 32767)) || ((m_options & Unsigned) && (v < 0 || v > 65535)))
            m_defaultValue = QVariant();
        else
            m_defaultValue = QVariant(v);
        break;
    }
    case Integer: {//4 bytes
        long v = def.toLong(&ok);
//js: FIXME   if (!ok || (!(m_options & Unsigned) && (-v > 0x080000000 || v > (0x080000000-1))) || ((m_options & Unsigned) && (v < 0 || v > 0x100000000)))
        if (!ok || (!(m_options & Unsigned) && (-v > (int)0x07FFFFFFF || v > (int)(0x080000000 - 1))))
            m_defaultValue = QVariant();
        else
            m_defaultValue = QVariant((qint64)v);
        break;
    }
    case BigInteger: {//8 bytes
//! @todo BigInteger support
        /*
              qint64 long v = def.toLongLong(&ok);
        //TODO: 2-part decoding
              if (!ok || (!(m_options & Unsigned) && (-v > 0x080000000 || v > (0x080000000-1))))
                m_defaultValue = QVariant();
              else
                if (m_options & Unsigned)
                  m_defaultValue=QVariant((quint64) v);
                else
                  m_defaultValue = QVariant((qint64)v);*/
        break;
    }
    case Boolean: {
        unsigned short v = def.toUShort(&ok);
        if (!ok || v > 1)
            m_defaultValue = QVariant();
        else
            m_defaultValue = QVariant((bool)v);
        break;
    }
    case Date: {//YYYY-MM-DD
        QDate date = QDate::fromString(def, Qt::ISODate);
        if (!date.isValid())
            m_defaultValue = QVariant();
        else
            m_defaultValue = QVariant(date);
        break;
    }
    case DateTime: {//YYYY-MM-DDTHH:MM:SS
        QDateTime dt = QDateTime::fromString(def, Qt::ISODate);
        if (!dt.isValid())
            m_defaultValue = QVariant();
        else
            m_defaultValue = QVariant(dt);
        break;
    }
    case Time: {//HH:MM:SS
        QTime time = QTime::fromString(def, Qt::ISODate);
        if (!time.isValid())
            m_defaultValue = QVariant();
        else
            m_defaultValue = QVariant(time);
        break;
    }
    case Float: {
        float v = def.toFloat(&ok);
        if (!ok || ((m_options & Unsigned) && (v < 0.0)))
            m_defaultValue = QVariant();
        else
            m_defaultValue = QVariant(v);
        break;
    }
    case Double: {
        double v = def.toDouble(&ok);
        if (!ok || ((m_options & Unsigned) && (v < 0.0)))
            m_defaultValue = QVariant();
        else
            m_defaultValue = QVariant(v);
        break;
    }
    case Text: {
        if (def.isNull() || (uint(def.length()) > maxLength()))
            m_defaultValue = QVariant();
        else
            m_defaultValue = QVariant((QString)def);
        break;
    }
    case LongText: {
        if (def.isNull())
            m_defaultValue = QVariant();
        else
            m_defaultValue = QVariant((QString)def);
        break;
    }
    case BLOB: {
//TODO
        if (def.isNull())
            m_defaultValue = QVariant();
        else
            m_defaultValue = QVariant(def);
        break;
    }
    default:
        m_defaultValue = QVariant();
    }
    return m_defaultValue.isNull();
}

void
Field::setAutoIncrement(bool a)
{
    if (a && !isAutoIncrementAllowed())
        return;
    if (isAutoIncrement() != a)
        m_constraints = static_cast<Field::Constraints>(m_constraints ^ Field::AutoInc);
}

void
Field::setPrimaryKey(bool p)
{
    if (isPrimaryKey() != p)
        m_constraints = static_cast<Field::Constraints>(m_constraints ^ Field::PrimaryKey);
    if (p) {//also set implied constraints
        setUniqueKey(true);
        setNotNull(true);
        setNotEmpty(true);
        setIndexed(true);
    } else {
//! \todo is this ok for all engines?
        setAutoIncrement(false);
    }
}

void
Field::setUniqueKey(bool u)
{
    if (isUniqueKey() != u) {
        m_constraints = static_cast<Field::Constraints>(m_constraints ^ Field::Unique);
        if (u) { //also set implied constraints
            setNotNull(true);
            setIndexed(true);
        }
    }
}

void
Field::setForeignKey(bool f)
{
    if (isForeignKey() != f)
        m_constraints = static_cast<Field::Constraints>(m_constraints ^ Field::ForeignKey);
}

void
Field::setNotNull(bool n)
{
    if (isNotNull() != n)
        m_constraints = static_cast<Field::Constraints>(m_constraints ^ Field::NotNull);
}

void Field::setNotEmpty(bool n)
{
    if (isNotEmpty() != n)
        m_constraints = static_cast<Field::Constraints>(m_constraints ^ Field::NotEmpty);
}

void Field::setIndexed(bool s)
{
    if (isIndexed() != s)
        m_constraints = static_cast<Field::Constraints>(m_constraints ^ Field::Indexed);
    if (!s) {//also set implied constraints
        setPrimaryKey(false);
        setUniqueKey(false);
        setNotNull(false);
        setNotEmpty(false);
    }
}


QString Field::debugString() const
{
    KexiDB::Connection *conn = table() ? table()->connection() : 0;
    QString dbg = (m_name.isEmpty() ? "<NONAME> " : m_name + " ");
    if (m_options & Field::Unsigned)
        dbg += " UNSIGNED ";
    dbg += (conn && conn->driver()) ? conn->driver()->sqlTypeName(type()) : Driver::defaultSQLTypeName(type());
    if (isFPNumericType() && m_precision > 0) {
        if (scale() > 0)
            dbg += QString::fromLatin1("(%1,%2)").arg(m_precision).arg(scale());
        else
            dbg += QString::fromLatin1("(%1)").arg(m_precision);
    } else if (m_type == Field::Text && m_maxLength > 0)
        dbg += QString::fromLatin1("(%1)").arg(m_maxLength);
    if (m_constraints & Field::AutoInc)
        dbg += " AUTOINC";
    if (m_constraints & Field::Unique)
        dbg += " UNIQUE";
    if (m_constraints & Field::PrimaryKey)
        dbg += " PKEY";
    if (m_constraints & Field::ForeignKey)
        dbg += " FKEY";
    if (m_constraints & Field::NotNull)
        dbg += " NOTNULL";
    if (m_constraints & Field::NotEmpty)
        dbg += " NOTEMPTY";
    if (!m_defaultValue.isNull())
        dbg += QString(" DEFAULT=[%1]").arg(m_defaultValue.typeName()) + KexiDB::variantToString(m_defaultValue);
    if (m_expr)
        dbg += " EXPRESSION=" + m_expr->debugString();
    if (m_customProperties && !m_customProperties->isEmpty()) {
        dbg += QString(" CUSTOM PROPERTIES (%1): ").arg(m_customProperties->count());
        bool first = true;
        for (CustomPropertiesMap::ConstIterator it(m_customProperties->constBegin());
                it != m_customProperties->constEnd(); ++it) {
            if (first)
                first = false;
            else
                dbg += ", ";
            dbg += QString("%1 = %2 (%3)").arg(QString(it.key())).arg(it.value().toString())
                   .arg(it.value().typeName());
        }
    }
    return dbg;
}

void Field::debug()
{
    KexiDBDbg << debugString();
}

void Field::setExpression(KexiDB::BaseExpr *expr)
{
    assert(!m_parent || dynamic_cast<QuerySchema*>(m_parent));
    if (m_expr == expr)
        return;
    if (m_expr) {
        delete m_expr;
    }
    m_expr = expr;
}

QVariant Field::customProperty(const QByteArray& propertyName,
                               const QVariant& defaultValue) const
{
    if (!m_customProperties)
        return defaultValue;
    return m_customProperties->value(propertyName, defaultValue);
}

void Field::setCustomProperty(const QByteArray& propertyName, const QVariant& value)
{
    if (propertyName.isEmpty())
        return;
    if (!m_customProperties)
        m_customProperties = new CustomPropertiesMap();
    m_customProperties->insert(propertyName, value);
}

//-------------------------------------------------------
#define ADDTYPE(type, i18, str) \
    (*this)[Field::type] = i18; \
    (*this)[Field::type+Field::LastType+1] = str; \
    str2num[ QString::fromLatin1(str).toLower() ] = type; \
    names.append(i18)
#define ADDGROUP(type, i18, str) \
    (*this)[Field::type] = i18; \
    (*this)[Field::type+Field::LastTypeGroup+1] = str; \
    str2num[ QString::fromLatin1(str).toLower() ] = type; \
    names.append(i18)

Field::FieldTypeNames::FieldTypeNames()
        : QVector<QString>()
        , m_initialized(false)
{
}

void Field::FieldTypeNames::init()
{
    if (m_initialized)
        return;
    m_initialized = true;
    resize((Field::LastType + 1)*2);

    ADDTYPE(InvalidType, i18n("Invalid Type"), "InvalidType");
    ADDTYPE(Byte, i18n("Byte"), "Byte");
    ADDTYPE(ShortInteger, i18n("Short Integer Number"), "ShortInteger");
    ADDTYPE(Integer, i18n("Integer Number"), "Integer");
    ADDTYPE(BigInteger, i18n("Big Integer Number"), "BigInteger");
    ADDTYPE(Boolean, i18n("Yes/No Value"), "Boolean");
    ADDTYPE(Date, i18n("Date"), "Date");
    ADDTYPE(DateTime, i18n("Date and Time"), "DateTime");
    ADDTYPE(Time, i18n("Time"), "Time");
    ADDTYPE(Float, i18n("Single Precision Number"), "Float");
    ADDTYPE(Double, i18n("Double Precision Number"), "Double");
    ADDTYPE(Text, i18n("Text"), "Text");
    ADDTYPE(LongText, i18n("Long Text"), "LongText");
    ADDTYPE(BLOB, i18n("Object"), "BLOB");
}

//-------------------------------------------------------

Field::FieldTypeGroupNames::FieldTypeGroupNames()
        : QVector<QString>()
        , m_initialized(false)
{
}

void Field::FieldTypeGroupNames::init()
{
    if (m_initialized)
        return;
    m_initialized = true;
    resize((Field::LastTypeGroup + 1)*2);

    ADDGROUP(InvalidGroup, i18n("Invalid Group"), "InvalidGroup");
    ADDGROUP(TextGroup, i18n("Text"), "TextGroup");
    ADDGROUP(IntegerGroup, i18n("Integer Number"), "IntegerGroup");
    ADDGROUP(FloatGroup, i18n("Floating Point Number"), "FloatGroup");
    ADDGROUP(BooleanGroup, i18n("Yes/No"), "BooleanGroup");
    ADDGROUP(DateTimeGroup, i18n("Date/Time"), "DateTimeGroup");
    ADDGROUP(BLOBGroup, i18n("Object"), "BLOBGroup");
}

//-------------------------------------------------------

