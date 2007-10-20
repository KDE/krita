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
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

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
            QFileInfo resultFileInfo(  QString(FILES_DATA_DIR) + "/results/" + sourceFileInfo.fileName() + ".png" );
            QVERIFY2(resultFileInfo.exists(), QString( "Result file %1 not found" ).arg(resultFileInfo.fileName()).toAscii().data() );
            KisDoc2 doc;
            doc.import( sourceFileInfo.absoluteFilePath() );
            QVERIFY(doc.image());
            QString id = doc.image()->colorSpace()->id();
            if(id != "GRAYA" and id != "GRAYA16" and id != "RGBA" and id != "RGBA16")
            {
              kDebug() << "Images need conversion";
              doc.image()->convertTo( KoColorSpaceRegistry::instance()->rgb8());
            }
            KTemporaryFile tmpFile;
            tmpFile.setSuffix(".png");
            tmpFile.open();
            tmpFile.setAutoRemove(false);
            doc.setOutputMimeType("image/png");
            doc.saveAs( "file://" + tmpFile.fileName());
            doc.saveAs( KUrl("file:///home/cyrille/test2.png" ) );
            QImage resultImg(resultFileInfo.absoluteFilePath());
            QImage sourceImg(tmpFile.fileName());
            resultImg.save("/home/cyrille/testResult.png");
            resultImg = resultImg.convertToFormat(QImage::Format_ARGB32);
            resultImg.save("/home/cyrille/testResultRGB.png");
            sourceImg.save("/home/cyrille/testSource.png");
            sourceImg = sourceImg.convertToFormat(QImage::Format_ARGB32);
            sourceImg.save("/home/cyrille/testSourceRGB.png");
            QVERIFY(resultImg.width() == sourceImg.width());
            QVERIFY(resultImg.height() == sourceImg.height());
            QVERIFY(resultImg.numBytes() == sourceImg.numBytes());
            if(memcmp(resultImg.bits(), sourceImg.bits(), sourceImg.numBytes()) != 0)
            {
                for(int i = 0; i < sourceImg.numBytes(); i+=4)
                {
                    int alpha = resultImg.bits()[i + 3];
                    if( resultImg.bits()[i + 3] == sourceImg.bits()[i + 3] and resultImg.bits()[i + 3] != 0 )
                    {
                        for(int j = 0; j < 4; j++ )
                        {
                            QVERIFY2( resultImg.bits()[i+j] == sourceImg.bits()[i+j], 
                                    QString("byte %1 is different : result: %2 krita: %3 in file %4").arg(i+j)
                                    .arg((int)resultImg.bits()[i+j])
                                    .arg((int)sourceImg.bits()[i+j])
                                    .arg(sourceFileInfo.fileName()).toAscii().data());
                        }
                    }
                }
            }
        }
    }
}
QTEST_KDEMAIN(KisFilesTest, GUI)

#include "kis_files_test.moc"
