/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
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

#include "kis_animation_exporter_test.h"
#include "kis_animation_exporter.h"

#include <QTest>
#include <testutil.h>
#include "KisPart.h"
#include "kis_image.h"
#include "KisDocument.h"
#include "kis_image_animation_interface.h"
#include "KoColor.h"
#include <KoUpdater.h>
#include "kis_time_range.h"
#include "kis_keyframe_channel.h"


void KisAnimationExporterTest::testAnimationExport()
{
    KisDocument *document = KisPart::instance()->createDocument();
    QRect rect(0,0,512,512);
    QRect fillRect(10,0,502,512);
    TestUtil::MaskParent p(rect);
    document->setCurrentImage(p.image);
    const KoColorSpace *cs = p.image->colorSpace();

    KUndo2Command parentCommand;

    p.layer->enableAnimation();
    KisKeyframeChannel *rasterChannel = p.layer->getKeyframeChannel(KisKeyframeChannel::Content.id(), true);

    rasterChannel->addKeyframe(1, &parentCommand);
    rasterChannel->addKeyframe(2, &parentCommand);
    p.image->animationInterface()->setFullClipRange(KisTimeRange::fromTime(0, 2));

    KisPaintDeviceSP dev = p.layer->paintDevice();

    dev->fill(fillRect, KoColor(Qt::red, cs));
    QImage frame0 = dev->convertToQImage(0, rect);

    p.image->animationInterface()->switchCurrentTimeAsync(1);
    p.image->waitForDone();
    dev->fill(fillRect, KoColor(Qt::green, cs));
    QImage frame1 = dev->convertToQImage(0, rect);

    p.image->animationInterface()->switchCurrentTimeAsync(2);
    p.image->waitForDone();
    dev->fill(fillRect, KoColor(Qt::blue, cs));
    QImage frame2 = dev->convertToQImage(0, rect);

    KisAnimationExportSaver exporter(document, "export-test.png", 0, 2);
    QSignalSpy spy(document, SIGNAL(sigProgress(int)));
    QVERIFY(spy.isValid());

    exporter.exportAnimation();

    // If we had Qt5 already:
    // spy.wait(100);
    // spy.wait(100);
    // spy.wait(100);
    // But in the meanwhile...
    QTest::qWait(1000);


    //QCOMPARE(spy.count(), 3);
    //QCOMPARE(spy.at(0).at(0).value<int>(), 0);
    //QCOMPARE(spy.at(1).at(0).value<int>(), 1);
    //QCOMPARE(spy.at(2).at(0).value<int>(), 2);

    // FIXME: Export doesn't seem to work from unit tests

    QImage exported;

    exported.load("export-test0000.png");
    QCOMPARE(exported, frame0);

    exported.load("export-test0001.png");
    QCOMPARE(exported, frame1);

    exported.load("export-test0002.png");
    QCOMPARE(exported, frame2);
}

QTEST_MAIN(KisAnimationExporterTest)
