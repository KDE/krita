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

#include "testutil.h"
#include <QDomDocument>
#include <qtest_kde.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColor.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>
#include "kis_types.h"
#include "kis_image.h"
#include "kis_paint_layer.h"
#include "recorder/kis_action_recorder.h"
#include "recorder/kis_macro.h"

#include <ktemporaryfile.h>
#include <recorder/kis_play_info.h>
#include <recorder/kis_macro_player.h>

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif


void KisActionRecorderTest::testCreation()
{

    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImage image(0, 512, 512, colorSpace, "paintop registry test");

    KisActionRecorder test();
}

void KisActionRecorderTest::testFiles()
{
    TestUtil::TestProgressBar progressProxy;
    KoProgressUpdater progressUpdater(&progressProxy);
    progressUpdater.start(100);

    QDir dirSources(QString(FILES_DATA_DIR) + "/actionrecorder/sources");
    foreach(QFileInfo sourceFileInfo, dirSources.entryInfoList()) {
        if (!sourceFileInfo.isHidden() && !sourceFileInfo.isDir()) {
            qDebug() << "handling " << sourceFileInfo.fileName();
            QFileInfo resultFileInfo(QString(FILES_DATA_DIR) + "/actionrecorder/results/" + sourceFileInfo.fileName() + ".png");
            QVERIFY2(resultFileInfo.exists(),
                     QString("Result file %1 not found").arg(resultFileInfo.fileName()).toLatin1());
            // Replay
            // Create an image and the document
            QDomDocument domDoc;

            const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
            KisImageSP image = new KisImage(0, 200, 200, cs, "");
            KoColor white(Qt::white, cs);
            KisPaintLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
            paintLayer1->paintDevice()->setDefaultPixel(white.data());
            image->addNode(paintLayer1);

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
            m.fromXML(docElem, 0);
            // Play
            KoUpdater *updater = progressUpdater.startSubtask();
            KisMacroPlayer player(&m, KisPlayInfo(image, paintLayer1), updater);
            player.start();
            player.wait();
            QImage sourceImage = image->convertToQImage(0, 0, 200, 200, 0);
            // load what we should have get from the hard drive
            QImage resultImage(resultFileInfo.absoluteFilePath());
            resultImage = resultImage.convertToFormat(QImage::Format_ARGB32);

            QPoint pt;
            if(!TestUtil::compareQImages(pt, sourceImage, resultImage, 40)) {
                sourceImage.save("action_recorder_source_" + sourceFileInfo.fileName() + ".png");
                resultImage.save("action_recorder_result_" + sourceFileInfo.fileName() + ".png");
                qCritical() << "Failed to play action:" << sourceFileInfo.fileName() << "image differs at point" << pt;
                QFAIL("Images do not coincide");
            }
        }
    }
}


QTEST_KDEMAIN(KisActionRecorderTest, GUI)
#include "kis_action_recorder_test.moc"
