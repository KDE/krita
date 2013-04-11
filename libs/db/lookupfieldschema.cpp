/* This file is part of the KDE project
   Copyright (C) 2006-2012 Jarosław Staniek <staniek@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "lookupfieldschema.h"
#include "utils.h"

#include <QDomElement>
#include <QVariant>
#include <QStringList>

#include <kdebug.h>

namespace KexiDB
{
//! @internal
class LookupFieldSchema::RowSource::Private
{
public:
    Private()
            : type(LookupFieldSchema::RowSource::NoType) {
    }
    LookupFieldSchema::RowSource::Type type;
    QString name;
    QStringList values;
};

//! @internal
class LookupFieldSchema::Private
{
public:
    Private()
            : boundColumn(-1)
            , maximumListRows(KEXIDB_LOOKUP_FIELD_DEFAULT_LIST_ROWS)
            , displayWidget(KEXIDB_LOOKUP_FIELD_DEFAULT_DISPLAY_WIDGET)
            , columnHeadersVisible(KEXIDB_LOOKUP_FIELD_DEFAULT_HEADERS_VISIBLE)
            , limitToList(KEXIDB_LOOKUP_FIELD_DEFAULT_LIMIT_TO_LIST) {
    }

    RowSource rowSource;
    int boundColumn;
    QList<uint> visibleColumns;
    QList<int> columnWidths;
    uint maximumListRows;
    DisplayWidget displayWidget;
    bool columnHeadersVisible : 1;
    bool limitToList : 1;
};
}

//----------------------------

using namespace KexiDB;

LookupFieldSchema::RowSource::RowSource()
        : d(new Private)
{
}

LookupFieldSchema::RowSource::RowSource(const RowSource& other)
        : d(new Private)
{
    *d = *other.d;
}

LookupFieldSchema::RowSource::~RowSource()
{
    delete d;
}

LookupFieldSchema::RowSource::Type LookupFieldSchema::RowSource::type() const
{
    return d->type;
}

void LookupFieldSchema::RowSource::setType(Type type)
{
    d->type = type;
}

QString LookupFieldSchema::RowSource::name() const
{
    return d->name;
}

void LookupFieldSchema::RowSource::setName(const QString& name)
{
    d->name = name;
    d->values.clear();
}

QString LookupFieldSchema::RowSource::typeName() const
{
    switch (d->type) {
    case Table: return "table";
    case Query: return "query";
    case SQLStatement: return "sql";
    case ValueList: return "valuelist";
    case FieldList: return "fieldlist";
    default:;
    }
    return QString();
}

void LookupFieldSchema::RowSource::setTypeByName(const QString& typeName)
{
    if (typeName == "table")
        setType(Table);
    else if (typeName == "query")
        setType(Query);
    else if (typeName == "sql")
        setType(SQLStatement);
    else if (typeName == "valuelist")
        setType(ValueList);
    else if (typeName == "fieldlist")
        setType(FieldList);
    else
        setType(NoType);
}

QStringList LookupFieldSchema::RowSource::values() const
{
    return d->values;
}

void LookupFieldSchema::RowSource::setValues(const QStringList& values)
{
    d->name.clear();
    d->values = values;
}

LookupFieldSchema::RowSource& LookupFieldSchema::RowSource::operator=(const RowSource & other)
{
    if (this != &other) {
        *d = *other.d;
    }
    return *this;
}

QString LookupFieldSchema::RowSource::debugString() const
{
    return QString("rowSourceType:'%1' rowSourceName:'%2' rowSourceValues:'%3'")
           .arg(typeName()).arg(name()).arg(d->values.join("|"));
}

void LookupFieldSchema::RowSource::debug() const
{
    KexiDBDbg << debugString();
}

//----------------------------

LookupFieldSchema::LookupFieldSchema()
        : d(new Private)
{
}

LookupFieldSchema::LookupFieldSchema(const LookupFieldSchema &schema)
: d(new Private)
{
    *d = *schema.d;
}

LookupFieldSchema::~LookupFieldSchema()
{
    delete d;
}

LookupFieldSchema::RowSource& LookupFieldSchema::rowSource() const
{
    return d->rowSource;
}

void LookupFieldSchema::setRowSource(const LookupFieldSchema::RowSource& rowSource)
{
    d->rowSource = rowSource;
}

void LookupFieldSchema::setMaximumListRows(uint rows)
{
    if (rows == 0)
        d->maximumListRows = KEXIDB_LOOKUP_FIELD_DEFAULT_LIST_ROWS;
    else if (rows > KEXIDB_LOOKUP_FIELD_MAX_LIST_ROWS)
        d->maximumListRows = KEXIDB_LOOKUP_FIELD_MAX_LIST_ROWS;
    else
        d->maximumListRows = rows;
}

QString LookupFieldSchema::debugString(const QString &fieldName) const
{
    QString columnWidthsStr;
    for (QList<int>::ConstIterator it = d->columnWidths.constBegin();
            it != d->columnWidths.constEnd();++it) {
        if (!columnWidthsStr.isEmpty())
            columnWidthsStr.append(";");
        columnWidthsStr.append(QString::number(*it));
    }

    QString visibleColumnsString;
    foreach(uint visibleColumn, d->visibleColumns) {
        if (!visibleColumnsString.isEmpty())
            visibleColumnsString.append(";");
        visibleColumnsString.append(QString::number(visibleColumn));
    }

    return QString("LookupFieldSchema( field:'%1 %2"
                   " boundColumn:%3 visibleColumns:%4 maximumListRows:%5 displayWidget:%6"
                   " columnHeadersVisible:%7 limitToList:%8"
                   " columnWidths:%9 )")
           .arg(fieldName)
           .arg(d->rowSource.debugString())
           .arg(d->boundColumn).arg(visibleColumnsString).arg(d->maximumListRows)
           .arg(d->displayWidget == ComboBox ? "ComboBox" : "ListBox")
           .arg(d->columnHeadersVisible).arg(d->limitToList)
           .arg(columnWidthsStr);
}

void LookupFieldSchema::debug(const QString &fieldName) const
{
    KexiDBDbg << debugString(fieldName);
}

static bool setBoundColumn(LookupFieldSchema *lookup, const QVariant &val)
{
    if (val.isNull()) {
        lookup->setBoundColumn(-1);
    }
    else {
        bool ok;
        const int ival = val.toInt(&ok);
        if (!ok)
            return false;
        lookup->setBoundColumn(ival);
    }
    return true;
}

static bool setVisibleColumns(LookupFieldSchema *lookup, const QVariant &val)
{
    QList<QVariant> variantList;
    if (val.canConvert(QVariant::Int)) {
    //! @todo Remove this case: it's for backward compatibility with Kexi's 1.1.2 table designer GUI
    //!       supporting only single lookup column.
        variantList.append(val);
    }
    else {
        variantList = val.toList();
    }
    QList<uint> visibleColumns;
    foreach(const QVariant& variant, variantList) {
        bool ok;
        const uint ival = variant.toUInt(&ok);
        if (!ok) {
            return false;
        }
        visibleColumns.append(ival);
    }
    lookup->setVisibleColumns(visibleColumns);
    return true;
}

static bool setColumnWidths(LookupFieldSchema *lookup, const QVariant &val)
{
    QList<int> widths;
    foreach(const QVariant& variant, val.toList()) {
        bool ok;
        const uint ival = variant.toInt(&ok);
        if (!ok)
            return false;
        widths.append(ival);
    }
    lookup->setColumnWidths(widths);
    return true;
}

static bool setDisplayWidget(LookupFieldSchema *lookup, const QVariant &val)
{
    bool ok;
    const uint ival = val.toUInt(&ok);
    if (!ok || ival > LookupFieldSchema::ListBox)
        return false;
    lookup->setDisplayWidget(static_cast<LookupFieldSchema::DisplayWidget>(ival));
    return true;
}

/* static */
LookupFieldSchema *LookupFieldSchema::loadFromDom(const QDomElement& lookupEl)
{
    LookupFieldSchema *lookupFieldSchema = new LookupFieldSchema();
    for (QDomNode node = lookupEl.firstChild(); !node.isNull(); node = node.nextSibling()) {
        QDomElement el = node.toElement();
        QString name(el.tagName());
        if (name == "row-source") {
            /*<row-source>
              empty
              | <type>table|query|sql|valuelist|fieldlist</type>  #required because there can be table and query with the same name
                        "fieldlist" (basically a list of column names of a table/query,
                              "Field List" as in MSA)
              <name>string</name> #table/query name, etc. or KEXISQL SELECT QUERY
              <values><value>...</value> #for "valuelist" type
                <value>...</value>
              </values>
             </row-source> */
            for (el = el.firstChild().toElement(); !el.isNull(); el = el.nextSibling().toElement()) {
                if (el.tagName() == "type")
                    lookupFieldSchema->rowSource().setTypeByName(el.text());
                else if (el.tagName() == "name")
                    lookupFieldSchema->rowSource().setName(el.text());
//! @todo handle fieldlist (retrieve from external table or so?), use lookupFieldSchema.rowSource().setValues()
            }
        } else if (name == "bound-column") {
            /* <bound-column>
                <number>number</number> #in later implementation there can be more columns
               </bound-column> */
            bool ok;
            const QVariant val = KexiDB::loadPropertyValueFromDom(el.firstChild(), &ok);
            if (!ok || !::setBoundColumn(lookupFieldSchema, val)) {
                delete lookupFieldSchema;
                return 0;
            }
        } else if (name == "visible-column") {
            /* <visible-column> #a column that has to be visible in the combo box
              <number>number 1</number>
              <number>number 2</number>
              [..]
               </visible-column> */
            QVariantList list;
            for (QDomNode childNode = el.firstChild(); !childNode.isNull(); childNode = childNode.nextSibling()) {
                bool ok;
                const QVariant val = KexiDB::loadPropertyValueFromDom(childNode, &ok);
                if (!ok) {
                    delete lookupFieldSchema;
                    return 0;
                }
                list.append(val);
            }
            if (!::setVisibleColumns(lookupFieldSchema, list)) {
                delete lookupFieldSchema;
                return 0;
            }
        } else if (name == "column-widths") {
            /* <column-widths> #column widths, -1 means 'default'
                <number>int</number>
                ...
                <number>int</number>
               </column-widths> */
            QVariantList columnWidths;
            for (el = el.firstChild().toElement(); !el.isNull(); el = el.nextSibling().toElement()) {
                bool ok;
                QVariant val = KexiDB::loadPropertyValueFromDom(el, &ok);
                if (!ok) {
                    delete lookupFieldSchema;
                    return 0;
                }
                columnWidths.append(val);
            }
            if (!::setColumnWidths(lookupFieldSchema, columnWidths)) {
                delete lookupFieldSchema;
                return 0;
            }
        } else if (name == "show-column-headers") {
            /* <show-column-headers>
                <bool>true/false</bool>
               </show-column-headers> */
            bool ok;
            const QVariant val = KexiDB::loadPropertyValueFromDom(el.firstChild(), &ok);
            if (!ok) {
                delete lookupFieldSchema;
                return 0;
            }
            if (val.type() == QVariant::Bool)
                lookupFieldSchema->setColumnHeadersVisible(val.toBool());
        } else if (name == "list-rows") {
            /* <list-rows>
                <number>1..100</number>
               </list-rows> */
            bool ok;
            const QVariant val = KexiDB::loadPropertyValueFromDom(el.firstChild(), &ok);
            if (!ok) {
                delete lookupFieldSchema;
                return 0;
            }
            if (val.type() == QVariant::Int)
                lookupFieldSchema->setMaximumListRows(val.toUInt());
        } else if (name == "limit-to-list") {
            /* <limit-to-list>
                <bool>true/false</bool>
               </limit-to-list> */
            bool ok;
            const QVariant val = KexiDB::loadPropertyValueFromDom(el.firstChild(), &ok);
            if (!ok) {
                delete lookupFieldSchema;
                return 0;
            }
            if (val.type() == QVariant::Bool)
                lookupFieldSchema->setLimitToList(val.toBool());
        } else if (name == "display-widget") {
            if (el.text() == "combobox")
                lookupFieldSchema->setDisplayWidget(LookupFieldSchema::ComboBox);
            else if (el.text() == "listbox")
                lookupFieldSchema->setDisplayWidget(LookupFieldSchema::ListBox);
        }
    }
    return lookupFieldSchema;
}

void LookupFieldSchema::saveToDom(QDomDocument *doc, QDomElement *parentEl) const
{
    QDomElement lookupColumnEl, rowSourceEl, rowSourceTypeEl, nameEl;
    if (!rowSource().name().isEmpty()) {
        lookupColumnEl = doc->createElement("lookup-column");
        parentEl->appendChild(lookupColumnEl);

        rowSourceEl = doc->createElement("row-source");
        lookupColumnEl.appendChild(rowSourceEl);

        rowSourceTypeEl = doc->createElement("type");
        rowSourceEl.appendChild(rowSourceTypeEl);
        rowSourceTypeEl.appendChild(doc->createTextNode(rowSource().typeName()));   //can be empty

        nameEl = doc->createElement("name");
        rowSourceEl.appendChild(nameEl);
        nameEl.appendChild(doc->createTextNode(rowSource().name()));
    }

    const QStringList values(rowSource().values());
    if (!values.isEmpty()) {
        QDomElement valuesEl(doc->createElement("values"));
        rowSourceEl.appendChild(valuesEl);
        for (QStringList::ConstIterator it = values.constBegin(); it != values.constEnd(); ++it) {
            QDomElement valueEl(doc->createElement("value"));
            valuesEl.appendChild(valueEl);
            valueEl.appendChild(doc->createTextNode(*it));
        }
    }

    if (boundColumn() >= 0)
        KexiDB::saveNumberElementToDom(*doc, lookupColumnEl, "bound-column", boundColumn());

    QList<uint> visibleColumns(this->visibleColumns());
    if (!visibleColumns.isEmpty()) {
        QDomElement visibleColumnEl(doc->createElement("visible-column"));
        lookupColumnEl.appendChild(visibleColumnEl);
        foreach(uint visibleColumn, visibleColumns) {
            QDomElement numberEl(doc->createElement("number"));
            visibleColumnEl.appendChild(numberEl);
            numberEl.appendChild(doc->createTextNode(QString::number(visibleColumn)));
        }
    }

    const QList<int> columnWidths(this->columnWidths());
    if (!columnWidths.isEmpty()) {
        QDomElement columnWidthsEl(doc->createElement("column-widths"));
        lookupColumnEl.appendChild(columnWidthsEl);
        foreach(int columnWidth, columnWidths) {
            QDomElement columnWidthEl(doc->createElement("number"));
            columnWidthsEl.appendChild(columnWidthEl);
            columnWidthEl.appendChild(doc->createTextNode(QString::number(columnWidth)));
        }
    }

    if (columnHeadersVisible() != KEXIDB_LOOKUP_FIELD_DEFAULT_HEADERS_VISIBLE)
        KexiDB::saveBooleanElementToDom(*doc, lookupColumnEl, "show-column-headers", columnHeadersVisible());
    if (maximumListRows() != KEXIDB_LOOKUP_FIELD_DEFAULT_LIST_ROWS)
        KexiDB::saveNumberElementToDom(*doc, lookupColumnEl, "list-rows", maximumListRows());
    if (limitToList() != KEXIDB_LOOKUP_FIELD_DEFAULT_LIMIT_TO_LIST)
        KexiDB::saveBooleanElementToDom(*doc, lookupColumnEl, "limit-to-list", limitToList());

    if (displayWidget() != KEXIDB_LOOKUP_FIELD_DEFAULT_DISPLAY_WIDGET) {
        QDomElement displayWidgetEl(doc->createElement("display-widget"));
        lookupColumnEl.appendChild(displayWidgetEl);
        displayWidgetEl.appendChild(
            doc->createTextNode((displayWidget() == ListBox) ? "listbox" : "combobox"));
    }
}

namespace KexiDB {
void getProperties(const LookupFieldSchema *lookup, QMap<QByteArray, QVariant> *values);
}

void LookupFieldSchema::getProperties(QMap<QByteArray, QVariant> *values) const
{
    values->clear();
    KexiDB::getProperties(this, values);
}

bool LookupFieldSchema::setProperty(const QByteArray& propertyName, const QVariant& value)
{
    bool ok;
    if (   "rowSource" == propertyName
        || "rowSourceType" == propertyName
        || "rowSourceValues" == propertyName)
    {
        LookupFieldSchema::RowSource rowSource(this->rowSource());
        if ("rowSource" == propertyName)
            rowSource.setName(value.toString());
        else if ("rowSourceType" == propertyName)
            rowSource.setTypeByName(value.toString());
        else if ("rowSourceValues" == propertyName) {
            if (value.isNull()) {
                return true;
            }
            rowSource.setValues(value.toStringList());
        }
        setRowSource(rowSource);
    } else if ("boundColumn" == propertyName) {
        if (!::setBoundColumn(this, value)) {
            return false;
        }
    } else if ("visibleColumn" == propertyName) {
        if (!::setVisibleColumns(this, value)) {
            return false;
        }
    } else if ("columnWidths" == propertyName) {
        if (!::setColumnWidths(this, value)) {
            return false;
        }
    } else if ("showColumnHeaders" == propertyName) {
        setColumnHeadersVisible(value.toBool());
    } else if ("listRows" == propertyName) {
        const uint ival = value.toUInt(&ok);
        if (!ok)
            return false;
        setMaximumListRows(ival);
    } else if ("limitToList" == propertyName) {
        setLimitToList(value.toBool());
    } else if ("displayWidget" == propertyName) {
        if (!::setDisplayWidget(this, value)) {
            return false;
        }
    }
    return true;
}

bool LookupFieldSchema::setProperties(const QMap<QByteArray, QVariant>& values)
{
    QMap<QByteArray, QVariant>::ConstIterator it;
    LookupFieldSchema::RowSource rowSource(this->rowSource());
    bool ok;
    bool updateRowSource = false;
    if ((it = values.find("rowSource")) != values.constEnd()) {
        rowSource.setName(it.value().toString());
        updateRowSource = true;
    }
    if ((it = values.find("rowSourceType")) != values.constEnd()) {
        rowSource.setTypeByName(it.value().toString());
        updateRowSource = true;
    }
    if ((it = values.find("rowSourceValues")) != values.constEnd()) {
        if (!it.value().isNull()) {
            rowSource.setValues(it.value().toStringList());
            updateRowSource = true;
        }
    }
    if (updateRowSource) {
        setRowSource(rowSource);
    }
    if ((it = values.find("boundColumn")) != values.constEnd()) {
        if (!::setBoundColumn(this, it.value())) {
            return false;
        }
    }
    if ((it = values.find("visibleColumn")) != values.constEnd()) {
        if (!::setVisibleColumns(this, it.value())) {
            return false;
        }
    }
    if ((it = values.find("columnWidths")) != values.constEnd()) {
        if (!::setColumnWidths(this, it.value())) {
            return false;
        }
    }
    if ((it = values.find("showColumnHeaders")) != values.constEnd()) {
        setColumnHeadersVisible(it.value().toBool());
    }
    if ((it = values.find("listRows")) != values.constEnd()) {
        int ival = it.value().toInt(&ok);
        if (!ok)
            return false;
        setMaximumListRows(ival);
    }
    if ((it = values.find("limitToList")) != values.constEnd()) {
        setLimitToList(it.value().toBool());
    }
    if ((it = values.find("displayWidget")) != values.constEnd()) {
        if (!::setDisplayWidget(this, it.value())) {
            return false;
        }
    }
    return true;
}

int LookupFieldSchema::boundColumn() const
{
    return d->boundColumn;
}

void LookupFieldSchema::setBoundColumn(int column)
{
    d->boundColumn = column >= 0 ? column : -1;
}

QList<uint> LookupFieldSchema::visibleColumns() const
{
    return d->visibleColumns;
}

void LookupFieldSchema::setVisibleColumns(const QList<uint>& list)
{
    d->visibleColumns = list;
}

int LookupFieldSchema::visibleColumn(uint fieldsCount) const
{
    if (d->visibleColumns.count() == 1)
        return (d->visibleColumns.first() < fieldsCount) ? (int)d->visibleColumns.first() : -1;
    if (d->visibleColumns.isEmpty())
        return -1;
    return fieldsCount - 1;
}

QList<int> LookupFieldSchema::columnWidths() const
{
    return d->columnWidths;
}

void LookupFieldSchema::setColumnWidths(const QList<int>& widths)
{
    d->columnWidths = widths;
}

bool LookupFieldSchema::columnHeadersVisible() const
{
    return d->columnHeadersVisible;
}

void LookupFieldSchema::setColumnHeadersVisible(bool set)
{
    d->columnHeadersVisible = set;
}

uint LookupFieldSchema::maximumListRows() const
{
    return d->maximumListRows;
}

bool LookupFieldSchema::limitToList() const
{
    return d->limitToList;
}

void LookupFieldSchema::setLimitToList(bool set)
{
    d->limitToList = set;
}

LookupFieldSchema::DisplayWidget LookupFieldSchema::displayWidget() const
{
    return d->displayWidget;
}

void LookupFieldSchema::setDisplayWidget(DisplayWidget widget)
{
    d->displayWidget = widget;
}
