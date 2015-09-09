/*
 * Copyright (C) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "kis_exr_test.h"



#include <QTest>
#include <QCoreApplication>

#include <qtest_kde.h>
#include <half.h>
#include <QMimeDatabase>
#include <QMimeType>
#include "filestest.h"

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif


void KisExrTest::testFiles()
{
    TestUtil::testFiles(QString(FILES_DATA_DIR) + "/sources", QStringList(), QString(), 1);
}

void KisExrTest::testRoundTrip()
{
    QString inputFileName(TestUtil::fetchDataFileLazy("CandleGlass.exr"));

    KisDocument *doc1 = KisPart::instance()->createDocument();

    KisImportExportManager manager(doc1);
    manager.setBatchMode(true);

    KisImportExportFilter::ConversionStatus status;
    QString s = manager.importDocument(inputFileName, QString(),
                                       status);

    QCOMPARE(status, KisImportExportFilter::OK);
    QVERIFY(doc1->image());


    QTemporaryFile savedFile(QDir::tempPath() + QLatin1String("/krita_XXXXXX") + QLatin1String(".exr"));
    savedFile.setAutoRemove(false);
    savedFile.open();

    KUrl savedFileURL("file://" + savedFile.fileName());
    QString savedFileName(savedFileURL.toLocalFile());

    QString typeName;
    QMimeDatabase db;
    QMimeType t = db.mimeTypeForFile(savedFileURL.path(), QMimeDatabase::MatchExtension);
    typeName = t.name();

    QByteArray mimeType(typeName.toLatin1());
    status = manager.exportDocument(savedFileName, mimeType);
    QVERIFY(QFileInfo(savedFileName).exists());

    {
        KisDocument *doc2 = KisPart::instance()->createDocument();

        KisImportExportManager manager(doc2);
        manager.setBatchMode(true);

        s = manager.importDocument(savedFileName, QString(), status);

        QCOMPARE(status, KisImportExportFilter::OK);
        QVERIFY(doc2->image());

        QVERIFY(TestUtil::comparePaintDevicesClever<half>(
                    doc1->image()->root()->firstChild()->paintDevice(),
                    doc2->image()->root()->firstChild()->paintDevice(),
                    0.01 /* meaningless alpha */));

        delete doc2;
    }

    savedFile.close();

    delete doc1;

}

QTEST_KDEMAIN(KisExrTest, GUI)


