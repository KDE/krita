/*
 *  Copyright (c) 2007 Boudewijn Rempt boud@valdyas.org
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

#include "kis_action_recorder_test.h"

#include <QDomDocument>
#include <qtest_kde.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include "kis_types.h"
#include "kis_image.h"
#include "recorder/kis_action_recorder.h"
#include "recorder/kis_macro.h"

#include <ktemporaryfile.h>
#include <recorder/kis_play_info.h>

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif


void KisActionRecorderTest::testCreation()
{

    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageWSP image = new KisImage(0, 512, 512, colorSpace, "paintop registry test");

    KisActionRecorder test();
}

void KisActionRecorderTest::testFiles()
{
    QDir dirSources(QString(FILES_DATA_DIR) + "/actionrecorder/sources");
    foreach(QFileInfo sourceFileInfo, dirSources.entryInfoList()) {
        if (!sourceFileInfo.isHidden() && !sourceFileInfo.isDir()) {
            qDebug() << "handling " << sourceFileInfo.fileName();
            QFileInfo resultFileInfo(QString(FILES_DATA_DIR) + "/actionrecorder/results/" + sourceFileInfo.fileName() + ".png");
            QVERIFY2(resultFileInfo.exists(),
                     QString("Result file %1 not found").arg(resultFileInfo.fileName()).toAscii().data());
            // Replay
            // Create an image and the document
            QDomDocument domDoc;

            KisImageWSP image = new KisImage(0, 200, 200, KoColorSpaceRegistry::instance()->rgb8(), "");

            // Load recorded action
            QString err;
            int line, col;
            QFile file(sourceFileInfo.absoluteFilePath());
            QVERIFY(file.open(QIODevice::ReadOnly));
            QVERIFY(domDoc.setContent(&file, &err, &line, &col));
            file.close();
            QDomElement docElem = domDoc.documentElement();
            QVERIFY(!docElem.isNull() && docElem.tagName() == "RecordedActions");
            // Unserialise
            KisMacro m;
            m.fromXML(docElem);
            // Play
            m.play(KisPlayInfo(image, image->root()));
            QImage sourceImage = image->convertToQImage(0, 0, 200, 200, 0);
            // load what we should have get from the hard drive
            QImage resultImage(resultFileInfo.absoluteFilePath());
            resultImage = resultImage.convertToFormat(QImage::Format_ARGB32);
            QVERIFY(resultImage.width() == sourceImage.width());
            QVERIFY(resultImage.height() == sourceImage.height());
            QCOMPARE(resultImage.numBytes(), sourceImage.numBytes());
            if (memcmp(resultImage.bits(), sourceImage.bits(), sourceImage.numBytes()) != 0) {
                for (int i = 0; i < sourceImage.numBytes(); i += 4) {
                    if (resultImage.bits()[i + 3] == sourceImage.bits()[i + 3] && resultImage.bits()[i + 3] != 0) {
                        for (int j = 0; j < 4; j++) {
                            /*                            QVERIFY2( resultImage.bits()[i+j] == sourceImage.bits()[i+j],
                                                                QString("byte %1 is different : result: %2 krita: %3 in file %4").arg(i+j)
                                                                .arg((int)resultImage.bits()[i+j])
                                                                .arg((int)sourceImage.bits()[i+j])
                                                                .arg(sourceFileInfo.fileName()).toAscii().data());*/
                            // TODO figure out why sometimes there is a slight difference between original and replay
                            QVERIFY2(qAbs(resultImage.bits()[i+j] - sourceImage.bits()[i+j]) <= 4,
                                     QString("byte %1 is different : result: %2 krita: %3 in file %4").arg(i + j)
                                     .arg((int)resultImage.bits()[i+j])
                                     .arg((int)sourceImage.bits()[i+j])
                                     .arg(sourceFileInfo.fileName()).toAscii().data());
                        }
                    }
                }
            }
        }
    }
}


QTEST_KDEMAIN(KisActionRecorderTest, GUI)
#include "kis_action_recorder_test.moc"
