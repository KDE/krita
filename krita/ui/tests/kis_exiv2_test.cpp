/*
 * Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_exiv2_test.h"

#include <QTest>
#include <QCoreApplication>

#include <qtest_kde.h>

#include "kis_debug.h"
#include "kis_meta_data_entry.h"
#include "kis_meta_data_io_backend.h"
#include "kis_meta_data_schema.h"
#include "kis_meta_data_schema_registry.h"
#include "kis_meta_data_store.h"
#include "kis_meta_data_validator.h"
#include "kis_meta_data_value.h"

#include "filestest.h"

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the metadata parser in krita"
#endif

using namespace KisMetaData;

void KisExiv2Test::testExifLoader()
{
    IOBackend* exifIO = IOBackendRegistry::instance()->get("exif");
    QVERIFY(exifIO);
    QFile exifFile( QString(FILES_DATA_DIR) + "/metadata/hpim3238.exv" );
    exifFile.open( QIODevice::ReadOnly );
    exifFile.seek(17);
    QByteArray exifBytes = exifFile.readAll();
    QBuffer exifBuffer(&exifBytes);
    
    Store* store = new Store;
    bool loadSuccess = exifIO->loadFrom(store, &exifBuffer);
    QVERIFY(loadSuccess);
    Validator validator(store);
    
    for(QMap<QString, Validator::Reason>::const_iterator it = validator.invalidEntries().begin();
        it != validator.invalidEntries().end(); ++it )
    {
        dbgKrita << it.key() << " = " << it.value().type() << " entry = " << store->getEntry(it.key());
    }
    
    QCOMPARE(validator.countInvalidEntries(), 0 );
    QCOMPARE(validator.countValidEntries(), 51 );
    
    const KisMetaData::Schema* tiffSchema = KisMetaData::SchemaRegistry::instance()->schemaFromUri(KisMetaData::Schema::TIFFSchemaUri);
    const KisMetaData::Schema* exifSchema = KisMetaData::SchemaRegistry::instance()->schemaFromUri(KisMetaData::Schema::EXIFSchemaUri);
    const KisMetaData::Schema* dcSchema = KisMetaData::SchemaRegistry::instance()->schemaFromUri(KisMetaData::Schema::DublinCoreSchemaUri);
    const KisMetaData::Schema* xmpSchema = KisMetaData::SchemaRegistry::instance()->schemaFromUri(KisMetaData::Schema::XMPSchemaUri);
    const KisMetaData::Schema* mknSchema = KisMetaData::SchemaRegistry::instance()->schemaFromUri(KisMetaData::Schema::MakerNoteSchemaUri);
    
    QCOMPARE(store->getEntry( tiffSchema, "Make" ).value(), Value("Hewlett-Packard" ) );
    QCOMPARE(store->getEntry( tiffSchema, "Model" ).value(), Value("HP PhotoSmart R707 (V01.00) " ) );
    QCOMPARE(store->getEntry( tiffSchema, "Orientation" ).value(), Value(1) );

    
    QCOMPARE(store->getEntry( mknSchema, "RawData" ).value(), Value("SFBNZXQ=" ) );
}

QTEST_KDEMAIN(KisExiv2Test, GUI)

#include "kis_exiv2_test.moc"
