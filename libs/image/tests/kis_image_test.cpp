/*
 *  SPDX-FileCopyrightText: 2005 Adrian Page <adrian@pagenet.plus.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_image_test.h"
#include <QApplication>
#include <QSignalSpy>

#include <simpletest.h>

#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>

#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"

#include "kis_image.h"
#include "kis_node_visitor.h"
#include "kis_external_layer_iface.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_selection.h"
#include <kis_debug.h>
#include <kis_layer_composition.h>
#include "kis_keyframe_channel.h"
#include "kis_selection_mask.h"
#include "kis_layer_utils.h"
#include "kis_annotation.h"
#include "KisProofingConfiguration.h"
#include <KisGlobalResourcesInterface.h>
#include "KisImageResolutionProxy.h"
#include <commands/kis_deselect_global_selection_command.h>
#include <commands/kis_reselect_global_selection_command.h>
#include <commands/kis_set_global_selection_command.h>

#include "kis_undo_stores.h"

#include <testimage.h>

#define IMAGE_WIDTH 128
#define IMAGE_HEIGHT 128

void KisImageTest::layerTests()
{
    KisImageSP image = new KisImage(0, IMAGE_WIDTH, IMAGE_WIDTH, 0, "layer tests");
    QVERIFY(image->rootLayer() != 0);
    QVERIFY(image->rootLayer()->firstChild() == 0);

    KisLayerSP layer = new KisPaintLayer(image, "layer 1", OPACITY_OPAQUE_U8);
    image->addNode(layer);

    QVERIFY(image->rootLayer()->firstChild()->objectName() == layer->objectName());
}

void KisImageTest::benchmarkCreation()
{
    const QRect imageRect(0,0,3000,2000);
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    QList<KisImageSP> images;
    QList<KisSurrogateUndoStore*> stores;


    QBENCHMARK {
        for (int i = 0; i < 10; i++) {
            stores << new KisSurrogateUndoStore();
        }

        for (int i = 0; i < 10; i++) {
            KisImageSP image = new KisImage(stores.takeLast(), imageRect.width(), imageRect.height(), cs, "test image");
            images << image;
        }
    }
}

#include <testutil.h>
#include "kis_stroke_strategy.h"
#include <functional>


class ForbiddenLodStrokeStrategy : public KisStrokeStrategy
{
public:
    ForbiddenLodStrokeStrategy(std::function<void()> lodCallback)
        : KisStrokeStrategy(QLatin1String("ForbiddenLodStrokeStrategy")),
          m_lodCallback(lodCallback)
    {
    }

    KisStrokeStrategy* createLodClone(int levelOfDetail) override {
        Q_UNUSED(levelOfDetail);
        m_lodCallback();
        return 0;
    }

private:
    std::function<void()> m_lodCallback;
};

void notifyVar(bool *value) {
    *value = true;
}

void testingSetOldDesiredLevelOfDetail(KisImageSP image, int lod)
{
    KisLodPreferences pref(lod);
    image->setLodPreferences(pref);
}

void testingSetOldLevelOfDetailBlocked(KisImageSP image, bool value)
{
    if (value) {
        testingSetOldDesiredLevelOfDetail(image, 0);
    } else {
        KisLodPreferences pref(KisLodPreferences::None, 0);
        image->setLodPreferences(pref);
    }
}

void KisImageTest::testBlockLevelOfDetail()
{
    TestUtil::MaskParent p;

    QCOMPARE(p.image->currentLevelOfDetail(), 0);

    testingSetOldDesiredLevelOfDetail(p.image, 1);
    p.image->waitForDone();

    QCOMPARE(p.image->currentLevelOfDetail(), 0);

    {
        bool lodCreated = false;
        KisStrokeId id = p.image->startStroke(
            new ForbiddenLodStrokeStrategy(
                std::bind(&notifyVar, &lodCreated)));
        p.image->endStroke(id);
        p.image->waitForDone();

        QVERIFY(lodCreated);
    }

    testingSetOldLevelOfDetailBlocked(p.image, true);

    {
        bool lodCreated = false;
        KisStrokeId id = p.image->startStroke(
            new ForbiddenLodStrokeStrategy(
                std::bind(&notifyVar, &lodCreated)));
        p.image->endStroke(id);
        p.image->waitForDone();

        QVERIFY(!lodCreated);
    }

    testingSetOldLevelOfDetailBlocked(p.image, false);
    testingSetOldDesiredLevelOfDetail(p.image, 1);

    {
        bool lodCreated = false;
        KisStrokeId id = p.image->startStroke(
            new ForbiddenLodStrokeStrategy(
                std::bind(&notifyVar, &lodCreated)));
        p.image->endStroke(id);
        p.image->waitForDone();

        QVERIFY(lodCreated);
    }
}

void KisImageTest::testConvertImageColorSpace()
{
    const KoColorSpace *cs8 = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 1000, 1000, cs8, "stest");

    KisPaintDeviceSP device1 = new KisPaintDevice(cs8);
    KisLayerSP paint1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8, device1);

    KisFilterSP filter = KisFilterRegistry::instance()->value("blur");
    Q_ASSERT(filter);
    KisFilterConfigurationSP configuration = filter->defaultConfiguration(KisGlobalResourcesInterface::instance());
    Q_ASSERT(configuration);

    KisLayerSP blur1 = new KisAdjustmentLayer(image, "blur1", configuration->cloneWithResourcesSnapshot(), 0);

    image->addNode(paint1, image->root());
    image->addNode(blur1, image->root());

    image->initialRefreshGraph();

    const KoColorSpace *cs16 = KoColorSpaceRegistry::instance()->rgb16();
    image->convertImageColorSpace(cs16,
                                  KoColorConversionTransformation::internalRenderingIntent(),
                                  KoColorConversionTransformation::internalConversionFlags());
    image->waitForDone();

    QVERIFY(*cs16 == *image->colorSpace());
    QVERIFY(*cs16 == *image->root()->colorSpace());
    QVERIFY(*cs16 == *paint1->colorSpace());
    QVERIFY(*cs16 == *blur1->colorSpace());

    QVERIFY(!image->root()->compositeOp());
    QVERIFY(*cs16 == *paint1->compositeOp()->colorSpace());
    QVERIFY(*cs16 == *blur1->compositeOp()->colorSpace());

    image->refreshGraphAsync();
    image->waitForDone();
}

void KisImageTest::testChannelFlagsAfterColorSpaceConversion_data()
{
    QTest::addColumn<QString>("layerType"); // "paint", "external", "group",
    QTest::addColumn<QString>("channelsMode"); // "empty", "inheritAlpha", "alphaOnly", "firstChannel"
    QTest::addColumn<QString>("imageColorSpaceModelMode"); // "src", "dst", "cmyk"
    QTest::addColumn<QString>("srcColorSpaceModel");
    QTest::addColumn<QString>("dstColorSpaceModel");
    QTest::addColumn<QBitArray>("expectedChannelFlags");

    auto strToBitArray = [] (const QString &str) {
        QBitArray result(str.size());
        for (auto it = str.begin(); it != str.end(); ++it) {
            result.setBit(std::distance(str.begin(), it), *it == '1');
        }
        return result;
    };

    /**
     * The general requirements for channel flags preservation on color space change:
     *
     * 1) Color channels are always reset into "all enabled" state
     * 2) If alpha channel was disabled, this disabled state will be preserved
     *    during the conversion stage.
     * 3) If all the channel are enabled after the conversion, then
     *    the channel flags object is reset into empty state
     */

    for (const char *imageColorSpaceModelMode : {"src", "dst", "cmyk"}) {
        for (const char *layerType : {"paint", "external", "group"}) {
            QTest::addRow("img_cs_%s-%s-rgb2gray-empty", imageColorSpaceModelMode, layerType)
                << layerType << "empty" << imageColorSpaceModelMode << RGBAColorModelID.id() << GrayAColorModelID.id()
                << strToBitArray("");
            QTest::addRow("img_cs_%s-%s-rgb2gray-inheritAlpha", imageColorSpaceModelMode, layerType)
                << layerType << "inheritAlpha" << imageColorSpaceModelMode << RGBAColorModelID.id()
                << GrayAColorModelID.id() << strToBitArray("10");
            QTest::addRow("img_cs_%s-%s-rgb2gray-alphaOnly", imageColorSpaceModelMode, layerType)
                << layerType << "alphaOnly" << imageColorSpaceModelMode << RGBAColorModelID.id()
                << GrayAColorModelID.id() << strToBitArray("");
            QTest::addRow("img_cs_%s-%s-rgb2gray-firstChannel", imageColorSpaceModelMode, layerType)
                << layerType << "firstChannel" << imageColorSpaceModelMode << RGBAColorModelID.id()
                << GrayAColorModelID.id() << strToBitArray("10");

            QTest::addRow("img_cs_%s-%s-gray2rgb-empty", imageColorSpaceModelMode, layerType)
                << layerType << "empty" << imageColorSpaceModelMode << GrayAColorModelID.id() << RGBAColorModelID.id()
                << strToBitArray("");
            QTest::addRow("img_cs_%s-%s-gray2rgb-inheritAlpha", imageColorSpaceModelMode, layerType)
                << layerType << "inheritAlpha" << imageColorSpaceModelMode << GrayAColorModelID.id()
                << RGBAColorModelID.id() << strToBitArray("1110");
            QTest::addRow("img_cs_%s-%s-gray2rgb-alphaOnly", imageColorSpaceModelMode, layerType)
                << layerType << "alphaOnly" << imageColorSpaceModelMode << GrayAColorModelID.id()
                << RGBAColorModelID.id() << strToBitArray("");
            QTest::addRow("img_cs_%s-%s-gray2rgb-firstChannel", imageColorSpaceModelMode, layerType)
                << layerType << "firstChannel" << imageColorSpaceModelMode << GrayAColorModelID.id()
                << RGBAColorModelID.id() << strToBitArray("1110");
        }
    }
}

namespace
{
class TestingExternalLayer : public KisExternalLayer
{
public:
    TestingExternalLayer(KisImageWSP image, const QString &name, quint8 opacity, const KoColorSpace *cs)
        : KisExternalLayer(image, name, opacity)
        , m_original(new KisPaintDevice(cs))
    {
    }

    KisNodeSP clone() const override
    {
        TestingExternalLayer *result = new TestingExternalLayer(image(), name(), opacity(), m_original->colorSpace());
        result->m_original->makeCloneFromRough(m_original, m_original->extent());
        return result;
    }

    bool allowAsChild(KisNodeSP) const override
    {
        return true;
    }

    KisPaintDeviceSP original() const override
    {
        return m_original;
    }

    KisPaintDeviceSP paintDevice() const override
    {
        return 0;
    }

    bool accept(KisNodeVisitor &visitor) override
    {
        return visitor.visit(this);
    }

    void accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter) override
    {
        return visitor.visit(this, undoAdapter);
    }

    KUndo2Command *setProfile(const KoColorProfile *profile) override
    {
        KUndo2Command *cmd = new KUndo2Command();
        m_original->setProfile(profile, cmd);

        return cmd;
    }

    KUndo2Command *convertTo(const KoColorSpace *dstColorSpace,
                             KoColorConversionTransformation::Intent renderingIntent,
                             KoColorConversionTransformation::ConversionFlags conversionFlags) override
    {
        KUndo2Command *cmd = new KUndo2Command();
        m_original->convertTo(dstColorSpace, renderingIntent, conversionFlags, cmd);
        return cmd;
    }

private:
    KisPaintDeviceSP m_original;
};

} // namespace

void KisImageTest::testChannelFlagsAfterColorSpaceConversion()
{
    QFETCH(QString, layerType);
    QFETCH(QString, channelsMode);
    QFETCH(QString, imageColorSpaceModelMode);
    QFETCH(QString, srcColorSpaceModel);
    QFETCH(QString, dstColorSpaceModel);
    QFETCH(QBitArray, expectedChannelFlags);

    QString imageColorSpaceModel;
    QString imageColorSpaceBitDepth = Integer8BitsColorDepthID.id();

    if (imageColorSpaceModelMode == "src") {
        imageColorSpaceModel = srcColorSpaceModel;
    } else if (imageColorSpaceModelMode == "dst") {
        // when converting into the same color model, make sure
        // we also change the bit depth, otherfise the whole
        // operation will be skipped
        imageColorSpaceModel = dstColorSpaceModel;
        imageColorSpaceBitDepth = Integer16BitsColorDepthID.id();
    } else if (imageColorSpaceModelMode == "cmyk") {
        imageColorSpaceModel = CMYKAColorModelID.id();
    } else {
        qFatal("Unknown image color space model testing mode: %s", imageColorSpaceModelMode.toLatin1().data());
    }

    const KoColorSpace *srcCS = KoColorSpaceRegistry::instance()->colorSpace(srcColorSpaceModel, Integer8BitsColorDepthID.id(), nullptr);
    const KoColorSpace *imageCS = KoColorSpaceRegistry::instance()->colorSpace(imageColorSpaceModel, imageColorSpaceBitDepth, nullptr);
    auto *undoStore = new KisSurrogateUndoStore();
    KisImageSP image = new KisImage(undoStore, 1000, 1000, imageCS, "stest");


    KisLayerSP layer1;

    if (layerType == "paint") {
        KisPaintDeviceSP device1 = new KisPaintDevice(srcCS);
        layer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8, device1);
    } else if (layerType == "external") {
        layer1 = new TestingExternalLayer(image, "external1", OPACITY_OPAQUE_U8, srcCS);
    } else if (layerType == "group") {
        layer1 = new KisGroupLayer(image, "group1", OPACITY_OPAQUE_U8, srcCS);
    } else {
        qFatal("Unknown layer type: %s", layerType.toLatin1().data());
    }

    image->addNode(layer1, image->root());

    const QBitArray initialChannelFlags = [&] () {
        if (channelsMode == "empty") {
            return QBitArray();
        } else if (channelsMode == "inheritAlpha") {
            return srcCS->channelFlags(true, false);
        } else if (channelsMode == "alphaOnly") {
            return srcCS->channelFlags(false, true);
        } else if (channelsMode == "firstChannel") {
            QBitArray channelFlags = srcCS->channelFlags(false, false);
            channelFlags.setBit(0, true);
            return channelFlags;
        } else {
            qFatal("Unknown channel flags testing mode!");
        }
        Q_UNREACHABLE_RETURN(QBitArray());
    }();

    layer1->setChannelFlags(initialChannelFlags);
    image->initialRefreshGraph();
    QCOMPARE(*layer1->colorSpace(), *srcCS);
    QCOMPARE(layer1->channelFlags(), initialChannelFlags);

    const KoColorSpace *dstCS = KoColorSpaceRegistry::instance()->colorSpace(dstColorSpaceModel, Integer8BitsColorDepthID.id(), nullptr);
    image->convertImageColorSpace(dstCS,
                                  KoColorConversionTransformation::internalRenderingIntent(),
                                  KoColorConversionTransformation::internalConversionFlags());
    image->waitForDone();
    QCOMPARE(*layer1->colorSpace(), *dstCS);
    QCOMPARE(layer1->channelFlags(), expectedChannelFlags);

    undoStore->undo();
    image->waitForDone();
    QCOMPARE(layer1->channelFlags(), initialChannelFlags);
}

void KisImageTest::testAssignImageProfile()
{
    const KoColorSpace *rgb8 = KoColorSpaceRegistry::instance()->rgb8();
    const KoColorSpace *gray8 = KoColorSpaceRegistry::instance()->graya8();
    KisImageSP image = new KisImage(0, 1000, 1000, rgb8, "stest");

    KisPaintDeviceSP device1 = new KisPaintDevice(rgb8);
    KisLayerSP paint1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8, device1);

    KisPaintDeviceSP device2 = new KisPaintDevice(gray8);
    KisLayerSP paint2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE_U8, device2);


    KisFilterSP filter = KisFilterRegistry::instance()->value("blur");
    Q_ASSERT(filter);
    KisFilterConfigurationSP configuration = filter->defaultConfiguration(KisGlobalResourcesInterface::instance());
    Q_ASSERT(configuration);

    KisLayerSP blur1 = new KisAdjustmentLayer(image, "blur1", configuration->cloneWithResourcesSnapshot(), 0);

    image->addNode(paint1, image->root());
    image->addNode(paint2, image->root());
    image->addNode(blur1, image->root());

    QCOMPARE(*image->colorSpace(), *rgb8);
    QCOMPARE(*image->colorSpace()->profile(), *KoColorSpaceRegistry::instance()->p709SRGBProfile());

    QCOMPARE(*paint1->colorSpace(), *rgb8);
    QCOMPARE(*paint1->colorSpace()->profile(), *KoColorSpaceRegistry::instance()->p709SRGBProfile());

    QCOMPARE(*paint2->colorSpace(), *gray8);

    QCOMPARE(*blur1->colorSpace(), *rgb8);
    QCOMPARE(*blur1->colorSpace()->profile(), *KoColorSpaceRegistry::instance()->p709SRGBProfile());


    image->assignImageProfile(KoColorSpaceRegistry::instance()->p2020G10Profile());
    image->waitForDone();

    QVERIFY(*image->colorSpace() != *rgb8);
    QCOMPARE(*image->colorSpace()->profile(), *KoColorSpaceRegistry::instance()->p2020G10Profile());

    QVERIFY(*paint1->colorSpace() != *rgb8);
    QCOMPARE(*paint1->colorSpace()->profile(), *KoColorSpaceRegistry::instance()->p2020G10Profile());

    QCOMPARE(*paint2->colorSpace(), *gray8);

    QVERIFY(*blur1->colorSpace() != *rgb8);
    QCOMPARE(*blur1->colorSpace()->profile(), *KoColorSpaceRegistry::instance()->p2020G10Profile());
}

void KisImageTest::testGlobalSelection()
{
    const KoColorSpace *cs8 = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 1000, 1000, cs8, "stest");

    QCOMPARE(image->globalSelection(), KisSelectionSP(0));
    QCOMPARE(image->canReselectGlobalSelection(), false);
    QCOMPARE(image->root()->childCount(), 0U);

    KisSelectionSP selection1 = new KisSelection(new KisDefaultBounds(image), toQShared(new KisImageResolutionProxy(image)));
    KisSelectionSP selection2 = new KisSelection(new KisDefaultBounds(image), toQShared(new KisImageResolutionProxy(image)));
    KisSelectionSP selection3 = new KisSelection(new KisDefaultBounds(image), toQShared(new KisImageResolutionProxy(image)));

    image->undoAdapter()->addCommand(new KisSetGlobalSelectionCommand(image, selection1));
    QCOMPARE(image->globalSelection(), selection1);
    QCOMPARE(image->canReselectGlobalSelection(), false);
    QCOMPARE(image->root()->childCount(), 1U);

    image->undoAdapter()->addCommand(new KisSetGlobalSelectionCommand(image, selection2));
    QCOMPARE(image->globalSelection(), selection2);
    QCOMPARE(image->canReselectGlobalSelection(), false);
    QCOMPARE(image->root()->childCount(), 1U);

    image->undoAdapter()->addCommand(new KisDeselectGlobalSelectionCommand(image));
    QCOMPARE(image->globalSelection(), KisSelectionSP(0));
    QCOMPARE(image->canReselectGlobalSelection(), true);
    QCOMPARE(image->root()->childCount(), 0U);

    image->undoAdapter()->addCommand(new KisReselectGlobalSelectionCommand(image));
    QCOMPARE(image->globalSelection(), selection2);
    QCOMPARE(image->canReselectGlobalSelection(), false);
    QCOMPARE(image->root()->childCount(), 1U);

    // mixed deselecting/setting/reselecting

    image->undoAdapter()->addCommand(new KisDeselectGlobalSelectionCommand(image));
    QCOMPARE(image->globalSelection(), KisSelectionSP(0));
    QCOMPARE(image->canReselectGlobalSelection(), true);
    QCOMPARE(image->root()->childCount(), 0U);

    image->undoAdapter()->addCommand(new KisSetGlobalSelectionCommand(image, selection3));
    QCOMPARE(image->globalSelection(), selection3);
    QCOMPARE(image->canReselectGlobalSelection(), false);
    QCOMPARE(image->root()->childCount(), 1U);
}

void KisImageTest::testCloneImage()
{
    KisImageSP image = new KisImage(0, IMAGE_WIDTH, IMAGE_WIDTH, 0, "layer tests");
    QVERIFY(image->rootLayer() != 0);
    QVERIFY(image->rootLayer()->firstChild() == 0);

    KisAnnotationSP annotation = new KisAnnotation("mytype", "mydescription", QByteArray());
    image->addAnnotation(annotation);
    QVERIFY(image->annotation("mytype"));

    KisProofingConfigurationSP proofing = toQShared(new KisProofingConfiguration());
    image->setProofingConfiguration(proofing);
    QVERIFY(image->proofingConfiguration());

    const KoColor defaultColor(Qt::green, image->colorSpace());
    image->setDefaultProjectionColor(defaultColor);
    QCOMPARE(image->defaultProjectionColor(), defaultColor);

    KisLayerSP layer = new KisPaintLayer(image, "layer1", OPACITY_OPAQUE_U8);
    image->addNode(layer);
    KisLayerSP layer2 = new KisPaintLayer(image, "layer2", OPACITY_OPAQUE_U8);
    image->addNode(layer2);

    QVERIFY(layer->visible());
    QVERIFY(layer2->visible());

    QVERIFY(TestUtil::findNode(image->root(), "layer1"));
    QVERIFY(TestUtil::findNode(image->root(), "layer2"));

    QUuid uuid1 = layer->uuid();
    QUuid uuid2 = layer2->uuid();

    {
        KisImageSP newImage = image->clone();

        KisNodeSP newLayer1 = TestUtil::findNode(newImage->root(), "layer1");
        KisNodeSP newLayer2 = TestUtil::findNode(newImage->root(), "layer2");

        QVERIFY(newLayer1);
        QVERIFY(newLayer2);

        QVERIFY(newLayer1->uuid() != uuid1);
        QVERIFY(newLayer2->uuid() != uuid2);

        KisAnnotationSP newAnnotation = newImage->annotation("mytype");
        QVERIFY(newAnnotation);
        QVERIFY(newAnnotation != annotation);


        KisProofingConfigurationSP newProofing = newImage->proofingConfiguration();
        QVERIFY(newProofing);
        QVERIFY(newProofing != proofing);

        QCOMPARE(newImage->defaultProjectionColor(), defaultColor);
    }

    {
        KisImageSP newImage = image->clone(true);

        KisNodeSP newLayer1 = TestUtil::findNode(newImage->root(), "layer1");
        KisNodeSP newLayer2 = TestUtil::findNode(newImage->root(), "layer2");

        QVERIFY(newLayer1);
        QVERIFY(newLayer2);

        QVERIFY(newLayer1->uuid() == uuid1);
        QVERIFY(newLayer2->uuid() == uuid2);
    }
}

void KisImageTest::testLayerComposition()
{
    KisImageSP image = new KisImage(0, IMAGE_WIDTH, IMAGE_WIDTH, 0, "layer tests");
    QVERIFY(image->rootLayer() != 0);
    QVERIFY(image->rootLayer()->firstChild() == 0);

    KisLayerSP layer = new KisPaintLayer(image, "layer1", OPACITY_OPAQUE_U8);
    image->addNode(layer);
    KisLayerSP layer2 = new KisPaintLayer(image, "layer2", OPACITY_OPAQUE_U8);
    image->addNode(layer2);

    QVERIFY(layer->visible());
    QVERIFY(layer2->visible());

    KisLayerComposition comp(image, "comp 1");
    comp.store();

    layer2->setVisible(false);

    QVERIFY(layer->visible());
    QVERIFY(!layer2->visible());

    KisLayerComposition comp2(image, "comp 2");
    comp2.store();

    KisLayerCompositionSP comp3 = toQShared(new KisLayerComposition(image, "comp 3"));
    comp3->store();
    image->addComposition(comp3);

    comp.apply();

    QVERIFY(layer->visible());
    QVERIFY(layer2->visible());

    comp2.apply();

    QVERIFY(layer->visible());
    QVERIFY(!layer2->visible());

    comp.apply();

    QVERIFY(layer->visible());
    QVERIFY(layer2->visible());

    KisImageSP newImage = image->clone();

    KisNodeSP newLayer1 = TestUtil::findNode(newImage->root(), "layer1");
    KisNodeSP newLayer2 = TestUtil::findNode(newImage->root(), "layer2");

    QVERIFY(newLayer1);
    QVERIFY(newLayer2);

    QVERIFY(newLayer1->visible());
    QVERIFY(newLayer2->visible());

    KisLayerComposition newComp1(comp, newImage);
    newComp1.apply();
    QVERIFY(newLayer1->visible());
    QVERIFY(newLayer2->visible());

    KisLayerComposition newComp2(comp2, newImage);
    newComp2.apply();
    QVERIFY(newLayer1->visible());
    QVERIFY(!newLayer2->visible());

    newComp1.apply();
    QVERIFY(newLayer1->visible());
    QVERIFY(newLayer2->visible());

    QVERIFY(!newImage->compositions().isEmpty());
    KisLayerCompositionSP newComp3 = newImage->compositions().first();
    newComp3->apply();
    QVERIFY(newLayer1->visible());
    QVERIFY(!newLayer2->visible());
}

#include "kis_transparency_mask.h"
#include "kis_psd_layer_style.h"

struct FlattenTestImage
{
    FlattenTestImage()
        : refRect(0,0,512,512)
        , p(refRect)
    {

        image = p.image;
        undoStore = p.undoStore;
        layer1 = p.layer;

        layer5 = new KisPaintLayer(p.image, "paint5", 0.4 * OPACITY_OPAQUE_U8);
        layer5->disableAlphaChannel(true);

        layer2 = new KisPaintLayer(p.image, "paint2", OPACITY_OPAQUE_U8);
        tmask = new KisTransparencyMask(p.image, "tmask");

        // check channel flags
        // make addition composite op
        group1 = new KisGroupLayer(p.image, "group1", OPACITY_OPAQUE_U8);
        layer3 = new KisPaintLayer(p.image, "paint3", OPACITY_OPAQUE_U8);
        layer4 = new KisPaintLayer(p.image, "paint4", OPACITY_OPAQUE_U8);

        layer6 = new KisPaintLayer(p.image, "paint6", OPACITY_OPAQUE_U8);

        layer7 = new KisPaintLayer(p.image, "paint7", OPACITY_OPAQUE_U8);
        layer8 = new KisPaintLayer(p.image, "paint8", OPACITY_OPAQUE_U8);
        layer7->setCompositeOpId(COMPOSITE_ADD);
        layer8->setCompositeOpId(COMPOSITE_ADD);

        QRect rect1(100, 100, 100, 100);
        QRect rect2(150, 150, 150, 150);
        QRect tmaskRect(200,200,100,100);

        QRect rect3(400, 100, 100, 100);
        QRect rect4(500, 100, 100, 100);

        QRect rect5(50, 50, 100, 100);

        QRect rect6(50, 250, 100, 100);

        QRect rect7(50, 350, 50, 50);
        QRect rect8(50, 400, 50, 50);

        layer1->paintDevice()->fill(rect1, KoColor(Qt::red, p.image->colorSpace()));

        layer2->paintDevice()->fill(rect2, KoColor(Qt::green, p.image->colorSpace()));
        tmask->testingInitSelection(tmaskRect, layer2);

        layer3->paintDevice()->fill(rect3, KoColor(Qt::blue, p.image->colorSpace()));
        layer4->paintDevice()->fill(rect4, KoColor(Qt::yellow, p.image->colorSpace()));
        layer5->paintDevice()->fill(rect5, KoColor(Qt::green, p.image->colorSpace()));

        layer6->paintDevice()->fill(rect6, KoColor(Qt::cyan, p.image->colorSpace()));

        layer7->paintDevice()->fill(rect7, KoColor(Qt::red, p.image->colorSpace()));
        layer8->paintDevice()->fill(rect8, KoColor(Qt::green, p.image->colorSpace()));

        KisPSDLayerStyleSP style(new KisPSDLayerStyle());
        style->dropShadow()->setEffectEnabled(true);
        style->dropShadow()->setDistance(10.0);
        style->dropShadow()->setSpread(80.0);
        style->dropShadow()->setSize(10);
        style->dropShadow()->setNoise(0);
        style->dropShadow()->setKnocksOut(false);
        style->dropShadow()->setOpacity(80.0);
        layer2->setLayerStyle(style);

        layer2->setCompositeOpId(COMPOSITE_ADD);
        group1->setCompositeOpId(COMPOSITE_ADD);

        p.image->addNode(layer5);

        p.image->addNode(layer2);
        p.image->addNode(tmask, layer2);

        p.image->addNode(group1);
        p.image->addNode(layer3, group1);
        p.image->addNode(layer4, group1);

        p.image->addNode(layer6);

        p.image->addNode(layer7);
        p.image->addNode(layer8);

        p.image->initialRefreshGraph();

        // dbgKrita << ppVar(layer1->exactBounds());
        // dbgKrita << ppVar(layer5->exactBounds());
        // dbgKrita << ppVar(layer2->exactBounds());
        // dbgKrita << ppVar(group1->exactBounds());
        // dbgKrita << ppVar(layer3->exactBounds());
        // dbgKrita << ppVar(layer4->exactBounds());

        TestUtil::ReferenceImageChecker chk("flatten", "imagetest");
        QVERIFY(chk.checkDevice(p.image->projection(), p.image, "00_initial"));
    }

    QRect refRect;
    TestUtil::MaskParent p;

    KisImageSP image;
    KisSurrogateUndoStore *undoStore;
    KisPaintLayerSP layer1;

    KisPaintLayerSP layer2;
    KisTransparencyMaskSP tmask;

    KisGroupLayerSP group1;
    KisPaintLayerSP layer3;
    KisPaintLayerSP layer4;

    KisPaintLayerSP layer5;

    KisPaintLayerSP layer6;

    KisPaintLayerSP layer7;
    KisPaintLayerSP layer8;
};

template<class ContainerTest>
KisLayerSP flattenLayerHelper(ContainerTest &p, KisLayerSP layer, bool nothingHappens = false)
{
    QSignalSpy spy(p.image.data(), SIGNAL(sigNodeAddedAsync(KisNodeSP, KisNodeAdditionFlags)));

    //p.image->flattenLayer(layer);
    KisLayerUtils::flattenLayer(p.image, layer);
    p.image->waitForDone();

    if (nothingHappens) {
        Q_ASSERT(!spy.count());
        return layer;
    }

    Q_ASSERT(spy.count() == 1);
    QList<QVariant> arguments = spy.takeFirst();
    KisNodeSP newNode = arguments.first().value<KisNodeSP>();

    KisLayerSP newLayer = qobject_cast<KisLayer*>(newNode.data());
    return newLayer;
}

void KisImageTest::testFlattenLayer()
{
    FlattenTestImage p;

    TestUtil::ReferenceImageChecker chk("flatten", "imagetest");

    {
        QCOMPARE(p.layer2->compositeOpId(), COMPOSITE_ADD);

        KisLayerSP newLayer = flattenLayerHelper(p, p.layer2);

        //KisLayerSP newLayer = p.image->flattenLayer(p.layer2);
        //p.image->waitForDone();

        QVERIFY(chk.checkDevice(p.image->projection(), p.image, "00_initial"));
        QVERIFY(chk.checkDevice(newLayer->projection(), p.image, "01_layer2_layerproj"));

        QCOMPARE(newLayer->compositeOpId(), COMPOSITE_OVER);
    }

    {
        QCOMPARE(p.group1->compositeOpId(), COMPOSITE_ADD);

        KisLayerSP newLayer = flattenLayerHelper(p, p.group1);

        //KisLayerSP newLayer = p.image->flattenLayer(p.group1);
        //p.image->waitForDone();

        QVERIFY(chk.checkDevice(p.image->projection(), p.image, "00_initial"));
        QVERIFY(chk.checkDevice(newLayer->projection(), p.image, "02_group1_layerproj"));

        QCOMPARE(newLayer->compositeOpId(), COMPOSITE_ADD);
        QCOMPARE(newLayer->exactBounds(), QRect(400, 100, 200, 100));
    }

    {
        QCOMPARE(p.layer5->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(p.layer5->alphaChannelDisabled(), true);

        KisLayerSP newLayer = flattenLayerHelper(p, p.layer5, true);

        //KisLayerSP newLayer = p.image->flattenLayer(p.layer5);
        //p.image->waitForDone();

        QVERIFY(chk.checkDevice(p.image->projection(), p.image, "00_initial"));
        QVERIFY(chk.checkDevice(newLayer->projection(), p.image, "03_layer5_layerproj"));

        QCOMPARE(newLayer->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(newLayer->exactBounds(), QRect(50, 50, 100, 100));

        QCOMPARE(newLayer->alphaChannelDisabled(), true);
    }
}

#include <kis_meta_data_merge_strategy_registry.h>

template<class ContainerTest>
KisLayerSP mergeHelper(ContainerTest &p, KisLayerSP layer)
{
    KisNodeSP parent = layer->parent();
    const int newIndex = parent->index(layer) - 1;

    p.image->mergeDown(layer, KisMetaData::MergeStrategyRegistry::instance()->get("Drop"));

    //KisLayerUtils::mergeDown(p.image, layer, KisMetaData::MergeStrategyRegistry::instance()->get("Drop"));
    p.image->waitForDone();

    KisLayerSP newLayer = qobject_cast<KisLayer*>(parent->at(newIndex).data());
    return newLayer;
}

void KisImageTest::testMergeDown()
{
    FlattenTestImage p;

    TestUtil::ReferenceImageChecker img("flatten", "imagetest");
    TestUtil::ReferenceImageChecker chk("mergedown_simple", "imagetest");


    {
        QCOMPARE(p.layer5->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(p.layer5->alphaChannelDisabled(), true);

        KisLayerSP newLayer = mergeHelper(p, p.layer5);

        QVERIFY(img.checkDevice(p.image->projection(), p.image, "00_initial"));
        QVERIFY(chk.checkDevice(newLayer->projection(), p.image, "01_layer5_layerproj"));

        QCOMPARE(newLayer->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(newLayer->alphaChannelDisabled(), false);
    }

    {
        QCOMPARE(p.layer2->compositeOpId(), COMPOSITE_ADD);
        QCOMPARE(p.layer2->alphaChannelDisabled(), false);

        KisLayerSP newLayer = mergeHelper(p, p.layer2);

        QVERIFY(img.checkDevice(p.image->projection(), p.image, "00_initial"));
        QVERIFY(chk.checkDevice(newLayer->projection(), p.image, "02_layer2_layerproj"));

        QCOMPARE(newLayer->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(newLayer->exactBounds(), QRect(100, 100, 213, 217));
        QCOMPARE(newLayer->alphaChannelDisabled(), false);
    }

    {
        QCOMPARE(p.group1->compositeOpId(), COMPOSITE_ADD);
        QCOMPARE(p.group1->alphaChannelDisabled(), false);

        KisLayerSP newLayer = mergeHelper(p, p.group1);

        QVERIFY(img.checkDevice(p.image->projection(), p.image, "00_initial"));
        QVERIFY(chk.checkDevice(newLayer->projection(), p.image, "03_group1_mergedown_layerproj"));

        QCOMPARE(newLayer->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(newLayer->exactBounds(), QRect(100, 100, 500, 217));
        QCOMPARE(newLayer->alphaChannelDisabled(), false);
    }
}

void KisImageTest::testMergeDownDestinationInheritsAlpha()
{
    FlattenTestImage p;

    TestUtil::ReferenceImageChecker img("flatten", "imagetest");
    TestUtil::ReferenceImageChecker chk("mergedown_dst_inheritsalpha", "imagetest");

    {
        QCOMPARE(p.layer2->compositeOpId(), COMPOSITE_ADD);
        QCOMPARE(p.layer2->alphaChannelDisabled(), false);

        KisLayerSP newLayer = mergeHelper(p, p.layer2);

        // WARN: this check is suspicious!
        QVERIFY(img.checkDevice(p.image->projection(), p.image, "00_proj_merged_layer2_over_layer5_IA"));
        QVERIFY(chk.checkDevice(newLayer->projection(), p.image, "01_layer2_layerproj"));

        QCOMPARE(newLayer->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(newLayer->exactBounds(), QRect(50,50, 263, 267));
        QCOMPARE(newLayer->alphaChannelDisabled(), false);
    }
}

void KisImageTest::testMergeDownDestinationCustomCompositeOp()
{
    FlattenTestImage p;

    TestUtil::ReferenceImageChecker img("flatten", "imagetest");
    TestUtil::ReferenceImageChecker chk("mergedown_dst_customop", "imagetest");

    {
        QCOMPARE(p.layer6->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(p.layer6->alphaChannelDisabled(), false);

        QCOMPARE(p.group1->compositeOpId(), COMPOSITE_ADD);
        QCOMPARE(p.group1->alphaChannelDisabled(), false);

        KisLayerSP newLayer = mergeHelper(p, p.layer6);

        QVERIFY(img.checkDevice(p.image->projection(), p.image, "00_initial"));
        QVERIFY(chk.checkDevice(newLayer->projection(), p.image, "01_layer6_layerproj"));

        QCOMPARE(newLayer->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(newLayer->exactBounds(), QRect(50, 100, 550, 250));
        QCOMPARE(newLayer->alphaChannelDisabled(), false);
    }
}

void KisImageTest::testMergeDownDestinationSameCompositeOpLayerStyle()
{
    FlattenTestImage p;

    TestUtil::ReferenceImageChecker img("flatten", "imagetest");
    TestUtil::ReferenceImageChecker chk("mergedown_sameop_ls", "imagetest");

    {
        QCOMPARE(p.group1->compositeOpId(), COMPOSITE_ADD);
        QCOMPARE(p.group1->alphaChannelDisabled(), false);

        QCOMPARE(p.layer2->compositeOpId(), COMPOSITE_ADD);
        QCOMPARE(p.layer2->alphaChannelDisabled(), false);

        KisLayerSP newLayer = mergeHelper(p, p.group1);

        QVERIFY(img.checkDevice(p.image->projection(), p.image, "00_initial"));
        QVERIFY(chk.checkDevice(newLayer->projection(), p.image, "01_group1_layerproj"));

        QCOMPARE(newLayer->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(newLayer->exactBounds(), QRect(197, 100, 403, 217));
        QCOMPARE(newLayer->alphaChannelDisabled(), false);
    }
}

void KisImageTest::testMergeDownDestinationSameCompositeOp()
{
    FlattenTestImage p;

    TestUtil::ReferenceImageChecker img("flatten", "imagetest");
    TestUtil::ReferenceImageChecker chk("mergedown_sameop_fastpath", "imagetest");

    {
        QCOMPARE(p.layer8->compositeOpId(), COMPOSITE_ADD);
        QCOMPARE(p.layer8->alphaChannelDisabled(), false);

        QCOMPARE(p.layer7->compositeOpId(), COMPOSITE_ADD);
        QCOMPARE(p.layer7->alphaChannelDisabled(), false);

        KisLayerSP newLayer = mergeHelper(p, p.layer8);

        QVERIFY(img.checkDevice(p.image->projection(), p.image, "00_initial"));
        QVERIFY(chk.checkDevice(newLayer->projection(), p.image, "01_layer8_layerproj"));

        QCOMPARE(newLayer->compositeOpId(), COMPOSITE_ADD);
        QCOMPARE(newLayer->exactBounds(), QRect(50, 350, 50, 100));
        QCOMPARE(newLayer->alphaChannelDisabled(), false);
    }
}
#include "kis_image_animation_interface.h"
void KisImageTest::testMergeDownMultipleFrames()
{
    FlattenTestImage p;

    TestUtil::ReferenceImageChecker img("flatten", "imagetest");
    TestUtil::ReferenceImageChecker chk("mergedown_simple", "imagetest");

    QSet<int> initialFrames;
    {
        KisLayerSP l = p.layer5;
        l->enableAnimation();
        KisKeyframeChannel *channel = l->getKeyframeChannel(KisKeyframeChannel::Raster.id(), true);
        channel->addKeyframe(10);
        channel->addKeyframe(20);
        channel->addKeyframe(30);

        QCOMPARE(channel->keyframeCount(), 4);
        initialFrames = KisLayerUtils::fetchLayerFramesRecursive(l);
        QCOMPARE(initialFrames.size(), 4);
    }

    {
        QCOMPARE(p.layer5->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(p.layer5->alphaChannelDisabled(), true);

        KisLayerSP newLayer = mergeHelper(p, p.layer5);

        QVERIFY(img.checkDevice(p.image->projection(), p.image, "00_initial"));
        QVERIFY(chk.checkDevice(newLayer->projection(), p.image, "01_layer5_layerproj"));

        QCOMPARE(newLayer->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(newLayer->alphaChannelDisabled(), false);

        QVERIFY(newLayer->isAnimated());

        QSet<int> newFrames = KisLayerUtils::fetchLayerFramesRecursive(newLayer);
        QCOMPARE(newFrames, initialFrames);

        foreach (int frame, newFrames) {
            KisImageAnimationInterface *interface = p.image->animationInterface();
            int savedSwitchedTime = 0;
            interface->saveAndResetCurrentTime(frame, &savedSwitchedTime);
            QCOMPARE(newLayer->exactBounds(), QRect(100,100,100,100));
            interface->restoreCurrentTime(&savedSwitchedTime);
        }

        p.undoStore->undo();
        p.image->waitForDone();

         QVERIFY(img.checkDevice(p.image->projection(), p.image, "00_initial"));
    }
}

template<class ContainerTest>
KisNodeSP mergeMultipleHelper(ContainerTest &p, QList<KisNodeSP> selectedNodes, KisNodeSP putAfter)
{
    QSignalSpy spy(p.image.data(), SIGNAL(sigNodeAddedAsync(KisNodeSP, KisNodeAdditionFlags)));

    p.image->mergeMultipleLayers(selectedNodes, putAfter);
    //KisLayerUtils::mergeMultipleLayers(p.image, selectedNodes, putAfter);
    p.image->waitForDone();

    Q_ASSERT(spy.count() == 1);
    QList<QVariant> arguments = spy.takeFirst();
    KisNodeSP newNode = arguments.first().value<KisNodeSP>();
    return newNode;
}
void KisImageTest::testMergeMultiple()
{
    FlattenTestImage p;

    TestUtil::ReferenceImageChecker img("flatten", "imagetest");
    TestUtil::ReferenceImageChecker chk("mergemultiple", "imagetest");

    {
        QList<KisNodeSP> selectedNodes;

        selectedNodes << p.layer2
                      << p.group1
                      << p.layer6;

        {
            KisNodeSP newLayer = mergeMultipleHelper(p, selectedNodes, 0);

            //KisNodeSP newLayer = p.image->mergeMultipleLayers(selectedNodes, 0);
            //p.image->waitForDone();

            QVERIFY(img.checkDevice(p.image->projection(), p.image, "00_initial"));
            QVERIFY(chk.checkDevice(newLayer->projection(), p.image, "01_layer8_layerproj"));

            QCOMPARE(newLayer->compositeOpId(), COMPOSITE_OVER);
            QCOMPARE(newLayer->exactBounds(), QRect(50, 100, 550, 250));
        }
    }

    p.p.undoStore->undo();
    p.image->waitForDone();


    // Test reversed order, the result must be the same

    {
        QList<KisNodeSP> selectedNodes;

        selectedNodes << p.layer6
                      << p.group1
                      << p.layer2;

        {
            KisNodeSP newLayer = mergeMultipleHelper(p, selectedNodes, 0);

            //KisNodeSP newLayer = p.image->mergeMultipleLayers(selectedNodes, 0);
            //p.image->waitForDone();

            QVERIFY(img.checkDevice(p.image->projection(), p.image, "00_initial"));
            QVERIFY(chk.checkDevice(newLayer->projection(), p.image, "01_layer8_layerproj"));

            QCOMPARE(newLayer->compositeOpId(), COMPOSITE_OVER);
            QCOMPARE(newLayer->exactBounds(), QRect(50, 100, 550, 250));
        }
    }

}

void testMergeCrossColorSpaceImpl(bool useProjectionColorSpace, bool swapSpaces)
{
    TestUtil::MaskParent p;

    KisPaintLayerSP layer1;
    KisPaintLayerSP layer2;
    KisPaintLayerSP layer3;

    const KoColorSpace *cs2 = useProjectionColorSpace ?
        p.image->colorSpace() : KoColorSpaceRegistry::instance()->lab16();

    const KoColorSpace *cs3 = KoColorSpaceRegistry::instance()->rgb16();

    if (swapSpaces) {
        std::swap(cs2, cs3);
    }

    dbgKrita << "Testing testMergeCrossColorSpaceImpl:";
    dbgKrita << "    " << ppVar(cs2);
    dbgKrita << "    " << ppVar(cs3);

    layer1 = p.layer;
    layer2 = new KisPaintLayer(p.image, "paint2", OPACITY_OPAQUE_U8, cs2);
    layer3 = new KisPaintLayer(p.image, "paint3", OPACITY_OPAQUE_U8, cs3);

    QRect rect1(100, 100, 100, 100);
    QRect rect2(150, 150, 150, 150);
    QRect rect3(250, 250, 200, 200);

    layer1->paintDevice()->fill(rect1, KoColor(Qt::red, layer1->colorSpace()));
    layer2->paintDevice()->fill(rect2, KoColor(Qt::green, layer2->colorSpace()));
    layer3->paintDevice()->fill(rect3, KoColor(Qt::blue, layer3->colorSpace()));

    p.image->addNode(layer2);
    p.image->addNode(layer3);

    p.image->initialRefreshGraph();

    {
        KisLayerSP newLayer = mergeHelper(p, layer3);

        QCOMPARE(newLayer->colorSpace(), p.image->colorSpace());

        p.undoStore->undo();
        p.image->waitForDone();
    }

    {
        layer2->disableAlphaChannel(true);

        KisLayerSP newLayer = mergeHelper(p, layer3);

        QCOMPARE(newLayer->colorSpace(), p.image->colorSpace());

        p.undoStore->undo();
        p.image->waitForDone();
    }
}

void KisImageTest::testMergeCrossColorSpace()
{
    testMergeCrossColorSpaceImpl(true, false);
    testMergeCrossColorSpaceImpl(true, true);
    testMergeCrossColorSpaceImpl(false, false);
    testMergeCrossColorSpaceImpl(false, true);
}

void KisImageTest::testMergeSelectionMasks()
{
    TestUtil::MaskParent p;

    QRect rect1(100, 100, 100, 100);
    QRect rect2(150, 150, 150, 150);
    QRect rect3(50, 50, 100, 100);

    KisPaintLayerSP layer1 = p.layer;
    layer1->paintDevice()->fill(rect1, KoColor(Qt::red, layer1->colorSpace()));

    p.image->initialRefreshGraph();

    KisSelectionSP sel = new KisSelection(layer1->paintDevice()->defaultBounds(), toQShared(new KisImageResolutionProxy(p.image)));

    sel->pixelSelection()->select(rect2, MAX_SELECTED);
    KisSelectionMaskSP mask1 = new KisSelectionMask(p.image);
    mask1->initSelection(sel, layer1);
    p.image->addNode(mask1, layer1);

    QVERIFY(!layer1->selection());

    mask1->setActive(true);

    QCOMPARE(layer1->selection()->selectedExactRect(), QRect(150,150,150,150));

    sel->pixelSelection()->select(rect3, MAX_SELECTED);
    KisSelectionMaskSP mask2 = new KisSelectionMask(p.image);
    mask2->initSelection(sel, layer1);
    p.image->addNode(mask2, layer1);

    QCOMPARE(layer1->selection()->selectedExactRect(), QRect(150,150,150,150));

    mask2->setActive(true);

    QCOMPARE(layer1->selection()->selectedExactRect(), QRect(50,50,250,250));

    QList<KisNodeSP> selectedNodes;

    selectedNodes << mask2
                  << mask1;

    {
        KisNodeSP newLayer = mergeMultipleHelper(p, selectedNodes, 0);
        QCOMPARE(newLayer->parent(), KisNodeSP(layer1));
        QCOMPARE((int)layer1->childCount(), 1);
        QCOMPARE(layer1->selection()->selectedExactRect(), QRect(50,50,250,250));
    }
}

void KisImageTest::testFlattenImage()
{
    FlattenTestImage p;
    KisImageSP image = p.image;

    TestUtil::ReferenceImageChecker img("flatten", "imagetest");

    {
        KisLayerUtils::flattenImage(p.image, 0);
        p.image->waitForDone();
        QVERIFY(img.checkDevice(p.image->projection(), p.image, "00_initial"));

        p.undoStore->undo();
        p.image->waitForDone();

        QVERIFY(img.checkDevice(p.image->projection(), p.image, "00_initial"));
    }

    {
        KisLayerUtils::flattenImage(p.image, p.layer5); // flatten with active layer just under the root (not inside any group)
        p.image->waitForDone();
        QVERIFY(img.checkDevice(p.image->projection(), p.image, "00_initial"));

        p.undoStore->undo();
        p.image->waitForDone();

        QVERIFY(img.checkDevice(p.image->projection(), p.image, "00_initial"));
    }

    {
        KisLayerUtils::flattenImage(p.image, p.layer2); // flatten with active layer just under the root (not inside any group), but with a mask
        p.image->waitForDone();
        QVERIFY(img.checkDevice(p.image->projection(), p.image, "00_initial"));

        p.undoStore->undo();
        p.image->waitForDone();

        QVERIFY(img.checkDevice(p.image->projection(), p.image, "00_initial"));
    }

    {
        KisLayerUtils::flattenImage(p.image, p.layer3); // flatten with active layer inside of a group
        p.image->waitForDone();
        QVERIFY(img.checkDevice(p.image->projection(), p.image, "00_initial"));

        p.undoStore->undo();
        p.image->waitForDone();

        QVERIFY(img.checkDevice(p.image->projection(), p.image, "00_initial"));
    }
}

struct FlattenPassThroughTestImage
{
    FlattenPassThroughTestImage()
        : refRect(0,0,512,512)
        , p(refRect)
    {

        image = p.image;
        undoStore = p.undoStore;

        group1 = new KisGroupLayer(p.image, "group1", OPACITY_OPAQUE_U8);
        layer2 = new KisPaintLayer(p.image, "paint2", OPACITY_OPAQUE_U8);
        layer3 = new KisPaintLayer(p.image, "paint3", OPACITY_OPAQUE_U8);

        group4 = new KisGroupLayer(p.image, "group4", OPACITY_OPAQUE_U8);
        layer5 = new KisPaintLayer(p.image, "paint5", OPACITY_OPAQUE_U8);
        layer6 = new KisPaintLayer(p.image, "paint6", OPACITY_OPAQUE_U8);

        QRect rect2(100, 100, 100, 100);
        QRect rect3(150, 150, 100, 100);

        QRect rect5(200, 200, 100, 100);
        QRect rect6(250, 250, 100, 100);

        group1->setPassThroughMode(true);
        layer2->paintDevice()->fill(rect2, KoColor(Qt::red, p.image->colorSpace()));
        layer3->paintDevice()->fill(rect3, KoColor(Qt::green, p.image->colorSpace()));

        group4->setPassThroughMode(true);
        layer5->paintDevice()->fill(rect5, KoColor(Qt::blue, p.image->colorSpace()));
        layer6->paintDevice()->fill(rect6, KoColor(Qt::yellow, p.image->colorSpace()));


        p.image->addNode(group1);
        p.image->addNode(layer2, group1);
        p.image->addNode(layer3, group1);

        p.image->addNode(group4);
        p.image->addNode(layer5, group4);
        p.image->addNode(layer6, group4);

        p.image->initialRefreshGraph();

        TestUtil::ReferenceImageChecker chk("passthrough", "imagetest");
        QVERIFY(chk.checkDevice(p.image->projection(), p.image, "00_initial"));
    }

    QRect refRect;
    TestUtil::MaskParent p;

    KisImageSP image;
    KisSurrogateUndoStore *undoStore;

    KisGroupLayerSP group1;
    KisPaintLayerSP layer2;
    KisPaintLayerSP layer3;

    KisGroupLayerSP group4;
    KisPaintLayerSP layer5;
    KisPaintLayerSP layer6;
};

void KisImageTest::testFlattenPassThroughLayer()
{
    FlattenPassThroughTestImage p;

    TestUtil::ReferenceImageChecker chk("passthrough", "imagetest");

    {
        QCOMPARE(p.group1->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(p.group1->passThroughMode(), true);

        KisLayerSP newLayer = flattenLayerHelper(p, p.group1);

        QVERIFY(chk.checkDevice(p.image->projection(), p.image, "00_initial"));
        QVERIFY(chk.checkDevice(newLayer->projection(), p.image, "01_group1_layerproj"));

        QCOMPARE(newLayer->compositeOpId(), COMPOSITE_OVER);
        QVERIFY(newLayer->inherits("KisPaintLayer"));
    }
}

void KisImageTest::testMergeTwoPassThroughLayers()
{
    FlattenPassThroughTestImage p;

    TestUtil::ReferenceImageChecker chk("passthrough", "imagetest");

    {
        QCOMPARE(p.group1->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(p.group1->passThroughMode(), true);

        KisLayerSP newLayer = mergeHelper(p, p.group4);

        QVERIFY(chk.checkDevice(p.image->projection(), p.image, "00_initial"));

        QCOMPARE(newLayer->compositeOpId(), COMPOSITE_OVER);
        QVERIFY(newLayer->inherits("KisGroupLayer"));
    }
}

void KisImageTest::testMergePaintOverPassThroughLayer()
{
    FlattenPassThroughTestImage p;

    TestUtil::ReferenceImageChecker chk("passthrough", "imagetest");

    {
        QCOMPARE(p.group1->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(p.group1->passThroughMode(), true);

        KisLayerSP newLayer = flattenLayerHelper(p, p.group4);
        QVERIFY(chk.checkDevice(p.image->projection(), p.image, "00_initial"));
        QVERIFY(newLayer->inherits("KisPaintLayer"));

        newLayer = mergeHelper(p, newLayer);
        QVERIFY(chk.checkDevice(p.image->projection(), p.image, "00_initial"));
        QVERIFY(newLayer->inherits("KisPaintLayer"));
    }
}

void KisImageTest::testMergePassThroughOverPaintLayer()
{
    FlattenPassThroughTestImage p;

    TestUtil::ReferenceImageChecker chk("passthrough", "imagetest");

    {
        QCOMPARE(p.group1->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(p.group1->passThroughMode(), true);

        KisLayerSP newLayer = flattenLayerHelper(p, p.group1);
        QVERIFY(chk.checkDevice(p.image->projection(), p.image, "00_initial"));
        QVERIFY(newLayer->inherits("KisPaintLayer"));

        newLayer = mergeHelper(p, p.group4);
        QVERIFY(chk.checkDevice(p.image->projection(), p.image, "00_initial"));
        QVERIFY(newLayer->inherits("KisPaintLayer"));
    }
}

#include "kis_paint_device_debug_utils.h"
#include "kis_algebra_2d.h"

void KisImageTest::testPaintOverlayMask()
{
    QRect refRect(0, 0, 512, 512);
    TestUtil::MaskParent p(refRect);

    QRect fillRect(50, 50, 412, 412);
    QRect selectionRect(200, 200, 100, 50);

    KisPaintLayerSP layer1 = p.layer;
    layer1->paintDevice()->fill(fillRect, KoColor(Qt::yellow, layer1->colorSpace()));

    KisSelectionMaskSP mask = new KisSelectionMask(p.image);
    KisSelectionSP selection = new KisSelection(new KisMaskDefaultBounds(layer1), toQShared(new KisImageResolutionProxy(p.image)));

    selection->pixelSelection()->select(selectionRect, 128);
    selection->pixelSelection()->select(KisAlgebra2D::blowRect(selectionRect,-0.3), 255);

    mask->setSelection(selection);

    //mask->setVisible(false);
    //mask->setActive(false);

    p.image->addNode(mask, layer1);

    // a simple layer to disable oblige-child mechanism
    KisPaintLayerSP layer2 = new KisPaintLayer(p.image, "layer2", OPACITY_OPAQUE_U8);
    p.image->addNode(layer2);

    p.image->initialRefreshGraph();

    KIS_DUMP_DEVICE_2(p.image->projection(), refRect, "00_initial", "dd");

    p.image->setOverlaySelectionMask(mask);
    p.image->waitForDone();

    KIS_DUMP_DEVICE_2(p.image->projection(), refRect, "01_activated_00_image", "dd");
    KIS_DUMP_DEVICE_2(p.image->root()->original(), refRect, "01_activated_01_root_original", "dd");
    KIS_DUMP_DEVICE_2(p.image->root()->projection(), refRect, "01_activated_02_root_projection", "dd");

    KisImageSP clonedImage = p.image->clone();
    clonedImage->waitForDone();
    KIS_DUMP_DEVICE_2(clonedImage->projection(), refRect, "02_cloned_when_activated_00_image", "dd");
    KIS_DUMP_DEVICE_2(clonedImage->root()->original(), refRect, "02_cloned_when_activated_01_root_original", "dd");
    KIS_DUMP_DEVICE_2(clonedImage->root()->projection(), refRect, "02_cloned_when_activated_02_root_projection", "dd");

    p.image->setOverlaySelectionMask(0);
    p.image->waitForDone();

    KIS_DUMP_DEVICE_2(p.image->projection(), refRect, "03_deactivated", "dd");
}

void KisImageTest::testHdrReferenceWhite_data()
{
    #if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QTest::addColumn<const KoColorSpace*>("srcSpace");
    QTest::addColumn<const KoColorSpace*>("dstSpace");
    QTest::addColumn<double>("expectedOriginalHdrReferenceWhite");
    QTest::addColumn<double>("srcHdrReferenceWhite");
    QTest::addColumn<double>("expectedDstHdrReferenceWhite");

    const double noHdrReferenceWhite = -1;

    auto *sRGBu8 = KoColorSpaceRegistry::instance()->colorSpace(
        RGBAColorModelID.id(),
        Integer8BitsColorDepthID.id(),
        KoColorSpaceRegistry::instance()->p709SRGBProfile());

    auto *rec2020g10u16 = KoColorSpaceRegistry::instance()->colorSpace(
        RGBAColorModelID.id(),
        Integer16BitsColorDepthID.id(),
        KoColorSpaceRegistry::instance()->p2020G10Profile());

    auto *rec2020g10f16 = KoColorSpaceRegistry::instance()->colorSpace(
        RGBAColorModelID.id(),
        Float16BitsColorDepthID.id(),
        KoColorSpaceRegistry::instance()->p2020G10Profile());

    auto *rec2020pqu16_203 = KoColorSpaceRegistry::instance()->colorSpace(
        RGBAColorModelID.id(),
        Integer16BitsColorDepthID.id(),
        KoColorSpaceRegistry::instance()->profileByName("Krita Rec. 2100 Perceptual Quantizer (203cd/m²)"));

    auto *rec2020pqu16_80 = KoColorSpaceRegistry::instance()->colorSpace(
        RGBAColorModelID.id(),
        Integer16BitsColorDepthID.id(),
        KoColorSpaceRegistry::instance()->profileByName("Krita Rec. 2100 Perceptual Quantizer (80cd/m²)"));

    // from SDR to display-referred HDR
    QTest::addRow("srgb-to-rec2020pq_203")
        << sRGBu8 << rec2020pqu16_203
        << noHdrReferenceWhite << noHdrReferenceWhite << 203.0;

    // from scene-referred HDR to display-referred HDR
    QTest::addRow("rec2020g10f16-to-rec2020pq_203")
        << rec2020g10f16 << rec2020pqu16_203
        << noHdrReferenceWhite << noHdrReferenceWhite << 203.0;

    // from scene-referred HDR with custom HDR Reference White
    // to display-referred HDR
        QTest::addRow("rec2020g10f16_116-to-rec2020pq_203")
        << rec2020g10f16 << rec2020pqu16_203
        << noHdrReferenceWhite << 116.0 << 203.0;

    // from display-referred HDR (80 nits) to another
    // display-referred HDR (203 nits)
    QTest::addRow("rec2020pq_80-to-rec2020pq_203")
        << rec2020pqu16_80 << rec2020pqu16_203
        << 80.0 << noHdrReferenceWhite << 203.0;

    // from display referred HDR to untagged scene-referred
    // (the metadata should persist)
    QTest::addRow("rec2020pq_203-to-rec2020g10f16")
        << rec2020pqu16_203 << rec2020g10f16
        << 203.0 << noHdrReferenceWhite << 203.0;

    // from display referred HDR to clipped integer scene-referred
    // (the metadata should be dropped)
    QTest::addRow("rec2020pq_203-to-rec2020g10u")
        << rec2020pqu16_203 << rec2020g10u16
        << 203.0 << noHdrReferenceWhite << noHdrReferenceWhite;

    // from display referred HDR to clipped integer sRGB
    // (the metadata should be dropped)
    QTest::addRow("rec2020pq_203-to-srgb")
        << rec2020pqu16_203 << sRGBu8
        << 203.0 << noHdrReferenceWhite << noHdrReferenceWhite;
    #endif
}

void KisImageTest::testHdrReferenceWhite()
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    auto optionalFromDouble = [] (double value) {
        return value >= 0 ? std::make_optional(value) : std::optional<double>();
    };

    QFETCH(const KoColorSpace*, srcSpace);
    QFETCH(const KoColorSpace*, dstSpace);

    QFETCH(double, expectedOriginalHdrReferenceWhite);
    QFETCH(double, srcHdrReferenceWhite);
    QFETCH(double, expectedDstHdrReferenceWhite);

    const QRect imageRect(0, 0, 32,32);

    KisUndoStore *undoStore = new KisSurrogateUndoStore();
    KisImageSP image = new KisImage(undoStore, imageRect.width(), imageRect.height(), srcSpace, "test image");

    QCOMPARE(image->hdrReferenceWhiteLightLevel(), optionalFromDouble(expectedOriginalHdrReferenceWhite));

    // assign the new reference white only when requested
    if (srcHdrReferenceWhite >= 0) {
        image->setHdrReferenceWhiteLightLevel(optionalFromDouble(srcHdrReferenceWhite));
        QCOMPARE(image->hdrReferenceWhiteLightLevel(), optionalFromDouble(srcHdrReferenceWhite));
    }

    image->convertImageColorSpace(dstSpace, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
    image->waitForDone();

    QCOMPARE(image->hdrReferenceWhiteLightLevel(), optionalFromDouble(expectedDstHdrReferenceWhite));
#endif
}

void KisImageTest::testHdrReferenceWhiteCloning()
{
    const QRect imageRect(0, 0, 32,32);

    auto *rec2020g10f16 = KoColorSpaceRegistry::instance()->colorSpace(
        RGBAColorModelID.id(),
        Float16BitsColorDepthID.id(),
        KoColorSpaceRegistry::instance()->p2020G10Profile());

    KisUndoStore *undoStore = new KisSurrogateUndoStore();
    KisImageSP image = new KisImage(undoStore, imageRect.width(), imageRect.height(), rec2020g10f16, "test image");

    image->setHdrReferenceWhiteLightLevel(116);

    KisImageSP clonedExactCopyImage = image->clone(true);
    QCOMPARE(clonedExactCopyImage->hdrReferenceWhiteLightLevel(), 116);

    KisImageSP clonedInexactCopyImage = image->clone(false);
    QCOMPARE(clonedInexactCopyImage->hdrReferenceWhiteLightLevel(), 116);

}

KISTEST_MAIN(KisImageTest)
