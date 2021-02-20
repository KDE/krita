/*
 * SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_exiv2_test.h"

#include <QTest>
#include <QCoreApplication>

#include <QBuffer>

#include "kis_debug.h"
#include "kis_meta_data_entry.h"
#include "kis_meta_data_io_backend.h"
#include "kis_meta_data_schema.h"
#include "kis_meta_data_schema_registry.h"
#include "kis_meta_data_store.h"
#include "kis_meta_data_validator.h"
#include <kis_meta_data_value.h>
#include "kisexiv2/kis_exiv2.h"
#include "filestest.h"
#include "sdk/tests/testui.h"

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the metadata parser in krita"
#endif

using namespace KisMetaData;

void KisExiv2Test::testExifLoader()
{
    KisExiv2::initialize();

    IOBackend* exifIO = IOBackendRegistry::instance()->get("exif");
    QVERIFY(exifIO);
    QFile exifFile(QString(FILES_DATA_DIR) + "/metadata/hpim3238.exv");
    exifFile.open(QIODevice::ReadOnly);
    exifFile.seek(17);
    QByteArray exifBytes = exifFile.readAll();
    QBuffer exifBuffer(&exifBytes);

    Store* store = new Store;
    bool loadSuccess = exifIO->loadFrom(store, &exifBuffer);
    QVERIFY(loadSuccess);
    Validator validator(store);

    for (QMap<QString, Validator::Reason>::const_iterator it = validator.invalidEntries().begin();
            it != validator.invalidEntries().end(); ++it) {
        dbgKrita << it.key() << " = " << it.value().type() << " entry = " << store->getEntry(it.key());
    }

    QCOMPARE(validator.countInvalidEntries(), 0);
    QCOMPARE(validator.countValidEntries(), 51);

    const KisMetaData::Schema* tiffSchema = KisMetaData::SchemaRegistry::instance()->schemaFromUri(KisMetaData::Schema::TIFFSchemaUri);

    QCOMPARE(store->getEntry(tiffSchema, "Make").value(), Value("Hewlett-Packard"));
    QCOMPARE(store->getEntry(tiffSchema, "Model").value(), Value("HP PhotoSmart R707 (V01.00) "));
    QCOMPARE(store->getEntry(tiffSchema, "Orientation").value(), Value(1));
    QCOMPARE(store->getEntry(tiffSchema, "XResolution").value(), Value(Rational(72 / 1)));
    QCOMPARE(store->getEntry(tiffSchema, "YResolution").value(), Value(Rational(72 / 1)));
    QCOMPARE(store->getEntry(tiffSchema, "ResolutionUnit").value(), Value(2));
    QCOMPARE(store->getEntry(tiffSchema, "YCbCrPositioning").value(), Value(1));

    const KisMetaData::Schema* exifSchema = KisMetaData::SchemaRegistry::instance()->schemaFromUri(KisMetaData::Schema::EXIFSchemaUri);

    QCOMPARE(store->getEntry(exifSchema, "ExposureTime").value(), Value(Rational(35355, 100000)));
    QCOMPARE(store->getEntry(exifSchema, "FNumber").value(), Value(Rational(280, 100)));
    QCOMPARE(store->getEntry(exifSchema, "ExposureProgram").value(), Value(2));
//     QCOMPARE(store->getEntry(exifSchema, "ISOSpeedRatings").value(), Value(100)); // TODO it's a list
    // TODO test OECF
    QCOMPARE(store->getEntry(exifSchema, "ExifVersion").value(), Value("0220"));
    QCOMPARE(store->getEntry(exifSchema, "DateTimeOriginal").value(), Value(QDateTime(QDate(2007, 5, 8), QTime(0, 19, 18))));
    QCOMPARE(store->getEntry(exifSchema, "DateTimeDigitized").value(), Value(QDateTime(QDate(2007, 5, 8), QTime(0, 19, 18))));
    // TODO ComponentsConfiguration
    QCOMPARE(store->getEntry(exifSchema, "ShutterSpeedValue").value(), Value(Rational(384, 256)));
    QCOMPARE(store->getEntry(exifSchema, "ApertureValue").value(), Value(Rational(780, 256)));
    QCOMPARE(store->getEntry(exifSchema, "BrightnessValue").value(), Value(Rational(-37, 256)));
    QCOMPARE(store->getEntry(exifSchema, "ExposureBiasValue").value(), Value(Rational(256, 256)));
    QCOMPARE(store->getEntry(exifSchema, "MaxApertureValue").value(), Value(Rational(280, 100)));
    QCOMPARE(store->getEntry(exifSchema, "SubjectDistance").value(), Value(Rational(65535, 1000)));


    const KisMetaData::Schema* dcSchema = KisMetaData::SchemaRegistry::instance()->schemaFromUri(KisMetaData::Schema::DublinCoreSchemaUri);
    Q_UNUSED(dcSchema);

    const KisMetaData::Schema* xmpSchema = KisMetaData::SchemaRegistry::instance()->schemaFromUri(KisMetaData::Schema::XMPSchemaUri);
    QCOMPARE(store->getEntry(xmpSchema, "CreatorTool").value(), Value("digiKam-0.9.1"));
    QCOMPARE(store->getEntry(xmpSchema, "ModifyDate").value(), Value(QDateTime(QDate(2007, 5, 8), QTime(0, 19, 18))));

    const KisMetaData::Schema* mknSchema = KisMetaData::SchemaRegistry::instance()->schemaFromUri(KisMetaData::Schema::MakerNoteSchemaUri);
    QCOMPARE(store->getEntry(mknSchema, "RawData").value(), Value("SFBNZXQ="));
}

KISTEST_MAIN(KisExiv2Test)

