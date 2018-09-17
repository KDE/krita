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

#include "kis_animation_importer_test.h"

#include "KisPart.h"
#include "kis_animation_importer.h"
#include "KisDocument.h"
#include "testutil.h"
#include "kis_keyframe_channel.h"
#include "kis_image_animation_interface.h"
#include "kis_group_layer.h"
#include <KoUpdater.h>

#include "kistest.h"

void KisAnimationImporterTest::testImport()
{
    KisDocument *document = KisPart::instance()->createDocument();
    TestUtil::MaskParent mp(QRect(0,0,512,512));
    document->setCurrentImage(mp.image);

    KisAnimationImporter importer(document->image());

    QStringList files;
    files.append(QString(FILES_DATA_DIR) + QDir::separator() + "file_layer_source.png");
    files.append(QString(FILES_DATA_DIR) + QDir::separator() + "carrot.png");
    files.append(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");

    importer.import(files, 7, 3);

    KisGroupLayerSP root = mp.image->rootLayer();
    KisNodeSP importedLayer = root->lastChild();

    KisKeyframeChannel* contentChannel = importedLayer->getKeyframeChannel(KisKeyframeChannel::Content.id());

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

    QImage source1(QString(FILES_DATA_DIR) + QDir::separator() + "file_layer_source.png");
    QImage source2(QString(FILES_DATA_DIR) + QDir::separator() + "carrot.png");
    QImage source3(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");

    QPoint pt;
    QVERIFY(TestUtil::compareQImages(pt, source1, imported1));
    QVERIFY(TestUtil::compareQImages(pt, source2, imported2));
    QVERIFY(TestUtil::compareQImages(pt, source3, imported3));

    delete document;
}

KISTEST_MAIN(KisAnimationImporterTest)
