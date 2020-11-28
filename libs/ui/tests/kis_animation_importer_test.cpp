/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_animation_importer_test.h"

#include "KisPart.h"
#include "kis_animation_importer.h"
#include "KisDocument.h"
#include <testutil.h>
#include "kis_keyframe_channel.h"
#include "kis_image_animation_interface.h"
#include "kis_group_layer.h"
#include <KoUpdater.h>

#include "testui.h"

void KisAnimationImporterTest::testImport()
{
    QScopedPointer<KisDocument> document(KisPart::instance()->createDocument());
    TestUtil::MaskParent mp(QRect(0,0,512,512));
    document->setCurrentImage(mp.image);

    KisAnimationImporter importer(document->image());

    QStringList files;
    files.append(QString(FILES_DATA_DIR) + '/' + "file_layer_source.png");
    files.append(QString(FILES_DATA_DIR) + '/' + "carrot.png");
    files.append(QString(FILES_DATA_DIR) + '/' + "hakonepa.png");

    importer.import(files, 7, 3);

    KisGroupLayerSP root = mp.image->rootLayer();
    KisNodeSP importedLayer = root->lastChild();

    KisKeyframeChannel* contentChannel = importedLayer->getKeyframeChannel(KisKeyframeChannel::Raster.id());

    QVERIFY(contentChannel != 0);
    QCOMPARE(contentChannel->keyframeCount(), 4); // Three imported ones + blank at time 0
    QVERIFY(!contentChannel->keyframeAt(7).isNull());
    QVERIFY(!contentChannel->keyframeAt(10).isNull());
    QVERIFY(!contentChannel->keyframeAt(13).isNull());

    mp.image->animationInterface()->switchCurrentTimeAsync(7);
    mp.image->waitForDone();
    QImage imported1 = importedLayer->projection()->convertToQImage(importedLayer->colorSpace()->profile());

    mp.image->animationInterface()->switchCurrentTimeAsync(10);
    mp.image->waitForDone();
    QImage imported2 = importedLayer->projection()->convertToQImage(importedLayer->colorSpace()->profile());

    mp.image->animationInterface()->switchCurrentTimeAsync(13);
    mp.image->waitForDone();
    QImage imported3 = importedLayer->projection()->convertToQImage(importedLayer->colorSpace()->profile());

    QImage source1(QString(FILES_DATA_DIR) + '/' + "file_layer_source.png");
    QImage source2(QString(FILES_DATA_DIR) + '/' + "carrot.png");
    QImage source3(QString(FILES_DATA_DIR) + '/' + "hakonepa.png");

    QPoint pt;
    QVERIFY(TestUtil::compareQImages(pt, source1, imported1));
    QVERIFY(TestUtil::compareQImages(pt, source2, imported2));
    QVERIFY(TestUtil::compareQImages(pt, source3, imported3));

    KisPart::instance()->removeDocument(document.data(), false);
}

KISTEST_MAIN(KisAnimationImporterTest)
