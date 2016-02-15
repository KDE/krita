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

#include "kis_kra_loader_test.h"

#include <QTest>

#include <KisDocument.h>
#include <KoDocumentInfo.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include <KoColor.h>

#include "KisDocument.h"
#include "kis_image.h"
#include "testutil.h"
#include "KisPart.h"

#include <filter/kis_filter_registry.h>
#include <generator/kis_generator_registry.h>

#include "kis_image_animation_interface.h"
#include "kis_keyframe_channel.h"
#include "kis_time_range.h"

void KisKraLoaderTest::initTestCase()
{
    KisFilterRegistry::instance();
    KisGeneratorRegistry::instance();
}

void KisKraLoaderTest::testLoading()
{
    KisDocument *doc = KisPart::instance()->createDocument();
    doc->loadNativeFormat(QString(FILES_DATA_DIR) + QDir::separator() + "load_test.kra");
    KisImageWSP image = doc->image();
    image->lock();
    QCOMPARE(image->nlayers(), 12);
    QCOMPARE(doc->documentInfo()->aboutInfo("title"), QString("test image for loading"));
    QCOMPARE(image->height(), 753);
    QCOMPARE(image->width(), 1000);
    QCOMPARE(image->colorSpace()->id(), KoColorSpaceRegistry::instance()->rgb8()->id());

    KisNodeSP node = image->root()->firstChild();
    QVERIFY(node);
    QCOMPARE(node->name(), QString("Background"));
    QVERIFY(node->inherits("KisPaintLayer"));

    node = node->nextSibling();
    QVERIFY(node);
    QCOMPARE(node->name(), QString("Group 1"));
    QVERIFY(node->inherits("KisGroupLayer"));
    QCOMPARE((int) node->childCount(), 2);

    delete doc;
}

void testObligeSingleChildImpl(bool transpDefaultPixel)
{

    QString id = !transpDefaultPixel ?
        "single_layer_no_channel_flags_nontransp_def_pixel.kra" :
        "single_layer_no_channel_flags_transp_def_pixel.kra";

    QString fileName = TestUtil::fetchDataFileLazy(id);

    KisDocument *doc = KisPart::instance()->createDocument();
    doc->loadNativeFormat(fileName);
    KisImageWSP image = doc->image();

    QVERIFY(image);
    QCOMPARE(image->nlayers(), 2);

    KisNodeSP root = image->root();
    KisNodeSP child = root->firstChild();

    QVERIFY(child);

    QCOMPARE(root->original(), root->projection());

    if (transpDefaultPixel) {
        QCOMPARE(root->original(), child->projection());
    } else {
        QVERIFY(root->original() != child->projection());
    }

    delete doc;
}

void KisKraLoaderTest::testObligeSingleChild()
{
    testObligeSingleChildImpl(true);
}

void KisKraLoaderTest::testObligeSingleChildNonTranspPixel()
{
    testObligeSingleChildImpl(false);
}

void KisKraLoaderTest::testLoadAnimated()
{
    KisDocument *doc = KisPart::instance()->createDocument();
    doc->loadNativeFormat(QString(FILES_DATA_DIR) + QDir::separator() + "load_test_animation.kra");
    KisImageWSP image = doc->image();

    KisNodeSP node1 = image->root()->firstChild();
    KisNodeSP node2 = node1->nextSibling();

    QVERIFY(node1->inherits("KisPaintLayer"));
    QVERIFY(node2->inherits("KisPaintLayer"));

    KisPaintLayerSP layer1 = qobject_cast<KisPaintLayer*>(node1.data());
    KisPaintLayerSP layer2 = qobject_cast<KisPaintLayer*>(node2.data());
    KisKeyframeChannel *channel1 = layer1->getKeyframeChannel(KisKeyframeChannel::Content.id());
    KisKeyframeChannel *channel2 = layer2->getKeyframeChannel(KisKeyframeChannel::Content.id());

    QCOMPARE(channel1->keyframeCount(), 3);
    QCOMPARE(channel2->keyframeCount(), 1);

    QCOMPARE(image->animationInterface()->framerate(), 17);
    QCOMPARE(image->animationInterface()->fullClipRange(), KisTimeRange::fromTime(15, 45));
    QCOMPARE(image->animationInterface()->currentTime(), 19);

    KisPaintDeviceSP dev = layer1->paintDevice();

    const KoColorSpace *cs = dev->colorSpace();
    KoColor transparent(Qt::transparent, cs);
    KoColor white(Qt::white, cs);
    KoColor red(Qt::red, cs);

    image->animationInterface()->switchCurrentTimeAsync(0);
    image->waitForDone();

    QCOMPARE(dev->exactBounds(), QRect(506, 378, 198, 198));
    QCOMPARE(dev->x(), -26);
    QCOMPARE(dev->y(), -128);
    QVERIFY(!memcmp(dev->defaultPixel(), transparent.data(), cs->pixelSize()));

    image->animationInterface()->switchCurrentTimeAsync(20);
    image->waitForDone();

    QCOMPARE(dev->nonDefaultPixelArea(), QRect(615, 416, 129, 129));
    QCOMPARE(dev->x(), 502);
    QCOMPARE(dev->y(), 224);
    QVERIFY(!memcmp(dev->defaultPixel(), white.data(), cs->pixelSize()));

    image->animationInterface()->switchCurrentTimeAsync(30);
    image->waitForDone();

    QCOMPARE(dev->nonDefaultPixelArea(), QRect(729, 452, 45, 44));
    QCOMPARE(dev->x(), 645);
    QCOMPARE(dev->y(), -10);
    QVERIFY(!memcmp(dev->defaultPixel(), red.data(), cs->pixelSize()));
}


QTEST_MAIN(KisKraLoaderTest)
