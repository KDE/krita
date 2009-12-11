/*
 *  Copyright (c) 2007,2009 Cyrille Berger <cberger@cberger.net>
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

#include "kis_meta_data_schema.h"

#include <QDateTime>
#include <QDomDocument>
#include <QFile>
#include <QString>
#include <QVariant>

#include "kis_debug.h"
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
const QString Schema::MakerNoteSchemaUri = "http://www.koffice.org/krita/xmp/MakerNote/1.0/";
const QString Schema::IPTCSchemaUri = "http://iptc.org/std/Iptc4xmpCore/1.0/xmlns/";
const QString Schema::PhotoshopSchemaUri = "http://ns.adobe.com/photoshop/1.0/";

bool Schema::Private::load(const QString& _fileName)
{
    dbgImage << "Loading from " << _fileName;
    QDomDocument document;
    QString error;
    int ligne, column;
    QFile file(_fileName);
    if (document.setContent(&file, &error, &ligne, &column)) {
        QDomElement docElem = document.documentElement();
        if (docElem.tagName() != "schema") {
            dbgImage << _fileName << ": invalid root name";
            return false;
        }
        if (!docElem.hasAttribute("prefix")) {
            dbgImage << _fileName << ": missing prefix.";
            return false;
        }
        if (!docElem.hasAttribute("uri")) {
            dbgImage << _fileName << ": missing uri.";
            return false;
        }
        prefix = docElem.attribute("prefix");
        uri = docElem.attribute("uri");
        dbgImage << ppVar(prefix) << ppVar(uri);
        QDomNode n = docElem.firstChild();
        while (!n.isNull()) {
            QDomElement e = n.toElement();
            if (!e.isNull()) {
                if (e.tagName() == "structures") {
                    parseStructures(e);
                } else if (e.tagName() == "properties") {
                    parseProperties(e);
                }
            }
            n = n.nextSibling();
        }
        return true;
    } else {
        dbgImage << error << " at " << ligne << ", " << column << " in " << _fileName;
        return false;
    }
}

void Schema::Private::parseStructures(QDomElement& elt)
{
    Q_ASSERT(elt.tagName() == "structures");
    dbgImage << "Parse sturctures";
    QDomNode n = elt.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (!e.isNull()) {
            if (e.tagName() == "structure") {
                parseStructure(e);
            } else {
                errImage << "Invalid tag: " << e.tagName() << " in structures section";
            }
        }
        n = n.nextSibling();
    }
}

void Schema::Private::parseStructure(QDomElement& elt)
{
    Q_ASSERT(elt.tagName() == "structure");
    if (!elt.hasAttribute("name")) {
        errImage << "Name is required for a structure";
        return;
    }
    QString structureName = elt.attribute("name");
    if (structures.contains(structureName)) {
        errImage << structureName << " is defined twice";
        return;
    }
    dbgImage << "Parsing structure " << structureName;
    if (!elt.hasAttribute("prefix")) {
        errImage << "prefix is required for structure " << structureName;
        return;
    }
    if (!elt.hasAttribute("uri")) {
        errImage << "uri is required for structure " << structureName;
        return;
    }
    QString structurePrefix = elt.attribute("prefix");
    QString structureUri = elt.attribute("uri");
    dbgImage << ppVar(structurePrefix) << ppVar(structureUri);
    Schema* schema = new Schema(structureUri, structurePrefix);
    QDomNode n = elt.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (!e.isNull()) {
            EntryInfo info;
            QString name;
            if (parseEltType(e, info, name, true, false)) {
                if (schema->d->types.contains(name)) {
                    errImage << structureName << " already contains a field " << name;
                } else {
                    schema->d->types[ name ] = info;
                }
            }
        }
        n = n.nextSibling();
    }
    structures[ structureName ] = TypeInfo::Private::createStructure(schema, structureName);
}

void Schema::Private::parseProperties(QDomElement& elt)
{
    Q_ASSERT(elt.tagName() == "properties");
    dbgImage << "Parse properties";
    QDomNode n = elt.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (!e.isNull()) {
            EntryInfo info;
            QString name;
            if (parseEltType(e, info, name, false, false)) {
                if (types.contains(name)) {
                    errImage << name << " already defined.";
                } else {
                    types[ name ] = info;
                }
            }
        }
        n = n.nextSibling();
    }
}

bool Schema::Private::parseEltType(QDomElement& elt, EntryInfo& entryInfo, QString& name, bool ignoreStructure, bool ignoreName)
{
    QString tagName = elt.tagName();
    if (!ignoreName && !elt.hasAttribute("name")) {
        errImage << "Missing name attribute for tag " << tagName;
        return false;
    }
    name = elt.attribute("name");
    // TODO parse qualifier
    if (tagName == "integer") {
        entryInfo.propertyType = TypeInfo::Private::Integer;
        return true;
    } else if (tagName == "boolean") {
        entryInfo.propertyType = TypeInfo::Private::Boolean;
        return true;
    } else if (tagName == "date") {
        entryInfo.propertyType = TypeInfo::Private::Date;
        return true;
    } else if (tagName == "text") {
        entryInfo.propertyType = TypeInfo::Private::Text;
        return true;
    } else if (tagName == "seq") {
        const TypeInfo* ei = parseAttType(elt, ignoreStructure);
        if (!ei) {
            ei = parseEmbType(elt, ignoreStructure);
        }
        if (!ei) {
            errImage << "No type defined for " << name;
            return false;
        }
        entryInfo.propertyType = TypeInfo::Private::orderedArray(ei);
        return true;
    } else if (tagName == "bag") {
        const TypeInfo* ei = parseAttType(elt, ignoreStructure);
        if (!ei) {
            ei = parseEmbType(elt, ignoreStructure);
        }
        if (!ei) {
            errImage << "No type defined for " << name;
            return false;
        }
        entryInfo.propertyType = TypeInfo::Private::unorderedArray(ei);
        return true;
    } else if (tagName == "alt") {
        const TypeInfo* ei = parseAttType(elt, ignoreStructure);
        if (!ei) {
            ei = parseEmbType(elt, ignoreStructure);
        }
        if (!ei) {
            errImage << "No type defined for " << name;
            return false;
        }
        entryInfo.propertyType = TypeInfo::Private::alternativeArray(ei);
        return true;
    } else if (tagName == "lang") {
        entryInfo.propertyType = TypeInfo::Private::LangArray;
        return true;
    } else if (tagName == "rational") {
        entryInfo.propertyType = TypeInfo::Private::Rational;
        return true;
    } else if (tagName == "gpscoordinate") {
        entryInfo.propertyType = TypeInfo::Private::GPSCoordinate;
        return true;
    } else if (tagName == "openedchoice" || tagName == "closedchoice") {
        entryInfo.propertyType = parseChoice(elt);
        return true;
    } else if (!ignoreStructure && structures.contains(tagName)) {
        entryInfo.propertyType = structures.value(tagName);
        return true;
    }
    errImage << tagName << " isn't a type.";
    return false;
}

const TypeInfo* Schema::Private::parseAttType(QDomElement& elt, bool ignoreStructure)
{
    Q_UNUSED(ignoreStructure);
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
    }
    errImage << "Unsupported type: " << type << " in an attribute";
    return 0;
}

const TypeInfo* Schema::Private::parseEmbType(QDomElement& elt, bool ignoreStructure)
{
    Q_UNUSED(ignoreStructure);
    dbgImage << "Parse embbedded type for " << elt.tagName();
    QDomNode n = elt.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (!e.isNull()) {
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
            }
        }
        n = n.nextSibling();
    }
    return 0;
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
    QDomNode n = elt.firstChild();
    QList< TypeInfo::Choice > choices;
    while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (!e.isNull()) {
            EntryInfo info;
            QString name;
            if (parseEltType(e, info, name, true, true)) {
                if (! choiceType) choiceType = info.propertyType;
                if (choiceType == info.propertyType) {
                    QString text = e.text();
                    QVariant var = text;
                    if (choiceType->propertyType() == TypeInfo::IntegerType) {
                        var = var.toInt();
                    } else if (choiceType->propertyType() == TypeInfo::DateType) { // TODO QVariant date parser isn't very good with XMP date (it doesn't support YYYY and YYYY-MM
                        var = var.toDateTime();
                    }
                    choices.push_back(TypeInfo::Choice(Value(var), name));
                } else {
                    errImage << "All members of a choice need to be of the same type";
                }
            }
        }
        n = n.nextSibling();
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
    dbgImage << "Deleting schema " << d->uri << " " << d->prefix;
    dbgImage << kBacktrace();
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
    dbgImage << "generateQualifiedName for " << name;
    Q_ASSERT(!name.isEmpty() && !name.isNull());
    return prefix() + ':' + name;
}

QDebug operator<<(QDebug debug, const KisMetaData::Schema &c)
{
    debug.nospace() << "Uri = " << c.uri() << " Prefix = " << c.prefix();
    return debug.space();
}
