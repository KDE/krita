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

#include "kis_kra_saver_test.h"

#include <QTest>

#include <QBitArray>

#include <KisDocument.h>
#include <KoDocumentInfo.h>
#include <KoShapeContainer.h>
#include <KoPathShape.h>

#include "filter/kis_filter_registry.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter.h"
#include "kis_image.h"
#include "kis_pixel_selection.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_clone_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_shape_layer.h"
#include "kis_filter_mask.h"
#include "kis_transparency_mask.h"
#include "kis_selection_mask.h"
#include "kis_selection.h"
#include "kis_fill_painter.h"
#include "kis_shape_selection.h"
#include "util.h"
#include "testutil.h"
#include "kis_keyframe_channel.h"
#include "kis_image_animation_interface.h"
#include "kis_layer_properties_icons.h"

#include "kis_transform_mask_params_interface.h"

#include <generator/kis_generator_registry.h>

#include <KoResourcePaths.h>
#include  <sdk/tests/kistest.h>

void KisKraSaverTest::initTestCase()
{
    KoResourcePaths::addResourceDir(ResourceType::Patterns, QString(SYSTEM_RESOURCES_DATA_DIR) + "/patterns");

    KisFilterRegistry::instance();
    KisGeneratorRegistry::instance();
}

void KisKraSaverTest::testCrashyShapeLayer()
{
    /**
     * KisShapeLayer used to call setImage from its destructor and
     * therefore causing an infinite recursion (when at least one transparency
     * mask was preset. This testcase just checks that.
     */

    //QScopedPointer<KisDocument> doc(createCompleteDocument(true));
    //Q_UNUSED(doc);
}

void KisKraSaverTest::testRoundTrip()
{
    KisDocument* doc = createCompleteDocument();
    KoColor bgColor(Qt::red, doc->image()->colorSpace());
    doc->image()->setDefaultProjectionColor(bgColor);
    doc->exportDocumentSync(QUrl::fromLocalFile("roundtriptest.kra"), doc->mimeType());

    QStringList list;
    KisCountVisitor cv1(list, KoProperties());
    doc->image()->rootLayer()->accept(cv1);

    KisDocument *doc2 = KisPart::instance()->createDocument();
    bool result = doc2->loadNativeFormat("roundtriptest.kra");
    QVERIFY(result);

    KisCountVisitor cv2(list, KoProperties());
    doc2->image()->rootLayer()->accept(cv2);
    QCOMPARE(cv1.count(), cv2.count());

    // check whether the BG color is saved correctly
    QCOMPARE(doc2->image()->defaultProjectionColor(), bgColor);

    // test round trip of a transform mask
    KisNode* tnode =
        TestUtil::findNode(doc2->image()->rootLayer(), "testTransformMask").data();
    QVERIFY(tnode);
    KisTransformMask *tmask = dynamic_cast<KisTransformMask*>(tnode);
    QVERIFY(tmask);
    KisDumbTransformMaskParams *params = dynamic_cast<KisDumbTransformMaskParams*>(tmask->transformParams().data());
    QVERIFY(params);
    QTransform t = params->testingGetTransform();
    QCOMPARE(t, createTestingTransform());


    delete doc2;
    delete doc;
}

void KisKraSaverTest::testSaveEmpty()
{
    KisDocument* doc = createEmptyDocument();
    doc->exportDocumentSync(QUrl::fromLocalFile("emptytest.kra"), doc->mimeType());
    QStringList list;
    KisCountVisitor cv1(list, KoProperties());
    doc->image()->rootLayer()->accept(cv1);

    KisDocument *doc2 = KisPart::instance()->createDocument();
    doc2->loadNativeFormat("emptytest.kra");

    KisCountVisitor cv2(list, KoProperties());
    doc2->image()->rootLayer()->accept(cv2);
    QCOMPARE(cv1.count(), cv2.count());

    delete doc2;
    delete doc;
}

#include <generator/kis_generator.h>

void testRoundTripFillLayerImpl(const QString &testName, KisFilterConfigurationSP config)
{
    TestUtil::ReferenceImageChecker chk(testName, "fill_layer");
    chk.setFuzzy(2);

    QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());

    // mask parent should be destructed before the document!
    QRect refRect(0,0,512,512);
    TestUtil::MaskParent p(refRect);

    doc->setCurrentImage(p.image);
    doc->documentInfo()->setAboutInfo("title", p.image->objectName());

    KisSelectionSP selection;
    KisGeneratorLayerSP glayer = new KisGeneratorLayer(p.image, "glayer", config, selection);

    p.image->addNode(glayer, p.image->root(), KisNodeSP());
    glayer->setDirty();

    p.image->waitForDone();
    chk.checkImage(p.image, "00_initial_layer_update");

    doc->exportDocumentSync(QUrl::fromLocalFile("roundtrip_fill_layer_test.kra"), doc->mimeType());

    QScopedPointer<KisDocument> doc2(KisPart::instance()->createDocument());
    doc2->loadNativeFormat("roundtrip_fill_layer_test.kra");

    doc2->image()->waitForDone();
    chk.checkImage(doc2->image(), "01_fill_layer_round_trip");

    QVERIFY(chk.testPassed());
}

void KisKraSaverTest::testRoundTripFillLayerColor()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    KisGeneratorSP generator = KisGeneratorRegistry::instance()->get("color");
    Q_ASSERT(generator);

    // warning: we pass null paint device to the default constructed value
    KisFilterConfigurationSP config = generator->factoryConfiguration();
    Q_ASSERT(config);

    QVariant v;
    v.setValue(KoColor(Qt::red, cs));
    config->setProperty("color", v);

    testRoundTripFillLayerImpl("fill_layer_color", config);
}

void KisKraSaverTest::testRoundTripFillLayerPattern()
{
    KisGeneratorSP generator = KisGeneratorRegistry::instance()->get("pattern");
    QVERIFY(generator);

    // warning: we pass null paint device to the default constructed value
    KisFilterConfigurationSP config = generator->factoryConfiguration();
    QVERIFY(config);

    QVariant v;
    v.setValue(QString("11_drawed_furry.png"));
    config->setProperty("pattern", v);

    testRoundTripFillLayerImpl("fill_layer_pattern", config);
}

#include "kis_psd_layer_style.h"


void KisKraSaverTest::testRoundTripLayerStyles()
{
    TestUtil::ReferenceImageChecker chk("kra_saver_test", "layer_styles");

    QRect imageRect(0,0,512,512);

    // the document should be created before the image!
    QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(new KisSurrogateUndoStore(), imageRect.width(), imageRect.height(), cs, "test image");
    KisPaintLayerSP layer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
    KisPaintLayerSP layer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE_U8);
    KisPaintLayerSP layer3 = new KisPaintLayer(image, "paint3", OPACITY_OPAQUE_U8);
    image->addNode(layer1);
    image->addNode(layer2);
    image->addNode(layer3);

    doc->setCurrentImage(image);
    doc->documentInfo()->setAboutInfo("title", image->objectName());

    layer1->paintDevice()->fill(QRect(100, 100, 100, 100), KoColor(Qt::red, cs));
    layer2->paintDevice()->fill(QRect(200, 200, 100, 100), KoColor(Qt::green, cs));
    layer3->paintDevice()->fill(QRect(300, 300, 100, 100), KoColor(Qt::blue, cs));

    KisPSDLayerStyleSP style(new KisPSDLayerStyle());
    style->dropShadow()->setEffectEnabled(true);


    style->dropShadow()->setAngle(-90);
    style->dropShadow()->setUseGlobalLight(false);
    layer1->setLayerStyle(style->clone());

    style->dropShadow()->setAngle(180);
    style->dropShadow()->setUseGlobalLight(true);
    layer2->setLayerStyle(style->clone());

    style->dropShadow()->setAngle(90);
    style->dropShadow()->setUseGlobalLight(false);
    layer3->setLayerStyle(style->clone());

    image->initialRefreshGraph();
    chk.checkImage(image, "00_initial_layers");

    doc->exportDocumentSync(QUrl::fromLocalFile("roundtrip_layer_styles.kra"), doc->mimeType());


    QScopedPointer<KisDocument> doc2(KisPart::instance()->createDocument());
    doc2->loadNativeFormat("roundtrip_layer_styles.kra");

    doc2->image()->waitForDone();
    chk.checkImage(doc2->image(), "00_initial_layers");

    QVERIFY(chk.testPassed());
}

void KisKraSaverTest::testRoundTripAnimation()
{
    QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());

    QRect imageRect(0,0,512,512);
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(new KisSurrogateUndoStore(), imageRect.width(), imageRect.height(), cs, "test image");
    KisPaintLayerSP layer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
    image->addNode(layer1);

    layer1->paintDevice()->fill(QRect(100, 100, 50, 50), KoColor(Qt::black, cs));
    layer1->paintDevice()->setDefaultPixel(KoColor(Qt::red, cs));

    KUndo2Command parentCommand;

    layer1->enableAnimation();
    KisKeyframeChannel *rasterChannel = layer1->getKeyframeChannel(KisKeyframeChannel::Content.id(), true);
    QVERIFY(rasterChannel);

    rasterChannel->addKeyframe(10, &parentCommand);
    image->animationInterface()->switchCurrentTimeAsync(10);
    image->waitForDone();
    layer1->paintDevice()->fill(QRect(200, 50, 10, 10), KoColor(Qt::black, cs));
    layer1->paintDevice()->moveTo(25, 15);
    layer1->paintDevice()->setDefaultPixel(KoColor(Qt::green, cs));

    rasterChannel->addKeyframe(20, &parentCommand);
    image->animationInterface()->switchCurrentTimeAsync(20);
    image->waitForDone();
    layer1->paintDevice()->fill(QRect(150, 200, 30, 30), KoColor(Qt::black, cs));
    layer1->paintDevice()->moveTo(100, 50);
    layer1->paintDevice()->setDefaultPixel(KoColor(Qt::blue, cs));

    QVERIFY(!layer1->useInTimeline());
    layer1->setUseInTimeline(true);

    doc->setCurrentImage(image);
    doc->exportDocumentSync(QUrl::fromLocalFile("roundtrip_animation.kra"), doc->mimeType());

    QScopedPointer<KisDocument> doc2(KisPart::instance()->createDocument());
    doc2->loadNativeFormat("roundtrip_animation.kra");
    KisImageSP image2 = doc2->image();
    KisNodeSP node = image2->root()->firstChild();

    QVERIFY(node->inherits("KisPaintLayer"));
    KisPaintLayerSP layer2 = qobject_cast<KisPaintLayer*>(node.data());
    cs = layer2->paintDevice()->colorSpace();

    QCOMPARE(image2->animationInterface()->currentTime(), 20);
    KisKeyframeChannel *channel = layer2->getKeyframeChannel(KisKeyframeChannel::Content.id());
    QVERIFY(channel);
    QCOMPARE(channel->keyframeCount(), 3);

    image2->animationInterface()->switchCurrentTimeAsync(0);
    image2->waitForDone();

    QCOMPARE(layer2->paintDevice()->nonDefaultPixelArea(), QRect(64, 64, 128, 128));
    QCOMPARE(layer2->paintDevice()->x(), 0);
    QCOMPARE(layer2->paintDevice()->y(), 0);
    QCOMPARE(layer2->paintDevice()->defaultPixel(), KoColor(Qt::red, cs));

    image2->animationInterface()->switchCurrentTimeAsync(10);
    image2->waitForDone();

    QCOMPARE(layer2->paintDevice()->nonDefaultPixelArea(), QRect(217, 15, 64, 64));
    QCOMPARE(layer2->paintDevice()->x(), 25);
    QCOMPARE(layer2->paintDevice()->y(), 15);
    QCOMPARE(layer2->paintDevice()->defaultPixel(), KoColor(Qt::green, cs));

    image2->animationInterface()->switchCurrentTimeAsync(20);
    image2->waitForDone();

    QCOMPARE(layer2->paintDevice()->nonDefaultPixelArea(), QRect(228, 242, 64, 64));
    QCOMPARE(layer2->paintDevice()->x(), 100);
    QCOMPARE(layer2->paintDevice()->y(), 50);
    QCOMPARE(layer2->paintDevice()->defaultPixel(), KoColor(Qt::blue, cs));

    QVERIFY(layer2->useInTimeline());

}

#include "lazybrush/kis_lazy_fill_tools.h"

void KisKraSaverTest::testRoundTripColorizeMask()
{
    QRect imageRect(0,0,512,512);
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    const KoColorSpace * weirdCS = KoColorSpaceRegistry::instance()->rgb16();

    QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());
    KisImageSP image = new KisImage(new KisSurrogateUndoStore(), imageRect.width(), imageRect.height(), cs, "test image");
    doc->setCurrentImage(image);

    KisPaintLayerSP layer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8, weirdCS);
    image->addNode(layer1);

    KisColorizeMaskSP mask = new KisColorizeMask();
    image->addNode(mask, layer1);
    mask->initializeCompositeOp();
    delete mask->setColorSpace(layer1->colorSpace());

    {
        KisPaintDeviceSP key1 = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
        key1->fill(QRect(50,50,10,20), KoColor(Qt::black, key1->colorSpace()));
        mask->testingAddKeyStroke(key1, KoColor(Qt::green, layer1->colorSpace()));
        // KIS_DUMP_DEVICE_2(key1, refRect, "key1", "dd");
    }

    {
        KisPaintDeviceSP key2 = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
        key2->fill(QRect(150,50,10,20), KoColor(Qt::black, key2->colorSpace()));
        mask->testingAddKeyStroke(key2, KoColor(Qt::red, layer1->colorSpace()));
        // KIS_DUMP_DEVICE_2(key2, refRect, "key2", "dd");
    }

    {
        KisPaintDeviceSP key3 = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
        key3->fill(QRect(0,0,10,10), KoColor(Qt::black, key3->colorSpace()));
        mask->testingAddKeyStroke(key3, KoColor(Qt::blue, layer1->colorSpace()), true);
        // KIS_DUMP_DEVICE_2(key3, refRect, "key3", "dd");
    }

    KisLayerPropertiesIcons::setNodeProperty(mask, KisLayerPropertiesIcons::colorizeEditKeyStrokes, false, image);
    KisLayerPropertiesIcons::setNodeProperty(mask, KisLayerPropertiesIcons::colorizeShowColoring, false, image);



    doc->exportDocumentSync(QUrl::fromLocalFile("roundtrip_colorize.kra"), doc->mimeType());

    QScopedPointer<KisDocument> doc2(KisPart::instance()->createDocument());
    doc2->loadNativeFormat("roundtrip_colorize.kra");
    KisImageSP image2 = doc2->image();
    KisNodeSP node = image2->root()->firstChild()->firstChild();

    KisColorizeMaskSP mask2 = dynamic_cast<KisColorizeMask*>(node.data());
    QVERIFY(mask2);

    QCOMPARE(mask2->compositeOpId(), mask->compositeOpId());
    QCOMPARE(mask2->colorSpace(), mask->colorSpace());
    QCOMPARE(KisLayerPropertiesIcons::nodeProperty(mask, KisLayerPropertiesIcons::colorizeEditKeyStrokes, true).toBool(), false);
    QCOMPARE(KisLayerPropertiesIcons::nodeProperty(mask, KisLayerPropertiesIcons::colorizeShowColoring, true).toBool(), false);

    QList<KisLazyFillTools::KeyStroke> strokes = mask->fetchKeyStrokesDirect();

    qDebug() << ppVar(strokes.size());

    QCOMPARE(strokes[0].dev->exactBounds(), QRect(50,50,10,20));
    QCOMPARE(strokes[0].isTransparent, false);
    QCOMPARE(strokes[0].color.colorSpace(), weirdCS);

    QCOMPARE(strokes[1].dev->exactBounds(), QRect(150,50,10,20));
    QCOMPARE(strokes[1].isTransparent, false);
    QCOMPARE(strokes[1].color.colorSpace(), weirdCS);

    QCOMPARE(strokes[2].dev->exactBounds(), QRect(0,0,10,10));
    QCOMPARE(strokes[2].isTransparent, true);
    QCOMPARE(strokes[2].color.colorSpace(), weirdCS);
}

#include <KoColorBackground.h>

void KisKraSaverTest::testRoundTripShapeLayer()
{
    TestUtil::ReferenceImageChecker chk("kra_saver_test", "shape_layer");

    QRect refRect(0,0,512,512);

    QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());
    TestUtil::MaskParent p(refRect);

    const qreal resolution = 144.0 / 72.0;
    p.image->setResolution(resolution, resolution);

    doc->setCurrentImage(p.image);
    doc->documentInfo()->setAboutInfo("title", p.image->objectName());

    KoPathShape* path = new KoPathShape();
    path->setShapeId(KoPathShapeId);
    path->moveTo(QPointF(10, 10));
    path->lineTo(QPointF( 10, 110));
    path->lineTo(QPointF(110, 110));
    path->lineTo(QPointF(110,  10));
    path->close();
    path->normalize();
    path->setBackground(toQShared(new KoColorBackground(Qt::red)));

    path->setName("my_precious_shape");

    KisShapeLayerSP shapeLayer = new KisShapeLayer(doc->shapeController(), p.image, "shapeLayer1", 75);
    shapeLayer->addShape(path);
    p.image->addNode(shapeLayer);
    shapeLayer->setDirty();

    qApp->processEvents();
    p.image->waitForDone();

    chk.checkImage(p.image, "00_initial_layer_update");

    doc->exportDocumentSync(QUrl::fromLocalFile("roundtrip_shapelayer_test.kra"), doc->mimeType());

    QScopedPointer<KisDocument> doc2(KisPart::instance()->createDocument());
    doc2->loadNativeFormat("roundtrip_shapelayer_test.kra");

    qApp->processEvents();
    doc2->image()->waitForDone();
    QCOMPARE(doc2->image()->xRes(), resolution);
    QCOMPARE(doc2->image()->yRes(), resolution);
    chk.checkImage(doc2->image(), "01_shape_layer_round_trip");

    QVERIFY(chk.testPassed());
}

void KisKraSaverTest::testRoundTripShapeSelection()
{
    TestUtil::ReferenceImageChecker chk("kra_saver_test", "shape_selection");

    QRect refRect(0,0,512,512);

    QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());
    TestUtil::MaskParent p(refRect);
    doc->setCurrentImage(p.image);
    const qreal resolution = 144.0 / 72.0;
    p.image->setResolution(resolution, resolution);

    doc->setCurrentImage(p.image);
    doc->documentInfo()->setAboutInfo("title", p.image->objectName());

    p.layer->paintDevice()->setDefaultPixel(KoColor(Qt::green, p.layer->colorSpace()));

    KisSelectionSP selection = new KisSelection(p.layer->paintDevice()->defaultBounds());

    KisShapeSelection *shapeSelection = new KisShapeSelection(doc->shapeController(), p.image, selection);
    selection->setShapeSelection(shapeSelection);

    KoPathShape* path = new KoPathShape();
    path->setShapeId(KoPathShapeId);
    path->moveTo(QPointF(10, 10));
    path->lineTo(QPointF( 10, 110));
    path->lineTo(QPointF(110, 110));
    path->lineTo(QPointF(110,  10));
    path->close();
    path->normalize();
    path->setBackground(toQShared(new KoColorBackground(Qt::red)));
    path->setName("my_precious_shape");

    shapeSelection->addShape(path);

    KisTransparencyMaskSP tmask = new KisTransparencyMask();
    tmask->setSelection(selection);
    p.image->addNode(tmask, p.layer);

    tmask->setDirty(p.image->bounds());

    qApp->processEvents();
    p.image->waitForDone();

    chk.checkImage(p.image, "00_initial_shape_selection");

    doc->exportDocumentSync(QUrl::fromLocalFile("roundtrip_shapeselection_test.kra"), doc->mimeType());

    QScopedPointer<KisDocument> doc2(KisPart::instance()->createDocument());
    doc2->loadNativeFormat("roundtrip_shapeselection_test.kra");

    qApp->processEvents();
    doc2->image()->waitForDone();
    QCOMPARE(doc2->image()->xRes(), resolution);
    QCOMPARE(doc2->image()->yRes(), resolution);
    chk.checkImage(doc2->image(), "00_initial_shape_selection");

    KisNodeSP node = doc2->image()->root()->firstChild()->firstChild();
    KisTransparencyMask *newMask = dynamic_cast<KisTransparencyMask*>(node.data());
    QVERIFY(newMask);

    QVERIFY(newMask->selection()->hasShapeSelection());

    QVERIFY(chk.testPassed());
}

KISTEST_MAIN(KisKraSaverTest)
