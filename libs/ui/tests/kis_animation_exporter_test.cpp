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

#include "dialogs/KisAsyncAnimationFramesSaveDialog.h"

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
#include <kistest.h>

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
    p.image->animationInterface()->setFullClipRange(KisTimeSpan(0, 2));

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

    KisAsyncAnimationFramesSaveDialog exporter(document->image(),
                                               KisTimeSpan(0,2),
                                               "export-test.png",
                                               0,
                                               0);



    exporter.setBatchMode(true);
    exporter.regenerateRange(0);

    QTest::qWait(1000);

    QImage exported;

    QPoint errpoint;
    exported.load("export-test0000.png");
    qDebug() << exported.size() << frame0.size();
    if (!TestUtil::compareQImages(errpoint, exported, frame0)) {
        QFAIL(QString("Failed to export identical frame0, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }

    exported.load("export-test0001.png");
    if (!TestUtil::compareQImages(errpoint, exported, frame1)) {
        QFAIL(QString("Failed to export identical frame1, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }

    exported.load("export-test0002.png");
    if (!TestUtil::compareQImages(errpoint, exported, frame2)) {
        QFAIL(QString("Failed to export identical frame2, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

KISTEST_MAIN(KisAnimationExporterTest)
