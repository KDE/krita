/* This file is part of the KDE project
   Copyright (C) 2004-2010 Jarosław Staniek <staniek@kde.org>

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

#ifndef KEXIDB_UTILS_H
#define KEXIDB_UTILS_H

#include <QList>
#include <QVariant>
#include <QByteArray>

#include "connection.h"
#include "driver.h"

class QDomNode;
class QDomElement;
class QDomDocument;

namespace KexiDB
{
//! for convenience
inline CALLIGRADB_EXPORT bool deleteRow(Connection &conn, TableSchema *table,
                                     const QString &keyname, const QString &keyval)
{
    return table != 0 && conn.executeSQL("DELETE FROM " + table->name() + " WHERE "
                                         + keyname + "=" + conn.driver()->valueToSQL(Field::Text, QVariant(keyval)));
}

inline CALLIGRADB_EXPORT bool deleteRow(Connection &conn, const QString &tableName,
                                     const QString &keyname, const QString &keyval)
{
    return conn.executeSQL("DELETE FROM " + tableName + " WHERE "
                           + keyname + "=" + conn.driver()->valueToSQL(Field::Text, QVariant(keyval)));
}

inline CALLIGRADB_EXPORT bool deleteRow(Connection &conn, TableSchema *table,
                                     const QString &keyname, int keyval)
{
    return table != 0 && conn.executeSQL("DELETE FROM " + table->name() + " WHERE "
                                         + keyname + "=" + conn.driver()->valueToSQL(Field::Integer, QVariant(keyval)));
}

inline CALLIGRADB_EXPORT bool deleteRow(Connection &conn, const QString &tableName,
                                     const QString &keyname, int keyval)
{
    return conn.executeSQL("DELETE FROM " + tableName + " WHERE "
                           + keyname + "=" + conn.driver()->valueToSQL(Field::Integer, QVariant(keyval)));
}

/*! Delete record with two generic criterias. */
inline CALLIGRADB_EXPORT bool deleteRow(Connection &conn, const QString &tableName,
                                     const QString &keyname1, Field::Type keytype1, const QVariant& keyval1,
                                     const QString &keyname2, Field::Type keytype2, const QVariant& keyval2)
{
    return conn.executeSQL("DELETE FROM " + tableName + " WHERE "
                           + keyname1 + "=" + conn.driver()->valueToSQL(keytype1, keyval1)
                           + " AND " + keyname2 + "=" + conn.driver()->valueToSQL(keytype2, keyval2));
}

inline CALLIGRADB_EXPORT bool replaceRow(Connection &conn, TableSchema *table,
                                      const QString &keyname, const QString &keyval, const QString &valname, QVariant val, int ftype)
{
    if (!table || !KexiDB::deleteRow(conn, table, keyname, keyval))
        return false;
    return conn.executeSQL("INSERT INTO " + table->name()
                           + " (" + keyname + "," + valname + ") VALUES ("
                           + conn.driver()->valueToSQL(Field::Text, QVariant(keyval)) + ","
                           + conn.driver()->valueToSQL(ftype, val) + ")");
}

typedef QList<uint> TypeGroupList;

/*! \return list of types for type group \a typeGroup. */
CALLIGRADB_EXPORT const TypeGroupList typesForGroup(Field::TypeGroup typeGroup);

/*! \return list of i18n'd type names for type group \a typeGroup. */
CALLIGRADB_EXPORT QStringList typeNamesForGroup(Field::TypeGroup typeGroup);

/*! \return list of (not-i18n'd) type names for type group \a typeGroup. */
CALLIGRADB_EXPORT QStringList typeStringsForGroup(Field::TypeGroup typeGroup);

/*! \return default field type for type group \a typeGroup,
 for example, Field::Integer for Field::IntegerGroup.
 It is used e.g. in KexiAlterTableDialog, to properly fill
 'type' property when user selects type group for a field. */
CALLIGRADB_EXPORT Field::Type defaultTypeForGroup(Field::TypeGroup typeGroup);

/*! \return a slightly simplified type name for \a field.
 For BLOB type it returns i18n'd "Image" string or other, depending on the mime type.
 For numbers (either floating-point or integer) it returns i18n'd "Number: string.
 For other types it the same string as Field::typeGroupName() is returned. */
//! @todo support names of other BLOB subtypes
CALLIGRADB_EXPORT QString simplifiedTypeName(const Field& field);

/*! \return true if \a v represents an empty (but not null) value.
 Values of some types (as for strings) can be both empty and not null. */
inline bool isEmptyValue(Field *f, const QVariant &v)
{
    if (f->hasEmptyProperty() && v.toString().isEmpty() && !v.toString().isNull())
        return true;
    return v.isNull();
}

/*! Sets \a msg to an error message retrieved from object \a obj, and \a details
 to details of this error (server message and result number).
 Does nothing if \a obj is null or no error occurred.
 \a msg and \a details strings are not overwritten.
 If \a msg is not empty, \a obj's error message is appended to \a details.
 */
CALLIGRADB_EXPORT void getHTMLErrorMesage(Object* obj, QString& msg, QString &details);

/*! This methods works like above, but appends both a message and a description
 to \a msg. */
CALLIGRADB_EXPORT void getHTMLErrorMesage(Object* obj, QString& msg);

/*! This methods works like above, but works on \a result's  members instead. */
CALLIGRADB_EXPORT void getHTMLErrorMesage(Object* obj, ResultInfo *result);

/*! Function useful for building WHERE parts of sql statements.
Constructs an sql string like "fielname = value" for specific \a drv driver,
 field type \a t, \a fieldName and \a value. If \a value is null, "fieldname is NULL"
 string is returned. */
inline CALLIGRADB_EXPORT QString sqlWhere(Driver *drv, Field::Type t,
                                       const QString &fieldName, const QVariant &value)
{
    if (value.isNull())
        return fieldName + " is NULL";
    return fieldName + "=" + drv->valueToSQL(t, value);
}

/*! \return identifier for object \a objName of type \a objType
 or 0 if such object does not exist. */
CALLIGRADB_EXPORT int idForObjectName(Connection &conn, const QString& objName, int objType);

/*! Variant class providing a pointer to table or query. */
class CALLIGRADB_EXPORT TableOrQuerySchema
{
public:
    /*! Creates a new TableOrQuerySchema variant object, retrieving table or query schema
     using \a conn connection and \a name. If both table and query exists for \a name,
     table has priority over query.
     You should check whether a query or table has been found by testing
     (query() || table()) expression. */
    TableOrQuerySchema(Connection *conn, const QByteArray& name);

    /*! Creates a new TableOrQuerySchema variant object, retrieving table or query schema
     using \a conn connection and \a name. If \a table is true, \a name is assumed
     to be a table name, otherwise \a name is assumed to be a query name.
     You should check whether a query or table has been found by testing
     (query() || table()) expression. */
    TableOrQuerySchema(Connection *conn, const QByteArray& name, bool table);

    /*! Creates a new TableOrQuerySchema variant object. \a tableOrQuery must be of
     class TableSchema or QuerySchema.
     You should check whether a query or table has been found by testing
     (query() || table()) expression. */
    TableOrQuerySchema(FieldList &tableOrQuery);

    /*! Creates a new TableOrQuerySchema variant object, retrieving table or query schema
     using \a conn connection and \a id.
     You should check whether a query or table has been found by testing
     (query() || table()) expression. */
    TableOrQuerySchema(Connection *conn, int id);

    /*! Creates a new TableOrQuerySchema variant object, keeping a pointer so \a table
     object. */
    TableOrQuerySchema(TableSchema* table);

    /*! Creates a new TableOrQuerySchema variant object, keeping a pointer so \a query
     object. */
    TableOrQuerySchema(QuerySchema* query);

    //! \return a pointer to the query if it's provided
    QuerySchema* query() const {
        return m_query;
    }

    //! \return a pointer to the table if it's provided
    TableSchema* table() const {
        return m_table;
    }

    //! \return name of a query or table
    QByteArray name() const;

    //! \return caption (if present) or name of the table/query
    QString captionOrName() const;

    //! \return number of fields
    uint fieldCount() const;

    //! \return all columns for the table or the query
    const QueryColumnInfo::Vector columns(bool unique = false);

    /*! \return a field of the table or the query schema for name \a name
     or 0 if there is no such field. */
    Field* field(const QString& name);

    /*! Like Field* field(const QString& name);
     but returns all information associated with field/column \a name. */
    QueryColumnInfo* columnInfo(const QString& name);

    /*! \return connection object, for table or query or 0 if there's no table or query defined. */
    Connection* connection() const;

    /*! \return String for debugging purposes. */
    QString debugString();

    /*! Shows debug information about table or query. */
    void debug();

protected:
    QByteArray m_name; //!< the name is kept here because m_table and m_table can be 0
    //! and we still want name() and acptionOrName() work.
    TableSchema* m_table;
    QuerySchema* m_query;
};

//! @todo perhaps use quint64 here?
/*! \return number of rows that can be retrieved after executing \a sql statement
 within a connection \a conn. The statement should be of type SELECT.
 For SQL data sources it does not fetch any records, only "COUNT(*)"
 SQL aggregation is used at the backed.
 -1 is returned if error occurred. */
int rowCount(Connection &conn, const QString& sql);

//! @todo perhaps use quint64 here?
/*! \return number of rows that can be retrieved from \a tableSchema.
 The table must be created or retrieved using a Connection object,
 i.e. tableSchema.connection() must not return 0.
 For SQL data sources it does not fetch any records, only "COUNT(*)"
 SQL aggregation is used at the backed.
 -1 is returned if error occurred. */
CALLIGRADB_EXPORT int rowCount(const TableSchema& tableSchema);

//! @todo perhaps use quint64 here?
/*! Like above but operates on a query schema. */
CALLIGRADB_EXPORT int rowCount(QuerySchema& querySchema);

//! @todo perhaps use quint64 here?
/*! Like above but operates on a table or query schema variant. */
CALLIGRADB_EXPORT int rowCount(TableOrQuerySchema& tableOrQuery);

/*! \return a number of columns that can be retrieved from table or query schema.
 In case of query, expanded fields are counted. Can return -1 if \a tableOrQuery
 has neither table or query assigned. */
CALLIGRADB_EXPORT int fieldCount(TableOrQuerySchema& tableOrQuery);

/*! shows connection test dialog with a progress bar indicating connection testing
 (within a second thread).
 \a data is used to perform a (temporary) test connection. \a msgHandler is used to display errors.
 On successful connecting, a message is displayed. After testing, temporary connection is closed. */
CALLIGRADB_EXPORT void connectionTestDialog(QWidget* parent, const ConnectionData& data,
        MessageHandler& msgHandler);

/*! Saves connection data \a data into \a map. */
CALLIGRADB_EXPORT QMap<QString, QString> toMap(const ConnectionData& data);

/*! Restores connection data \a data from \a map. */
CALLIGRADB_EXPORT void fromMap(const QMap<QString, QString>& map, ConnectionData& data);

//! Used in splitToTableAndFieldParts().
enum SplitToTableAndFieldPartsOptions {
    FailIfNoTableOrFieldName = 0, //!< default value for splitToTableAndFieldParts()
    SetFieldNameIfNoTableName = 1 //!< see splitToTableAndFieldParts()
};

/*! Splits \a string like "table.field" into "table" and "field" parts.
 On success, a table name is passed to \a tableName and a field name is passed to \a fieldName.
 The function fails if either:
 - \a string is empty, or
 - \a string does not contain '.' character and \a option is FailIfNoTableOrFieldName
    (the default), or
 - '.' character is the first of last character of \a string (in this case table name
   or field name could become empty what is not allowed).

 If \a option is SetFieldNameIfNoTableName and \a string does not contain '.',
 \a string is passed to \a fieldName and \a tableName is set to QString()
 without failure.

 If function fails, \a tableName and \a fieldName remain unchanged.
 \return true on success. */
CALLIGRADB_EXPORT bool splitToTableAndFieldParts(const QString& string,
        QString& tableName, QString& fieldName,
        SplitToTableAndFieldPartsOptions option = FailIfNoTableOrFieldName);

/*! \return true if \a type supports "visibleDecimalPlaces" property. */
CALLIGRADB_EXPORT bool supportsVisibleDecimalPlacesProperty(Field::Type type);

/*! \return string constructed by converting \a value.
 * If \a decimalPlaces is < 0, all meaningful fractional digits are returned.
 * If \a automatically is 0, just integer part is returned.
 * If \a automatically is > 0, fractional part should take exactly
   N digits: if the fractional part is shorter than N, additional zeros are appended.
   For example, "12.345" becomes "12.345000" if N=6.

 No rounding is actually performed.
 KLocale::formatNumber() and KLocale::decimalSymbol() are used to get locale settings.

 @see KexiDB::Field::visibleDecimalPlaces() */
CALLIGRADB_EXPORT QString formatNumberForVisibleDecimalPlaces(double value, int decimalPlaces);

//! \return true if \a propertyName is a builtin field property.
CALLIGRADB_EXPORT bool isBuiltinTableFieldProperty(const QByteArray& propertyName);

//! \return true if \a propertyName is an extended field property.
CALLIGRADB_EXPORT bool isExtendedTableFieldProperty(const QByteArray& propertyName);

/*! \return type of field for integer value \a type.
 If \a type cannot be casted to KexiDB::Field::Type, KexiDB::Field::InvalidType is returned.
 This can be used when type information is deserialized from a string or QVariant. */
CALLIGRADB_EXPORT Field::Type intToFieldType(int type);

/*! Sets property values for \a field. \return true if all the values are valid and allowed.
 On failure contents of \a field is undefined.
 Properties coming from extended schema are also supported.
 This function is used e.g. by AlterTableHandler when property information comes in form of text.
 */
CALLIGRADB_EXPORT bool setFieldProperties(Field& field, const QHash<QByteArray, QVariant>& values);

/*! Sets property value for \a field. \return true if the property has been found and
 the value is valid for this property. On failure contents of \a field is undefined.
 Properties coming from extended schema are also supported as well as
   QVariant customProperty(const QString& propertyName) const;

 This function is used e.g. by AlterTableHandler when property information comes in form of text.
 */
CALLIGRADB_EXPORT bool setFieldProperty(Field& field, const QByteArray& propertyName,
                                     const QVariant& value);

/*! @return property value loaded from a DOM \a node, written in a QtDesigner-like
 notation: &lt;number&gt;int&lt;/number&gt; or &lt;bool&gt;bool&lt;/bool&gt;, etc. Supported types are
 "string", "cstring", "bool", "number". For invalid values null QVariant is returned.
 You can check the validity of the returned value using QVariant::type(). */
CALLIGRADB_EXPORT QVariant loadPropertyValueFromDom(const QDomNode& node, bool *ok);

/*! Convenience version of loadPropertyValueFromDom(). \return int value. */
CALLIGRADB_EXPORT int loadIntPropertyValueFromDom(const QDomNode& node, bool* ok);

/*! Convenience version of loadPropertyValueFromDom(). \return QString value. */
CALLIGRADB_EXPORT QString loadStringPropertyValueFromDom(const QDomNode& node, bool* ok);

/*! Saves integer element for value \a value to \a doc document within parent element
 \a parentEl. The value will be enclosed in "number" element and "elementName" element.
 Example: saveNumberElementToDom(doc, parentEl, "height", 15) will create
 \code
  <height><number>15</number></height>
 \endcode
 \return the reference to element created with tag elementName. */
CALLIGRADB_EXPORT QDomElement saveNumberElementToDom(QDomDocument& doc, QDomElement& parentEl,
        const QString& elementName, int value);

/*! Saves boolean element for value \a value to \a doc document within parent element
 \a parentEl. Like saveNumberElementToDom() but creates "bool" tags. True/false values will be
 saved as "true"/"false" strings.
 \return the reference to element created with tag elementName. */
CALLIGRADB_EXPORT QDomElement saveBooleanElementToDom(QDomDocument& doc, QDomElement& parentEl,
        const QString& elementName, bool value);

/*! \return an empty value that can be set for a database field of type \a type having
 "null" property set. Empty string is returned for text type, 0 for integer
 or floating-point types, false for boolean type, empty null byte array for BLOB type.
 For date, time and date/time types current date, time, date+time is returned, respectively.
 Returns null QVariant for unsupported values like KexiDB::Field::InvalidType.
 This function is efficient (uses a cache) and is heavily used by the AlterTableHandler
 for filling new columns. */
CALLIGRADB_EXPORT QVariant emptyValueForType(Field::Type type);

/*! \return a value that can be set for a database field of type \a type having
 "notEmpty" property set. It works in a similar way as
 @ref QVariant emptyValueForType( KexiDB::Field::Type type ) with the following differences:
 - " " string (a single space) is returned for Text and LongText types
 - a byte array with saved "filenew" PNG image (icon) for BLOB type
 Returns null QVariant for unsupported values like KexiDB::Field::InvalidType.
 This function is efficient (uses a cache) and is heavily used by the AlterTableHandler
 for filling new columns. */
CALLIGRADB_EXPORT QVariant notEmptyValueForType(Field::Type type);

//! Escaping types used in escapeBLOB().
enum BLOBEscapingType {
    BLOBEscapeXHex = 1,        //!< escaping like X'1FAD', used by sqlite (hex numbers)
    BLOBEscape0xHex,           //!< escaping like 0x1FAD, used by mysql (hex numbers)
    BLOBEscapeHex,              //!< escaping like 1FAD without quotes or prefixes
    BLOBEscapeOctal           //!< escaping like 'zk\\000$x', used by pgsql
    //!< (only non-printable characters are escaped using octal numbers)
    //!< See http://www.postgresql.org/docs/8.1/interactive/datatype-binary.html
};

//! @todo reverse function for BLOBEscapeOctal is available: processBinaryData() in pqxxcursor.cpp - move it here
/*! \return a string containing escaped, printable representation of \a array.
 Escaping is controlled by \a type. For empty array, QString() is returned,
 so if you want to use this function in an SQL statement, empty arrays should be
 detected and "NULL" string should be put instead.
 This is helper, used in Driver::escapeBLOB() and KexiDB::variantToString(). */
CALLIGRADB_EXPORT QString escapeBLOB(const QByteArray& array, BLOBEscapingType type);

/*! \return byte array converted from \a data of length \a length.
 \a data is escaped in format used by PostgreSQL's bytea datatype
 described at http://www.postgresql.org/docs/8.1/interactive/datatype-binary.html
 This function is used by PostgreSQL KexiDB and migration drivers. */
CALLIGRADB_EXPORT QByteArray pgsqlByteaToByteArray(const char* data, int length);

/*! \return string value serialized from a variant value \a v.
 This functions works like QVariant::toString() except the case when \a v is of type ByteArray.
 In this case KexiDB::escapeBLOB(v.toByteArray(), KexiDB::BLOBEscapeHex) is used.
 This function is needed for handling values of random type, for example "defaultValue"
 property of table fields can contain value of any type.
 Note: the returned string is an unescaped string. */
CALLIGRADB_EXPORT QString variantToString(const QVariant& v);

/*! \return variant value of type \a type for a string \a s that was previously serialized using
 \ref variantToString( const QVariant& v ) function.
 \a ok is set to the result of the operation. */
CALLIGRADB_EXPORT QVariant stringToVariant(const QString& s, QVariant::Type type, bool &ok);

/*! \return true if setting default value for \a field field is allowed. Fields with unique
 (and thus primary key) flags set do not accept  default values.
 False is returned aslo if \a field is 0. */
CALLIGRADB_EXPORT bool isDefaultValueAllowed(Field* field);

/*! Gets limits for values of type \a type. The result is put into \a minValue and \a maxValue.
 Supported types are Byte, ShortInteger, Integer and BigInteger
 Results for BigInteger or non-integer types are the same as for Integer due
 to limitation of int type.
 Signed integers are assumed. */
//! @todo add support for unsigned flag
CALLIGRADB_EXPORT void getLimitsForType(Field::Type type, int &minValue, int &maxValue);

/*! \return type that's maximum of two integer types \a t1 and \a t2, e.g. Integer for (Byte, Integer).
 If one of the types is not of the integer group, Field::InvalidType is returned. */
CALLIGRADB_EXPORT Field::Type maximumForIntegerTypes(Field::Type t1, Field::Type t2);

/*! \return QVariant value converted from null-terminated \a data string.
 In case of BLOB type, \a data is not nul lterminated, so passing length is needed. */
inline QVariant cstringToVariant(const char* data, KexiDB::Field* f, int length = -1)
{
    if (!data)
        return QVariant();
    // from mo st to least frequently used types:

    if (!f || f->isTextType())
        return QString::fromUtf8(data, length);
    if (f->isIntegerType()) {
        if (f->type() == KexiDB::Field::BigInteger)
            return QVariant(QString::fromLatin1(data, length).toLongLong());
        return QVariant(QString::fromLatin1(data, length).toInt());
    }
    if (f->isFPNumericType())
        return QString::fromLatin1(data, length).toDouble();
    if (f->type() == KexiDB::Field::BLOB)
        return QByteArray::fromRawData(data, length);
    // the default
//! @todo date/time?
    QVariant result(QString::fromUtf8(data, length));
    if (!result.convert(KexiDB::Field::variantType(f->type())))
        return QVariant();
    return result;
}

/*! \return default file-based driver mime type
 (typically something like "application/x-kexiproject-sqlite") */
CALLIGRADB_EXPORT QString defaultFileBasedDriverMimeType();

/*! \return icon name for default file-based driver
 (typically icon for something like "application/x-kexiproject-sqlite").
 @see KexiDB::defaultFileBasedDriverMimeType() */
CALLIGRADB_EXPORT QString defaultFileBasedDriverIconName();

/*! \return default file-based driver name (currently, "sqlite3"). */
CALLIGRADB_EXPORT QString defaultFileBasedDriverName();

//! A set created from static (0-terminated) array of raw null-terminated strings.
class CALLIGRADB_EXPORT StaticSetOfStrings
{
public:
    StaticSetOfStrings();
    StaticSetOfStrings(const char* array[]);
    ~StaticSetOfStrings();
    void setStrings(const char* array[]);
    bool isEmpty() const;
    bool contains(const QByteArray& string) const;
private:
    class Private;
    Private * const d;
};

/*! \return true if \a s is a valid identifier, ie. starts with a letter or '_' character
 and contains only letters, numbers and '_' character. */
CALLIGRADB_EXPORT bool isIdentifier(const QString& s);

//! \return Valid filename based on \a s
CALLIGRADB_EXPORT QString string2FileName(const QString &s);

//! QDateTime - a hack needed because QVariant(QTime) has broken isNull()
inline CALLIGRADB_EXPORT QDateTime stringToHackedQTime(const QString& s)
{
    if (s.isEmpty())
        return QDateTime();
    //  kDebug() << QDateTime( QDate(0,1,2), QTime::fromString( s, Qt::ISODate ) ).toString(Qt::ISODate);
    return QDateTime(QDate(0, 1, 2), QTime::fromString(s, Qt::ISODate));
}

}

#endif
