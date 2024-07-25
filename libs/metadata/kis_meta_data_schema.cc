/*
 *  SPDX-FileCopyrightText: 2007, 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "kis_meta_data_schema.h"

#include <QDateTime>
#include <QDomDocument>
#include <QFile>
#include <QString>
#include <QVariant>

#include "kis_meta_data_type_info_p.h"
#include "kis_meta_data_schema_p.h"
#include "kis_meta_data_value.h"

using namespace KisMetaData;

const QString Schema::TIFFSchemaUri = "http://ns.adobe.com/tiff/1.0/";
const QString Schema::EXIFSchemaUri = "http://ns.adobe.com/exif/1.0/";
const QString Schema::DublinCoreSchemaUri = "http://purl.org/dc/elements/1.1/";
const QString Schema::XMPSchemaUri = "http://ns.adobe.com/xap/1.0/";
const QString Schema::XMPRightsSchemaUri = "http://ns.adobe.com/xap/1.0/rights/";
const QString Schema::XMPMediaManagementUri = "http://ns.adobe.com/xap/1.0/sType/ResourceRef#";
const QString Schema::MakerNoteSchemaUri = "http://www.calligra.org/krita/xmp/MakerNote/1.0/";
const QString Schema::IPTCSchemaUri = "http://iptc.org/std/Iptc4xmpCore/1.0/xmlns/";
const QString Schema::PhotoshopSchemaUri = "http://ns.adobe.com/photoshop/1.0/";

bool Schema::Private::load(const QString& _fileName)
{
    dbgMetaData << "Loading from " << _fileName;

    QDomDocument document;
    QString error;
    int line, column;
    QFile file(_fileName);
    if (!document.setContent(&file, &error, &line, &column)) {
        dbgMetaData << error << " at " << line << ", " << column << " in " << _fileName;
        return false;
    }

    QDomElement docElem = document.documentElement();
    if (docElem.tagName() != "schema") {
        dbgMetaData << _fileName << ": invalid root name";
        return false;
    }

    if (!docElem.hasAttribute("prefix")) {
        dbgMetaData << _fileName << ": missing prefix.";
        return false;
    }

    if (!docElem.hasAttribute("uri")) {
        dbgMetaData << _fileName << ": missing uri.";
        return false;
    }

    prefix = docElem.attribute("prefix");
    uri = docElem.attribute("uri");
    dbgMetaData << ppVar(prefix) << ppVar(uri);

    QDomElement structuresElt = docElem.firstChildElement("structures");
    if (structuresElt.isNull()) {
        return false;
    }

    QDomElement propertiesElt = docElem.firstChildElement("properties");
    if (propertiesElt.isNull()) {
        return false;
    }

    parseStructures(structuresElt);
    parseProperties(propertiesElt);

    return true;
}

void Schema::Private::parseStructures(QDomElement& elt)
{
    Q_ASSERT(elt.tagName() == "structures");
    dbgMetaData << "Parse structures";

    QDomElement e = elt.firstChildElement();
    for (; !e.isNull(); e = e.nextSiblingElement()) {
        if (e.tagName() == "structure") {
            parseStructure(e);
        } else {
            errMetaData << "Invalid tag: " << e.tagName() << " in structures section";
        }
    }
}

void Schema::Private::parseStructure(QDomElement& elt)
{
    Q_ASSERT(elt.tagName() == "structure");

    if (!elt.hasAttribute("name")) {
        errMetaData << "Name is required for a structure";
        return;
    }

    QString structureName = elt.attribute("name");
    if (structures.contains(structureName)) {
        errMetaData << structureName << " is defined twice";
        return;
    }
    dbgMetaData << "Parsing structure " << structureName;

    if (!elt.hasAttribute("prefix")) {
        errMetaData << "prefix is required for structure " << structureName;
        return;
    }

    if (!elt.hasAttribute("uri")) {
        errMetaData << "uri is required for structure " << structureName;
        return;
    }

    QString structurePrefix = elt.attribute("prefix");
    QString structureUri = elt.attribute("uri");
    dbgMetaData << ppVar(structurePrefix) << ppVar(structureUri);

    Schema* schema = new Schema(structureUri, structurePrefix);
    QDomElement e;
    for (e = elt.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
        EntryInfo info;
        QString name;

        if (!parseEltType(e, info, name, false, false)) {
            continue;
        }

        if (schema->d->types.contains(name)) {
            errMetaData << structureName << " already contains a field " << name;
            continue;
        }

        schema->d->types[ name ] = info;
    }

    structures[ structureName ] = TypeInfo::Private::createStructure(schema, structureName);
}

void Schema::Private::parseProperties(QDomElement& elt)
{
    Q_ASSERT(elt.tagName() == "properties");
    dbgMetaData << "Parse properties";

    QDomElement e;
    for (e = elt.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
        EntryInfo info;
        QString name;

        if (!parseEltType(e, info, name, false, false)) {
            continue;
        }

        if (types.contains(name)) {
            errMetaData << name << " already defined.";
            continue;
        }

        types[ name ] = info;
    }
}

bool Schema::Private::parseEltType(QDomElement &elt,
                                   EntryInfo &entryInfo,
                                   QString &name,
                                   bool ignoreStructure,
                                   bool ignoreName)
{
    dbgMetaData << elt.tagName() << elt.attributes().count() << name << ignoreStructure << ignoreName;

    QString tagName = elt.tagName();
    if (!ignoreName && !elt.hasAttribute("name")) {
        errMetaData << "Missing name attribute for tag " << tagName;
        return false;
    }
    name = elt.attribute("name");

    // TODO parse qualifier
    if (tagName == "integer") {
        entryInfo.propertyType = TypeInfo::Private::Integer;
    } else if (tagName == "boolean") {
        entryInfo.propertyType = TypeInfo::Private::Boolean;
    } else if (tagName == "date") {
        entryInfo.propertyType = TypeInfo::Private::Date;
    } else if (tagName == "text") {
        entryInfo.propertyType = TypeInfo::Private::Text;
    } else if (tagName == "seq") {
        const TypeInfo* ei = parseAttType(elt, ignoreStructure);
        if (!ei) {
            ei = parseEmbType(elt, ignoreStructure);
        }

        if (!ei) {
            errMetaData << "No type defined for " << name;
            return false;
        }

        entryInfo.propertyType = TypeInfo::Private::orderedArray(ei);
    } else if (tagName == "bag") {
        const TypeInfo* ei = parseAttType(elt, ignoreStructure);
        if (!ei) {
            ei = parseEmbType(elt, ignoreStructure);
        }

        if (!ei) {
            errMetaData << "No type defined for " << name;
            return false;
        }

        entryInfo.propertyType = TypeInfo::Private::unorderedArray(ei);
    } else if (tagName == "alt") {
        const TypeInfo* ei = parseAttType(elt, ignoreStructure);
        if (!ei) {
            ei = parseEmbType(elt, ignoreStructure);
        }

        if (!ei) {
            errMetaData << "No type defined for " << name;
            return false;
        }

        entryInfo.propertyType = TypeInfo::Private::alternativeArray(ei);
    } else if (tagName == "lang") {
        entryInfo.propertyType = TypeInfo::Private::LangArray;
    } else if (tagName == "rational") {
        entryInfo.propertyType = TypeInfo::Private::Rational;
    } else if (tagName == "gpscoordinate") {
        entryInfo.propertyType = TypeInfo::Private::GPSCoordinate;
    } else if (tagName == "openedchoice" || tagName == "closedchoice") {
        entryInfo.propertyType = parseChoice(elt);
    } else if (!ignoreStructure && structures.contains(tagName)) {
        entryInfo.propertyType = structures.value(tagName);
    } else {
        errMetaData << tagName << " isn't a type.";
        return false;
    }

    return true;
}

const TypeInfo* Schema::Private::parseAttType(QDomElement& elt, bool ignoreStructure)
{
    if (!elt.hasAttribute("type")) {
        return 0;
    }

    QString type = elt.attribute("type");
    if (type == "integer") {
        return TypeInfo::Private::Integer;
    } else if (type == "boolean") {
        return TypeInfo::Private::Boolean;
    } else if (type == "date") {
        return TypeInfo::Private::Date;
    } else if (type == "text") {
        return TypeInfo::Private::Text;
    } else if (type == "rational") {
        return TypeInfo::Private::Rational;
    } else if (!ignoreStructure && structures.contains(type)) {
        return structures[type];
    }

    errMetaData << "Unsupported type: " << type << " in an attribute";
    return nullptr;
}

const TypeInfo* Schema::Private::parseEmbType(QDomElement& elt, bool ignoreStructure)
{
    dbgMetaData << "Parse embedded type for " << elt.tagName();

    QDomElement e;
    for (e = elt.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
        QString type = e.tagName();
        if (type == "integer") {
            return TypeInfo::Private::Integer;
        } else if (type == "boolean") {
            return TypeInfo::Private::Boolean;
        } else if (type == "date") {
            return TypeInfo::Private::Date;
        } else if (type == "text") {
            return TypeInfo::Private::Text;
        } else if (type == "openedchoice" || type == "closedchoice") {
            return parseChoice(e);
        } else if (!ignoreStructure && structures.contains(type)) {
            return structures[type];
        }
    }

    return nullptr;
}

const TypeInfo* Schema::Private::parseChoice(QDomElement& elt)
{
    const TypeInfo* choiceType = parseAttType(elt, true);
    TypeInfo::PropertyType propertyType;
    if (elt.tagName() == "openedchoice") {
        propertyType = TypeInfo::OpenedChoice;
    } else {
        Q_ASSERT(elt.tagName() == "closedchoice");
        propertyType = TypeInfo::ClosedChoice;
    }

    QDomElement e;
    QList<TypeInfo::Choice> choices;
    for (e = elt.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
        EntryInfo info;
        QString name;

        if (!parseEltType(e, info, name, true, true)) {
            continue;
        }

        if (!choiceType) {
            choiceType = info.propertyType;
        }

        if (choiceType != info.propertyType) {
            errMetaData << "All members of a choice need to be of the same type";
            continue;
        }

        QString text = e.text();
        QVariant var = text;

        if (choiceType->propertyType() == TypeInfo::IntegerType) {
            var = var.toInt();
        } else if (choiceType->propertyType() == TypeInfo::DateType) {
            // TODO: QVariant date parser isn't very good with XMP date
            // (it doesn't support YYYY and YYYY-MM)
            var = var.toDateTime();
        }
        choices.push_back(TypeInfo::Choice(Value(var), name));
    }

    return TypeInfo::Private::createChoice(propertyType, choiceType, choices);
}

Schema::Schema()
        : d(new Private)
{
}

Schema::Schema(const QString & _uri, const QString & _ns)
        : d(new Private)
{
    d->uri = _uri;
    d->prefix = _ns;
}

Schema::~Schema()
{
    dbgMetaData << "Deleting schema " << d->uri << " " << d->prefix;
    dbgMetaData.noquote() << kisBacktrace();
    delete d;
}

const TypeInfo* Schema::propertyType(const QString& _propertyName) const
{
    if (d->types.contains(_propertyName)) {
        return d->types.value(_propertyName).propertyType;
    }
    return 0;
}

const TypeInfo* Schema::structure(const QString& _structureName) const
{
    return d->structures.value(_structureName);
}


QString Schema::uri() const
{
    return d->uri;
}

QString Schema::prefix() const
{
    return d->prefix;
}

QString Schema::generateQualifiedName(const QString & name) const
{
    dbgMetaData << "generateQualifiedName for " << name;
    Q_ASSERT(!name.isEmpty() && !name.isNull());
    return prefix() + ':' + name;
}

QDebug operator<<(QDebug debug, const KisMetaData::Schema &c)
{
    debug.nospace() << "Uri = " << c.uri() << " Prefix = " << c.prefix();
    return debug.space();
}
