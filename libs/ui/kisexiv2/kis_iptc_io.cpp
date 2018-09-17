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
#include "kis_iptc_io.h"

#include <kis_debug.h>

#include <exiv2/iptc.hpp>

#include "kis_exiv2.h"

#include <kis_meta_data_store.h>
#include <kis_meta_data_entry.h>
#include <kis_meta_data_value.h>
#include <kis_meta_data_schema.h>
#include <kis_meta_data_schema_registry.h>

const char photoshopMarker[] = "Photoshop 3.0\0";
const char photoshopBimId_[] = "8BIM";
const uint16_t photoshopIptc = 0x0404;
const QByteArray photoshopIptc_((char*)&photoshopIptc, 2);

struct IPTCToKMD {
    QString exivTag;
    QString namespaceUri;
    QString name;
};

static const IPTCToKMD mappings[] = {
    { "Iptc.Application2.City", KisMetaData::Schema::PhotoshopSchemaUri, "City" },
    { "Iptc.Application2.Copyright", KisMetaData::Schema::DublinCoreSchemaUri, "rights" },
    { "Iptc.Application2.CountryName", KisMetaData::Schema::PhotoshopSchemaUri, "Country" },
    { "Iptc.Application2.CountryCode", KisMetaData::Schema::IPTCSchemaUri, "CountryCode" },
    { "Iptc.Application2.Byline", KisMetaData::Schema::DublinCoreSchemaUri, "creator" },
    { "Iptc.Application2.BylineTitle", KisMetaData::Schema::PhotoshopSchemaUri, "AuthorsPosition" },
    { "Iptc.Application2.DateCreated", KisMetaData::Schema::PhotoshopSchemaUri, "DateCreated" },
    { "Iptc.Application2.Caption", KisMetaData::Schema::DublinCoreSchemaUri, "description" },
    { "Iptc.Application2.Writer", KisMetaData::Schema::PhotoshopSchemaUri, "CaptionWriter" },
    { "Iptc.Application2.Headline", KisMetaData::Schema::PhotoshopSchemaUri, "Headline" },
    { "Iptc.Application2.SpecialInstructions", KisMetaData::Schema::PhotoshopSchemaUri, "Instructions" },
    { "Iptc.Application2.ObjectAttribute", KisMetaData::Schema::IPTCSchemaUri, "IntellectualGenre" },
    { "Iptc.Application2.TransmissionReference", KisMetaData::Schema::PhotoshopSchemaUri, "JobID" },
    { "Iptc.Application2.Keywords", KisMetaData::Schema::DublinCoreSchemaUri, "subject" },
    { "Iptc.Application2.SubLocation", KisMetaData::Schema::IPTCSchemaUri, "Location" },
    { "Iptc.Application2.Credit", KisMetaData::Schema::PhotoshopSchemaUri, "Credit" },
    { "Iptc.Application2.ProvinceState", KisMetaData::Schema::PhotoshopSchemaUri, "State" },
    { "Iptc.Application2.Source", KisMetaData::Schema::PhotoshopSchemaUri, "Source" },
    { "Iptc.Application2.Subject", KisMetaData::Schema::IPTCSchemaUri, "SubjectCode" },
    { "Iptc.Application2.ObjectName", KisMetaData::Schema::DublinCoreSchemaUri, "title" },
    { "Iptc.Application2.Urgency", KisMetaData::Schema::PhotoshopSchemaUri, "Urgency" },
    { "Iptc.Application2.Category", KisMetaData::Schema::PhotoshopSchemaUri, "Category" },
    { "Iptc.Application2.SuppCategory", KisMetaData::Schema::PhotoshopSchemaUri, "SupplementalCategory" },
    { "", "", "" } // indicates the end of the array
};

struct KisIptcIO::Private {
    QHash<QString, IPTCToKMD> iptcToKMD;
    QHash<QString, IPTCToKMD> kmdToIPTC;
};

// ---- Implementation of KisExifIO ----//
KisIptcIO::KisIptcIO() : d(new Private)
{
}

KisIptcIO::~KisIptcIO()
{
    delete d;
}

void KisIptcIO::initMappingsTable() const
{
    // For some reason, initializing the tables in the constructor makes the it crash
    if (d->iptcToKMD.size() == 0) {
        for (int i = 0; !mappings[i].exivTag.isEmpty(); i++) {
            dbgKrita << "mapping[i] = " << mappings[i].exivTag << " " << mappings[i].namespaceUri << " " << mappings[i].name;
            d->iptcToKMD[mappings[i].exivTag] = mappings[i];
            d->kmdToIPTC[
                KisMetaData::SchemaRegistry::instance()
                ->schemaFromUri(mappings[i].namespaceUri)
                ->generateQualifiedName(mappings[i].name)] = mappings[i];
        }
    }
}

bool KisIptcIO::saveTo(KisMetaData::Store* store, QIODevice* ioDevice, HeaderType headerType) const
{
    QStringList blockedEntries = QStringList() << "photoshop:DateCreated";

    initMappingsTable();
    ioDevice->open(QIODevice::WriteOnly);
    Exiv2::IptcData iptcData;
    for (QHash<QString, KisMetaData::Entry>::const_iterator it = store->begin();
            it != store->end(); ++it) {
        const KisMetaData::Entry& entry = *it;
        if (d->kmdToIPTC.contains(entry.qualifiedName())) {
            if (blockedEntries.contains(entry.qualifiedName())) {
                warnKrita << "skipping" << entry.qualifiedName() << entry.value();
                continue;
            }
            try {
                QString iptcKeyStr = d->kmdToIPTC[ entry.qualifiedName()].exivTag;
                Exiv2::IptcKey iptcKey(qPrintable(iptcKeyStr));
                Exiv2::Value *v = kmdValueToExivValue(entry.value(),
                                                      Exiv2::IptcDataSets::dataSetType(iptcKey.tag(), iptcKey.record()));

                if (v && v->typeId() != Exiv2::invalidTypeId) {
                    iptcData.add(iptcKey, v);
                }
            } catch (Exiv2::AnyError& e) {
                dbgMetaData << "exiv error " << e.what();
            }
        }
    }
#if EXIV2_MAJOR_VERSION == 0 && EXIV2_MINOR_VERSION <= 17
    Exiv2::DataBuf rawData = iptcData.copy();
#else
    Exiv2::DataBuf rawData = Exiv2::IptcParser::encode(iptcData);
#endif

    if (headerType == KisMetaData::IOBackend::JpegHeader) {
        QByteArray header;
        header.append(photoshopMarker);
        header.append(QByteArray(1, 0));   // Null terminated string
        header.append(photoshopBimId_);
        header.append(photoshopIptc_);
        header.append(QByteArray(2, 0));
        qint32 size = rawData.size_;
        QByteArray sizeArray(4, 0);
        sizeArray[0] = (char)((size & 0xff000000) >> 24);
        sizeArray[1] = (char)((size & 0x00ff0000) >> 16);
        sizeArray[2] = (char)((size & 0x0000ff00) >> 8);
        sizeArray[3] = (char)(size & 0x000000ff);
        header.append(sizeArray);
        ioDevice->write(header);
    }

    ioDevice->write((const char*) rawData.pData_, rawData.size_);
    ioDevice->close();
    return true;
}

bool KisIptcIO::canSaveAllEntries(KisMetaData::Store* store) const
{
    Q_UNUSED(store);
    return false;
}

bool KisIptcIO::loadFrom(KisMetaData::Store* store, QIODevice* ioDevice) const
{
    initMappingsTable();
    dbgMetaData << "Loading IPTC Tags";
    ioDevice->open(QIODevice::ReadOnly);
    QByteArray arr = ioDevice->readAll();
    Exiv2::IptcData iptcData;
#if EXIV2_MAJOR_VERSION == 0 && EXIV2_MINOR_VERSION <= 17
    iptcData.load((const Exiv2::byte*)arr.data(), arr.size());
#else
    Exiv2::IptcParser::decode(iptcData, (const Exiv2::byte*)arr.data(), arr.size());
#endif
    dbgMetaData << "There are" << iptcData.count() << " entries in the IPTC section";
    for (Exiv2::IptcMetadata::const_iterator it = iptcData.begin();
            it != iptcData.end(); ++it) {
        dbgMetaData << "Reading info for key" << it->key().c_str();
        if (d->iptcToKMD.contains(it->key().c_str())) {
            const IPTCToKMD& iptcToKMd = d->iptcToKMD[it->key().c_str()];
            const KisMetaData::Schema* schema = KisMetaData::SchemaRegistry::instance()->schemaFromUri(iptcToKMd.namespaceUri);
            KisMetaData::Value value;
            if (iptcToKMd.exivTag == "Iptc.Application2.Keywords") {
                Q_ASSERT(it->getValue()->typeId() == Exiv2::string);
                QString data = it->getValue()->toString().c_str();

                QStringList list = data.split(',');
                QList<KisMetaData::Value> values;
                Q_FOREACH (const QString &entry, list) {
                    values.push_back(KisMetaData::Value(entry));
                }
                value = KisMetaData::Value(values, KisMetaData::Value::UnorderedArray);
            } else {
                value = exivValueToKMDValue(it->getValue(), false);
            }
            store->addEntry(KisMetaData::Entry(schema, iptcToKMd.name, value));
        }
    }
    return false;
}
