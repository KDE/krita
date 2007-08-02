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

#include "kis_files_test.h"

#include <QTest>
#include <QCoreApplication>

#include <qtest_kde.h>
#include <kis_doc2.h>
#include <kis_image.h>

#include <ktemporaryfile.h>

#ifndef FILES_DATA_DIR
    #error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif


void KisFilesTest::testFiles()
{
    QDir dirSources ( QString(FILES_DATA_DIR) + "/sources" );
    QDir dirResults ( QString(FILES_DATA_DIR) + "/results" );
    foreach(QFileInfo sourceFileInfo, dirSources.entryInfoList())
    {
        if( not sourceFileInfo.isHidden())
        {
            kDebug() << "Testing : " << sourceFileInfo.absoluteFilePath() << endl;
            QFileInfo resultFileInfo(  QString(FILES_DATA_DIR) + "/results/" + sourceFileInfo.fileName() + ".png" );
            QVERIFY(resultFileInfo.exists());
            KisDoc2 doc;
            doc.import( sourceFileInfo.absoluteFilePath() );
            QVERIFY(doc.image());
            KTemporaryFile tmpFile;
            tmpFile.setSuffix(".png");
            tmpFile.open();
            doc.setOutputMimeType("image/png");
            doc.saveAs( "file://" + tmpFile.fileName());
            QImage resultImg(resultFileInfo.absoluteFilePath());
            QImage sourceImg(tmpFile.fileName());
            resultImg = resultImg.convertToFormat(QImage::Format_ARGB32);
            sourceImg = sourceImg.convertToFormat(QImage::Format_ARGB32);
//             kDebug() << resultImg.width() << " " << sourceImg.width() << endl;
            QVERIFY(resultImg.width() == sourceImg.width());
            QVERIFY(resultImg.height() == sourceImg.height());
//             kDebug() << resultImg.numBytes() << " " << sourceImg.numBytes() << endl;
            QVERIFY(resultImg.numBytes() == sourceImg.numBytes());
            QVERIFY(memcmp( resultImg.bits(), sourceImg.bits(), resultImg.numBytes() )==0);
#if 0
            for(int i = 0; i < sourceImg.numBytes(); i++)
            {
//                 kDebug() <<(int)resultImg.bits()[i] << " " << (int)sourceImg.bits()[i] << endl;
//                 QVERIFY2(resultImg.bits()[i] == sourceImg.bits()[i], QString("pixel %1 is different").arg(i).toAscii().data());
            }
#endif
        }
    }
}
QTEST_KDEMAIN(KisFilesTest, GUI)

#include "kis_files_test.moc"