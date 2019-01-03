/*
 *  Copyright (c) 2008-2010 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
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
#include "kis_xmp_io.h"

#include <string>

#include "kis_exiv2.h"

#include <metadata/kis_meta_data_store.h>
#include <metadata/kis_meta_data_entry.h>
#include <metadata/kis_meta_data_parser.h>
#include <metadata/kis_meta_data_value.h>
#include <metadata/kis_meta_data_schema.h>
#include <metadata/kis_meta_data_schema_registry.h>
#include <metadata/kis_meta_data_type_info.h>

#include <kis_debug.h>

KisXMPIO::KisXMPIO()
{
}

KisXMPIO::~KisXMPIO()
{
}

inline std::string exiv2Prefix(const KisMetaData::Schema* _schema)
{
    const QByteArray latin1SchemaUri = _schema->uri().toLatin1();
    std::string prefix = Exiv2::XmpProperties::prefix(latin1SchemaUri.constData());
    if (prefix.empty()) {
        dbgMetaData << "Unknown namespace " << ppVar(_schema->uri()) << ppVar(_schema->prefix());
        prefix = _schema->prefix().toLatin1().constData();
        Exiv2::XmpProperties::registerNs(latin1SchemaUri.constData(), prefix);
    }
    return prefix;
}

namespace
{
void saveStructure(Exiv2::XmpData& xmpData_, const QString& name, const std::string& prefix, const QMap<QString, KisMetaData::Value>& structure, const KisMetaData::Schema* structureSchema)
{
    std::string structPrefix = exiv2Prefix(structureSchema);
    for (QMap<QString, KisMetaData::Value>::const_iterator it = structure.begin();
            it != structure.end(); ++it) {
        Q_ASSERT(it.value().type() != KisMetaData::Value::Structure);   // Can't nest structure
        QString key = QString("%1/%2:%3").arg(name).arg(structPrefix.c_str()).arg(it.key());
        Exiv2::XmpKey ekey(prefix, key.toLatin1().constData());
        dbgMetaData << ppVar(key) << ppVar(ekey.key().c_str());
        Exiv2::Value *v = kmdValueToExivXmpValue(it.value());
        if (v) {
            xmpData_.add(ekey, v);
        }
    }
}
}

bool KisXMPIO::saveTo(KisMetaData::Store* store, QIODevice* ioDevice, HeaderType headerType) const
{
    dbgMetaData << "Save XMP Data";
    Exiv2::XmpData xmpData_;

    for (QHash<QString, KisMetaData::Entry>::const_iterator it = store->begin();
            it != store->end(); ++it) {
        const KisMetaData::Entry& entry = *it;

        // Check whether the prefix and namespace are know to exiv2
        std::string prefix = exiv2Prefix(entry.schema());
        dbgMetaData << "Saving " << entry.name();

        const KisMetaData::Value& value = entry.value();

        const KisMetaData::TypeInfo* typeInfo = entry.schema()->propertyType(entry.name());
        if (value.type() == KisMetaData::Value::Structure) {
            QMap<QString, KisMetaData::Value> structure = value.asStructure();
            const KisMetaData::Schema* structureSchema = 0;
            if (typeInfo) {
                structureSchema = typeInfo->structureSchema();
            }
            if (!structureSchema) {
                dbgMetaData << "Unknown schema for " << entry.name();
                structureSchema = entry.schema();
            }
            Q_ASSERT(structureSchema);
            saveStructure(xmpData_, entry.name(), prefix, structure, structureSchema);
        } else {
            Exiv2::XmpKey key(prefix, entry.name().toLatin1().constData());
            if (typeInfo && (typeInfo->propertyType() == KisMetaData::TypeInfo::OrderedArrayType
                             || typeInfo->propertyType() == KisMetaData::TypeInfo::UnorderedArrayType
                             || typeInfo->propertyType() == KisMetaData::TypeInfo::AlternativeArrayType)
                    && typeInfo->embeddedPropertyType()->propertyType() == KisMetaData::TypeInfo::StructureType) {
                // Here is the bad part, again we need to do it by hand
                Exiv2::XmpTextValue tv;
                switch (typeInfo->propertyType()) {
                case KisMetaData::TypeInfo::OrderedArrayType:
                    tv.setXmpArrayType(Exiv2::XmpValue::xaSeq);
                    break;
                case KisMetaData::TypeInfo::UnorderedArrayType:
                    tv.setXmpArrayType(Exiv2::XmpValue::xaBag);
                    break;
                case KisMetaData::TypeInfo::AlternativeArrayType:
                    tv.setXmpArrayType(Exiv2::XmpValue::xaAlt);
                    break;
                default:
                    // Cannot happen
                    ;
                }
                xmpData_.add(key, &tv); // set the arrya type
                const KisMetaData::TypeInfo* stuctureTypeInfo = typeInfo->embeddedPropertyType();
                const KisMetaData::Schema* structureSchema = 0;
                if (stuctureTypeInfo) {
                    structureSchema = stuctureTypeInfo->structureSchema();
                }
                if (!structureSchema) {
                    dbgMetaData << "Unknown schema for " << entry.name();
                    structureSchema = entry.schema();
                }
                Q_ASSERT(structureSchema);
                QList<KisMetaData::Value> array = value.asArray();
                for (int idx = 0; idx < array.size(); ++idx) {
                    saveStructure(xmpData_, QString("%1[%2]").arg(entry.name()).arg(idx + 1), prefix, array[idx].asStructure(), structureSchema);
                }
            } else {
                dbgMetaData << ppVar(key.key().c_str());
                Exiv2::Value *v = kmdValueToExivXmpValue(value);
                if (v) {
                    xmpData_.add(key, v);
                }
            }
        }
        // TODO property qualifier
    }
    // Serialize data
    std::string xmpPacket_;
    Exiv2::XmpParser::encode(xmpPacket_, xmpData_);
    // Save data into the IO device
    ioDevice->open(QIODevice::WriteOnly);
    if (headerType == KisMetaData::IOBackend::JpegHeader) {
        xmpPacket_ = "http://ns.adobe.com/xap/1.0/\0" + xmpPacket_;
    }
    ioDevice->write(xmpPacket_.c_str(), xmpPacket_.length());
    return true;
}

bool parseTagName(const QString &tagString,
                  QString &structName,
                  int &arrayIndex,
                  QString &tagName,
                  const KisMetaData::TypeInfo** typeInfo,
                  const KisMetaData::Schema *schema)
{
    arrayIndex = -1;
    *typeInfo = 0;

    int numSubNames = tagString.count('/') + 1;

    if (numSubNames == 1) {
        structName.clear();
        tagName = tagString;
        *typeInfo = schema->propertyType(tagName);
        return true;
    }


    if (numSubNames == 2) {
        QRegExp regexp("([A-Za-z]\\w+)/([A-Za-z]\\w+):([A-Za-z]\\w+)");
        if (regexp.indexIn(tagString) != -1) {
            structName = regexp.capturedTexts()[1];
            tagName =  regexp.capturedTexts()[3];
            *typeInfo = schema->propertyType(structName);

            if (*typeInfo && (*typeInfo)->propertyType() == KisMetaData::TypeInfo::StructureType) {
                *typeInfo = (*typeInfo)->structureSchema()->propertyType(tagName);
            }

            return true;
        }

        QRegExp regexp2("([A-Za-z]\\w+)\\[(\\d+)\\]/([A-Za-z]\\w+):([A-Za-z]\\w+)");
        if (regexp2.indexIn(tagString) != -1) {
            structName = regexp2.capturedTexts()[1];
            arrayIndex = regexp2.capturedTexts()[2].toInt() - 1;
            tagName = regexp2.capturedTexts()[4];

            if (schema->propertyType(structName)) {
                *typeInfo = schema->propertyType(structName)->embeddedPropertyType();
                Q_ASSERT(*typeInfo);

                if ((*typeInfo)->propertyType() == KisMetaData::TypeInfo::StructureType) {
                    *typeInfo = (*typeInfo)->structureSchema()->propertyType(tagName);
                }
            }

            return true;
        }
    }

    warnKrita << "WARNING: Unsupported tag. We do not yet support nested tags. The tag will be dropped!";
    warnKrita << "         Failing tag:" << tagString;
    return false;
}

bool KisXMPIO::loadFrom(KisMetaData::Store* store, QIODevice* ioDevice) const
{
    ioDevice->open(QIODevice::ReadOnly);
    dbgMetaData << "Load XMP Data";
    std::string xmpPacket_;
    QByteArray arr = ioDevice->readAll();
    xmpPacket_.assign(arr.data(), arr.length());
    dbgMetaData << xmpPacket_.length();
//     dbgMetaData << xmpPacket_.c_str();
    Exiv2::XmpData xmpData_;
    Exiv2::XmpParser::decode(xmpData_, xmpPacket_);
    QMap< const KisMetaData::Schema*, QMap<QString, QMap<QString, KisMetaData::Value> > > structures;
    QMap< const KisMetaData::Schema*, QMap<QString, QVector< QMap<QString, KisMetaData::Value> > > > arraysOfStructures;
    for (Exiv2::XmpData::iterator it = xmpData_.begin(); it != xmpData_.end(); ++it) {
        dbgMetaData << "Start iteration" << it->key().c_str();

        Exiv2::XmpKey key(it->key());
        dbgMetaData << key.groupName().c_str() << " " << key.tagName().c_str() << " " << key.ns().c_str();
        if ((key.groupName() == "exif" || key.groupName() == "tiff") && key.tagName() == "NativeDigest") {  // TODO: someone who has time to lose can look in adding support for NativeDigest, it's undocumented use by the XMP SDK to check if exif data has been changed while XMP hasn't been updated
            dbgMetaData << "dropped";
        } else {
            const KisMetaData::Schema* schema = KisMetaData::SchemaRegistry::instance()->schemaFromPrefix(key.groupName().c_str());
            if (!schema) {
                schema = KisMetaData::SchemaRegistry::instance()->schemaFromUri(key.ns().c_str());
                if (!schema) {
                    schema = KisMetaData::SchemaRegistry::instance()->create(key.ns().c_str(), key.groupName().c_str());
                    Q_ASSERT(schema);
                }
            }
            const Exiv2::Value::AutoPtr value = it->getValue();

            QString structName;
            int arrayIndex = -1;
            QString tagName;
            const KisMetaData::TypeInfo* typeInfo = 0;

            if (!parseTagName(key.tagName().c_str(),
                              structName, arrayIndex, tagName,
                              &typeInfo, schema)) continue;

            bool isStructureEntry = !structName.isEmpty() && arrayIndex == -1;
            bool isStructureInArrayEntry = !structName.isEmpty() && arrayIndex != -1;
            Q_ASSERT(isStructureEntry != isStructureInArrayEntry || !isStructureEntry);


            KisMetaData::Value v;
            bool ignoreValue = false;
            // Compute the value
            if (value->typeId() == Exiv2::xmpBag
                    || value->typeId() == Exiv2::xmpSeq
                    || value->typeId() == Exiv2::xmpAlt) {
                const KisMetaData::TypeInfo* embeddedTypeInfo = 0;
                if (typeInfo) {
                    embeddedTypeInfo = typeInfo->embeddedPropertyType();
                }
                const KisMetaData::Parser* parser = 0;
                if (embeddedTypeInfo) {
                    parser = embeddedTypeInfo->parser();
                }
                const Exiv2::XmpArrayValue* xav = dynamic_cast<const Exiv2::XmpArrayValue*>(value.get());
                Q_ASSERT(xav);
                QList<KisMetaData::Value> array;
                for (int i = 0; i < xav->count(); ++i) {
                    QString value = QString::fromStdString(xav->toString(i));
                    if (parser) {
                        array.push_back(parser->parse(value));
                    } else {
                        dbgImage << "No parser " << tagName;
                        array.push_back(KisMetaData::Value(value));
                    }
                }
                KisMetaData::Value::ValueType vt = KisMetaData::Value::Invalid;
                switch (xav->xmpArrayType()) {
                case Exiv2::XmpValue::xaNone:
                    warnKrita << "KisXMPIO: Unsupported array";
                    break;
                case Exiv2::XmpValue::xaAlt:
                    vt = KisMetaData::Value::AlternativeArray;
                    break;
                case Exiv2::XmpValue::xaBag:
                    vt = KisMetaData::Value::UnorderedArray;
                    break;
                case Exiv2::XmpValue::xaSeq:
                    vt = KisMetaData::Value::OrderedArray;
                    break;
                }
                v = KisMetaData::Value(array, vt);
            } else if (value->typeId() == Exiv2::langAlt) {
                const Exiv2::LangAltValue* xav = dynamic_cast<const Exiv2::LangAltValue*>(value.get());
                QList<KisMetaData::Value> alt;
                for (std::map< std::string, std::string>::const_iterator it = xav->value_.begin();
                        it != xav->value_.end(); ++it) {
                    KisMetaData::Value valt(it->second.c_str());
                    valt.addPropertyQualifier("xml:lang", KisMetaData::Value(it->first.c_str()));
                    alt.push_back(valt);
                }
                v = KisMetaData::Value(alt, KisMetaData::Value::LangArray);
            } else {
                QString valTxt = value->toString().c_str();
                if (typeInfo && typeInfo->parser()) {
                    v = typeInfo->parser()->parse(valTxt);
                } else {
                    dbgMetaData << "No parser " << tagName;
                    v = KisMetaData::Value(valTxt);
                }
                if (valTxt == "type=\"Struct\"") {
                    if (!typeInfo || typeInfo->propertyType() == KisMetaData::TypeInfo::StructureType) {
                        ignoreValue = true;
                    }
                }
            }

            // set the value
            if (isStructureEntry) {
                structures[schema][structName][tagName] = v;
            } else if (isStructureInArrayEntry) {
                if (arraysOfStructures[schema][structName].size() <= arrayIndex) {
                    arraysOfStructures[schema][structName].resize(arrayIndex + 1);
                }

                if (!arraysOfStructures[schema][structName][arrayIndex].contains(tagName)) {
                    arraysOfStructures[schema][structName][arrayIndex][tagName] = v;
                } else {
                    warnKrita << "WARNING: trying to overwrite tag" << tagName << "in" << structName << arrayIndex;
                }
            } else {
                if (!ignoreValue) {
                    store->addEntry(KisMetaData::Entry(schema, tagName, v));
                } else {
                    dbgMetaData << "Ignoring value for " << tagName << " " << v;
                }
            }
        }
    }

    for (QMap< const KisMetaData::Schema*, QMap<QString, QMap<QString, KisMetaData::Value>  > >::iterator it = structures.begin();
            it != structures.end(); ++it) {
        const KisMetaData::Schema* schema = it.key();
        for (QMap<QString, QMap<QString, KisMetaData::Value> >::iterator it2 = it.value().begin();
                it2 != it.value().end(); ++it2) {
            store->addEntry(KisMetaData::Entry(schema, it2.key(), KisMetaData::Value(it2.value())));
        }
    }
    for (QMap< const KisMetaData::Schema*, QMap<QString, QVector< QMap<QString, KisMetaData::Value> > > >::iterator it = arraysOfStructures.begin(); it != arraysOfStructures.end(); ++it) {
        const KisMetaData::Schema* schema = it.key();
        for (QMap<QString, QVector<QMap<QString, KisMetaData::Value> > >::iterator it2 = it.value().begin();
                it2 != it.value().end(); ++it2) {
            KisMetaData::Value::ValueType type = KisMetaData::Value::OrderedArray;
            QString entryName = it2.key();
            if (schema->propertyType(entryName)) {
                switch (schema->propertyType(entryName)->propertyType()) {
                case KisMetaData::TypeInfo::OrderedArrayType:
                    type = KisMetaData::Value::OrderedArray;
                    break;
                case KisMetaData::TypeInfo::UnorderedArrayType:
                    type = KisMetaData::Value::OrderedArray;
                    break;
                case KisMetaData::TypeInfo::AlternativeArrayType:
                    type = KisMetaData::Value::AlternativeArray;
                    break;
                default:
                    type = KisMetaData::Value::Invalid;
                    break;
                }
            } else if (store->containsEntry(schema, entryName)) {
                KisMetaData::Value value = store->getEntry(schema, entryName).value();
                if (value.isArray()) {
                    type = value.type();
                }
            }
            store->removeEntry(schema, entryName);
            if (type != KisMetaData::Value::Invalid) {
                QList< KisMetaData::Value > valueList;
                for (int i = 0; i < it2.value().size(); ++i) {
                    valueList.append(it2.value()[i]);
                }
                store->addEntry(KisMetaData::Entry(schema, entryName, KisMetaData::Value(valueList, type)));
            }
        }
    }

    return true;
}
