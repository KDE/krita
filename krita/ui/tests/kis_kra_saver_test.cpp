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

#include <qtest_kde.h>

#include <QBitArray>

#include <KisDocument.h>
#include <KoDocumentInfo.h>
#include <KoShapeContainer.h>
#include <KoPathShape.h>

#include "filter/kis_filter_registry.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter.h"
#include "KisDocument.h"
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

#include "kis_transform_mask_params_interface.h"

#include <filter/kis_filter_registry.h>
#include <generator/kis_generator_registry.h>

void KisKraSaverTest::initTestCase()
{
    KisFilterRegistry::instance();
    KisGeneratorRegistry::instance();
}


void KisKraSaverTest::testRoundTrip()
{
    KisDocument* doc = createCompleteDocument();
    KoColor bgColor(Qt::red, doc->image()->colorSpace());
    doc->image()->setDefaultProjectionColor(bgColor);
    doc->saveNativeFormat("roundtriptest.kra");
    QStringList list;
    KisCountVisitor cv1(list, KoProperties());
    doc->image()->rootLayer()->accept(cv1);

    KisDocument *doc2 = KisPart::instance()->createDocument();

    doc2->loadNativeFormat("roundtriptest.kra");

    KisCountVisitor cv2(list, KoProperties());
    doc2->image()->rootLayer()->accept(cv2);
    QCOMPARE(cv1.count(), cv2.count());

    // check whether the BG color is saved correctly
    QCOMPARE(doc2->image()->defaultProjectionColor(), bgColor);

    // test round trip of a transform mask
    KisNodeSP tnode =
        TestUtil::findNode(doc2->image()->rootLayer(), "testTransformMask");
    QVERIFY(tnode);
    KisTransformMask *tmask = dynamic_cast<KisTransformMask*>(tnode.data());
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
    doc->saveNativeFormat("emptytest.kra");
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

#include <filter/kis_filter_configuration.h>
#include "generator/kis_generator_registry.h"
#include <generator/kis_generator.h>

void testRoundTripFillLayerImpl(const QString &testName, KisFilterConfiguration *config)
{
    TestUtil::ExternalImageChecker chk(testName, "fill_layer");

    QRect refRect(0,0,512,512);
    TestUtil::MaskParent p(refRect);

    QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());
    doc->setCurrentImage(p.image);
    doc->documentInfo()->setAboutInfo("title", p.image->objectName());

    KisSelectionSP selection;
    KisGeneratorLayerSP glayer = new KisGeneratorLayer(p.image, "glayer", config, selection);

    p.image->addNode(glayer, p.image->root(), KisNodeSP());
    glayer->setDirty();

    p.image->waitForDone();
    chk.checkImage(p.image, "00_initial_layer_update");

    doc->saveNativeFormat("roundtrip_fill_layer_test.kra");


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
    KisFilterConfiguration *config = generator->factoryConfiguration(0);
    Q_ASSERT(config);

    QVariant v;
    v.setValue(KoColor(Qt::red, cs));
    config->setProperty("color", v);

    testRoundTripFillLayerImpl("fill_layer_color", config);
}

void KisKraSaverTest::testRoundTripFillLayerPattern()
{
    KisGeneratorSP generator = KisGeneratorRegistry::instance()->get("pattern");
    Q_ASSERT(generator);

    // warning: we pass null paint device to the default constructed value
    KisFilterConfiguration *config = generator->factoryConfiguration(0);
    Q_ASSERT(config);

    QVariant v;
    v.setValue(QString("11_drawed_furry.png"));
    config->setProperty("pattern", v);

    testRoundTripFillLayerImpl("fill_layer_pattern", config);
}

#include "kis_psd_layer_style.h"


void KisKraSaverTest::testRoundTripLayerStyles()
{
    TestUtil::ExternalImageChecker chk("kra_saver_test", "layer_styles");

    QRect imageRect(0,0,512,512);

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(new KisSurrogateUndoStore(), imageRect.width(), imageRect.height(), cs, "test image");
    KisPaintLayerSP layer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
    KisPaintLayerSP layer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE_U8);
    KisPaintLayerSP layer3 = new KisPaintLayer(image, "paint3", OPACITY_OPAQUE_U8);
    image->addNode(layer1);
    image->addNode(layer2);
    image->addNode(layer3);

    QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());
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

    doc->saveNativeFormat("roundtrip_layer_styles.kra");


    QScopedPointer<KisDocument> doc2(KisPart::instance()->createDocument());
    doc2->loadNativeFormat("roundtrip_layer_styles.kra");

    doc2->image()->waitForDone();
    chk.checkImage(doc2->image(), "00_initial_layers");

    QVERIFY(chk.testPassed());
}

QTEST_KDEMAIN(KisKraSaverTest, GUI)
