/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_clipboard_test.h"

#include <testui.h>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <KisDocument.h>
#include <KisPart.h>

#include "kis_clipboard.h"
#include <kis_clone_layer.h>
#include "kis_default_bounds.h"
#include "kis_paint_device.h"
#include <kis_pixel_selection.h>
#include <kis_transparency_mask.h>

#include <QClipboard>

#include <testutil.h>

namespace {
bool checkHole(KisPaintDeviceSP dev,
               const QRect &totalRect,
               const KoColor &bgColor,
               const QRect &holeRect,
               const KoColor &holeColor)
{
    KisSequentialIterator it(dev, totalRect);
    while (it.nextPixel()) {
        const KoColor &expectedColor = holeRect.contains(it.x(), it.y()) ? holeColor : bgColor;

        KoColor pixel(it.rawDataConst(), dev->colorSpace());

        if (pixel != expectedColor) {
            qDebug() << "Failed to compare pixel at" << it.x() << it.y();
            qDebug() << "    " << ppVar(pixel);
            qDebug() << "    " << ppVar(expectedColor);
            qDebug() << "    " << ppVar(totalRect);
            qDebug() << "    " << ppVar(holeRect);
            return false;
        }
    }

    return true;
}

bool checkHole(const QImage &image,
               const QRect &totalRect,
               const KoColor &bgColor,
               const QRect &holeRect,
               const KoColor &holeColor)
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, nullptr);
    return checkHole(dev, totalRect, bgColor, holeRect, holeColor);
}

struct TestImageWrapper
{
    TestImageWrapper(const QSize &size, const QRect &fillRect, const QString &defaultPixelName)
    {
        const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
        image = new KisImage(nullptr, size.width(), size.height(), cs, "src test image");

        paintLayer = new KisPaintLayer(image, "paintlayer", OPACITY_OPAQUE_U8);
        image->addNode(paintLayer, image->root());

        KisPaintDeviceSP dev = paintLayer->paintDevice();
        QCOMPARE(dev->defaultBounds()->bounds(), image->bounds());

        defaultPixel = getPixel(defaultPixelName);
        dev->setDefaultPixel(defaultPixel);

        fillPixel = getFillPixel();
        dev->fill(fillRect, fillPixel);
    }

    static KoColor getPixel(const QString &pixelName) {
        const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();

        KoColor pixel;

        if (pixelName == "transparent") {
            pixel = KoColor(Qt::transparent, cs);
        } else if (pixelName == "blue") {
            pixel = KoColor(Qt::blue, cs);
        } else {
            qFatal("Unknown default pixel name: %s", pixelName.toLatin1().data());
        }

        return pixel;
    }

    static KoColor getFillPixel() {
        const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
        return KoColor(Qt::red, cs);
    }

    KoColor defaultPixel;
    KoColor fillPixel;
    KisImageSP image;
    KisPaintLayerSP paintLayer;
};
}


void KisClipboardTest::testLoadAdaptedFromLayers_data()
{
    QTest::addColumn<QString>("testMode");
    QTest::addColumn<QString>("defaultPixelName");
    QTest::addColumn<QSize>("srcImageSize");
    QTest::addColumn<QSize>("dstImageSize");
    QTest::addColumn<QRect>("srcFillRect");
    QTest::addColumn<bool>("expectedSameDstDefaultPixel");
    QTest::addColumn<QRect>("expectedDstBgFillRect");

    QTest::addRow("layer-transparent-bg") << "layer" << "transparent" << QSize(100, 100) << QSize(100, 100) <<  QRect(10,10,20,20) << true << QRect(0,0,100,100);
    QTest::addRow("layer-opaque-bg") << "layer" << "blue" << QSize(100, 100) << QSize(100, 100) <<  QRect(10,10,20,20) << true << QRect(0,0,100,100);

    QTest::addRow("multilayer-transparent-bg") << "multilayer" << "transparent" << QSize(100, 100) << QSize(100, 100) << QRect(10,10,20,20) << true << QRect(0,0,100,100);
    QTest::addRow("multilayer-opaque-bg") << "multilayer" << "blue" << QSize(100, 100) << QSize(100, 100) << QRect(10,10,20,20) << false << QRect(0,0,100,100);

    // with only one layer, the default pixel grows naturally
    QTest::addRow("layer-transparent-bg-grow") << "layer" << "transparent" << QSize(100, 100) << QSize(200, 200) <<  QRect(10,10,20,20) << true << QRect(0,0,200,200);
    QTest::addRow("layer-opaque-bg-grow") << "layer" << "blue" << QSize(100, 100) << QSize(200, 200) <<  QRect(10,10,20,20) << true << QRect(0,0,200,200);

    // with multilayer configuration, both layers grow iif we pass the proper
    // default bounds object to them
    QTest::addRow("multilayer-transparent-bg-grow") << "multilayer" << "transparent" << QSize(100, 100) << QSize(200, 200) << QRect(10,10,20,20) << true << QRect(0,0,200,200);
    QTest::addRow("multilayer-opaque-bg-grow") << "multilayer" << "blue" << QSize(100, 100) << QSize(200, 200) << QRect(10,10,20,20) << false << QRect(0,0,200,200);
}

void KisClipboardTest::testLoadAdaptedFromLayers()
{
    QFETCH(QString, testMode);
    QFETCH(QString, defaultPixelName);
    QFETCH(QSize, srcImageSize);
    QFETCH(QSize, dstImageSize);
    QFETCH(QRect, srcFillRect);
    QFETCH(bool, expectedSameDstDefaultPixel);
    QFETCH(QRect, expectedDstBgFillRect);

    TestImageWrapper w(srcImageSize, srcFillRect, defaultPixelName);
    KisImageSP srcImage = w.image;
    KisPaintLayerSP paintLayer = w.paintLayer;
    KisPaintDeviceSP dev = paintLayer->paintDevice();

    const QRect expectedSrcExactBounds = w.defaultPixel.opacityU8() > 0 ? srcImage->bounds() : srcFillRect;

    QCOMPARE(dev->exactBounds(), expectedSrcExactBounds);
    QVERIFY(checkHole(dev, srcImage->bounds(), w.defaultPixel, srcFillRect, w.fillPixel));

    if (testMode == "layer") {
        KisClipboard::instance()->setLayers({paintLayer}, srcImage, QRect(), true);
    } else if (testMode == "multilayer") {
        KisPaintLayerSP paintLayer2 = new KisPaintLayer(srcImage, "test layer 2", OPACITY_OPAQUE_U8);
        srcImage->addNode(paintLayer2, srcImage->root());
        KisClipboard::instance()->setLayers({paintLayer, paintLayer2}, srcImage, QRect(), true);
    } else {
        qFatal("Unknown mode: %s", testMode.toLatin1().data());
    }

    KisImageSP dstImage = new KisImage(nullptr, dstImageSize.width(), dstImageSize.height(), w.image->colorSpace(), "dst test image");
    KisDefaultBoundsSP dstDefaultBounds = new KisDefaultBounds(dstImage);

    KisPaintDeviceSP newDev = KisClipboard::instance()->clipFromKritaLayers(nullptr, dstDefaultBounds);

    if (expectedSameDstDefaultPixel) {
        QCOMPARE(newDev->defaultPixel(), w.defaultPixel);
    } else {
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
        QCOMPARE_NE(newDev->defaultPixel(), w.defaultPixel);
#endif
    }

    const QRect expectedDstExactBounds = w.defaultPixel.opacityU8() > 0 ? expectedDstBgFillRect : srcFillRect;
    QCOMPARE(newDev->exactBounds(), expectedDstExactBounds);
    QVERIFY(checkHole(newDev, expectedDstBgFillRect, w.defaultPixel, srcFillRect, w.fillPixel));
}

void KisClipboardTest::testLoadFromLayersStandalone_data()
{
    QTest::addColumn<QString>("testMode");
    QTest::addColumn<QString>("defaultPixelName");
    QTest::addColumn<QRect>("referenceRect");
    QTest::addColumn<QSize>("expectedDstImageSize");
    QTest::addColumn<QRect>("expectedDstExactBounds");
    QTest::addColumn<QRect>("expectedDstFillRect");

    QTest::addRow("internal-transparent-ref_auto") << "internal" << "transparent" << QRect() << QSize(40,40) << QRect(0,0,40,40) << QRect(0,0,40,40);
    QTest::addRow("internal-transparent-ref_image") << "internal" << "transparent" << QRect(0,0,100,100) << QSize(100,100) << QRect(20,20,40,40) << QRect(20,20,40,40);
    QTest::addRow("internal-transparent-ref_custom") << "internal" << "transparent" << QRect(10,10,200,200) << QSize(200,200) << QRect(10,10,40,40) << QRect(10,10,40,40);

    QTest::addRow("internal-blue-ref_auto") << "internal" << "blue" << QRect() << QSize(100,100) << QRect(0,0,100,100) << QRect(20,20,40,40);
    QTest::addRow("internal-blue-ref_image") << "internal" << "blue" << QRect(0,0,100,100) << QSize(100,100) << QRect(0,0,100,100) << QRect(20,20,40,40);
    QTest::addRow("internal-blue-ref_custom") << "internal" << "blue" << QRect(10,10,200,200) << QSize(200,200) << QRect(0,0,200,200) << QRect(10,10,40,40);

    QTest::addRow("qimage-transparent-ref_auto") << "qimage" << "transparent" << QRect() << QSize(40,40) << QRect(0,0,40,40) << QRect(0,0,40,40);
    QTest::addRow("qimage-transparent-ref_image") << "qimage" << "transparent" << QRect(0,0,100,100) << QSize(100,100) << QRect(20,20,40,40) << QRect(20,20,40,40);
    QTest::addRow("qimage-transparent-ref_custom") << "qimage" << "transparent" << QRect(10,10,200,200) << QSize(200,200) << QRect(10,10,40,40) << QRect(10,10,40,40);

    QTest::addRow("qimage-blue-ref_auto") << "qimage" << "blue" << QRect() << QSize(100,100) << QRect(0,0,100,100) << QRect(20,20,40,40);
    QTest::addRow("qimage-blue-ref_image") << "qimage" << "blue" << QRect(0,0,100,100) << QSize(100,100) << QRect(0,0,100,100) << QRect(20,20,40,40);
    QTest::addRow("qimage-blue-ref_custom") << "qimage" << "blue" << QRect(10,10,200,200) << QSize(200,200) << QRect(0,0,200,200) << QRect(10,10,40,40);
}

void KisClipboardTest::testLoadFromLayersStandalone()
{
    QFETCH(QString, testMode);
    QFETCH(QString, defaultPixelName);
    QFETCH(QRect, referenceRect);
    QFETCH(QSize, expectedDstImageSize);
    QFETCH(QRect, expectedDstExactBounds);
    QFETCH(QRect, expectedDstFillRect);

    const QSize imageSize(100,100);
    const QRect fillRect(20,20,40,40);

    TestImageWrapper w(imageSize, fillRect, defaultPixelName);
    w.image->setResolution(300.0 / 72.0, 300 / 72.0);

    KisClipboard::instance()->setLayers({w.paintLayer}, w.image, referenceRect);

    if (testMode == "internal") {
        KisImageSP loadedImage = KisClipboard::instance()->clipImageFromKritaLayers(nullptr);
        QVERIFY(loadedImage);

        QCOMPARE(qRound(loadedImage->xRes() * 72.0), 300); // 300 dpi
        QCOMPARE(qRound(loadedImage->yRes() * 72.0), 300); // 300 dpi

        QCOMPARE(loadedImage->size(), expectedDstImageSize);

        KisNodeSP paintLayer = loadedImage->root()->firstChild();
        QVERIFY(paintLayer);

        KisPaintDeviceSP device = paintLayer->paintDevice();

        QCOMPARE(device->defaultPixel(), w.paintLayer->paintDevice()->defaultPixel());
        QCOMPARE(device->defaultBounds()->bounds(), loadedImage->bounds());
        QCOMPARE(device->exactBounds(), expectedDstExactBounds);
        QVERIFY(checkHole(device, QRect(QPoint(), expectedDstImageSize), w.defaultPixel, expectedDstFillRect, w.fillPixel));
    } else if (testMode == "qimage") {
        QImage image = qApp->clipboard()->image();
        QVERIFY(!image.isNull());

        QCOMPARE(image.size(), expectedDstImageSize);
        QCOMPARE(image.dotsPerMeterX(), qRound(300 / 2.54 * 100));
        QCOMPARE(image.dotsPerMeterY(), qRound(300 / 2.54 * 100));
        QVERIFY(checkHole(image, QRect(QPoint(), expectedDstImageSize), w.defaultPixel, expectedDstFillRect, w.fillPixel));
    }
}

void KisClipboardTest::testLoadFromPngUrl()
{
    const QString outputFilePath = QString(FILES_OUTPUT_DIR) + '/' + "test-load-from-png-url.png";

    QString defaultPixelName = "transparent";

    const QSize imageSize(100,100);
    const QRect fillRect(20,20,40,40);

    TestImageWrapper w(imageSize, fillRect, defaultPixelName);

    QImage image = w.paintLayer->paintDevice()->convertToQImage(0, QRect(0,0,100,100));
    image.setDotsPerMeterX(qRound(300 / 2.54 * 100)); // 300 dpi
    image.setDotsPerMeterY(qRound(300 / 2.54 * 100)); // 300 dpi

    image.save(outputFilePath, "PNG");

    KisImageSP loadedImage = KisClipboard::instance()->fetchImageByURL(QUrl::fromLocalFile(outputFilePath));
    QVERIFY(loadedImage);

    QCOMPARE(qRound(loadedImage->xRes() * 72.0), 300); // 300 dpi
    QCOMPARE(qRound(loadedImage->yRes() * 72.0), 300); // 300 dpi
    QCOMPARE(loadedImage->size(), imageSize);

    KisNodeSP paintLayer = loadedImage->root()->firstChild();
    QVERIFY(paintLayer);

    KisPaintDeviceSP device = paintLayer->paintDevice();

    QCOMPARE(device->defaultBounds()->bounds(), loadedImage->bounds());
    QCOMPARE(device->exactBounds(), fillRect);
    QVERIFY(checkHole(device, QRect(QPoint(), imageSize), w.defaultPixel, fillRect, w.fillPixel));
}

void KisClipboardTest::testLoadFromKraUrl_data()
{
    QTest::addColumn<QString>("testMode");
    QTest::addColumn<QString>("defaultPixelName");
    QTest::addColumn<bool>("expectedSameDstDefaultPixel");
    QTest::addColumn<QRect>("expectedDstExactBounds");

    QTest::addRow("transparent") << "single-layer" << "transparent" << true << QRect(20,20,40,40);
    QTest::addRow("blue") << "single-layer" << "blue" << true << QRect(0,0,100,100);

    QTest::addRow("transparent-multi") << "multi-layer" << "transparent" << true << QRect(20,20,40,40);
    QTest::addRow("blue-multi") << "multi-layer" << "blue" << false << QRect(0,0,100,100);
}

void KisClipboardTest::testLoadFromKraUrl()
{
    QFETCH(QString, testMode);
    QFETCH(QString, defaultPixelName);
    QFETCH(bool, expectedSameDstDefaultPixel);
    QFETCH(QRect, expectedDstExactBounds);

    const QString outputFilePath = QString("%1/test-load-from-kra-url-%2-%3.png").arg(FILES_OUTPUT_DIR).arg(defaultPixelName).arg(testMode);

    const QSize imageSize(100,100);
    const QRect fillRect(20,20,40,40);

    TestImageWrapper w(imageSize, fillRect, defaultPixelName);
    w.image->setResolution(300.0 / 72.0, 300 / 72.0);

    if (testMode == "single-layer") {
        // noop
    } else if (testMode == "multi-layer") {
        KisPaintLayerSP secondLayer = new KisPaintLayer(w.image, "secondlayer", OPACITY_OPAQUE_U8);
        w.image->addNode(secondLayer);
    } else {
        qFatal("Unknown testing mode: %s", testMode.toLatin1().data());
    }

    {
        std::unique_ptr<KisDocument> doc(KisPart::instance()->createTemporaryDocument());
        doc->setCurrentImage(w.image);
        doc->exportDocumentSync(outputFilePath, KisDocument::nativeFormatMimeType());
        doc->setCurrentImage(nullptr);
    }

    KisImageSP loadedImage = KisClipboard::instance()->fetchImageByURL(QUrl::fromLocalFile(outputFilePath));
    QVERIFY(loadedImage);

    QCOMPARE(qRound(loadedImage->xRes() * 72.0), 300); // 300 dpi
    QCOMPARE(qRound(loadedImage->yRes() * 72.0), 300); // 300 dpi
    QCOMPARE(loadedImage->size(), imageSize);

    KisNodeSP paintLayer = loadedImage->root()->firstChild();
    QVERIFY(paintLayer);

    KisPaintDeviceSP device = paintLayer->paintDevice();

    if (expectedSameDstDefaultPixel) {
        QCOMPARE(device->defaultPixel(), w.paintLayer->paintDevice()->defaultPixel());
    } else {
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
        QCOMPARE_NE(device->defaultPixel(), w.paintLayer->paintDevice()->defaultPixel());
#endif
    }

    QCOMPARE(device->defaultBounds()->bounds(), loadedImage->bounds());
    QCOMPARE(device->exactBounds(), expectedDstExactBounds);
    QVERIFY(checkHole(device, QRect(QPoint(), imageSize), w.defaultPixel, fillRect, w.fillPixel));
}

void KisClipboardTest::testLoadFromPngClipboard()
{
    const QString outputFilePath = QString(FILES_OUTPUT_DIR) + '/' + "test-load-from-png-url.png";

    QString defaultPixelName = "transparent";

    const QSize imageSize(100,100);
    const QRect fillRect(20,20,40,40);

    TestImageWrapper w(imageSize, fillRect, defaultPixelName);

    QImage image = w.paintLayer->paintDevice()->convertToQImage(0, QRect(0,0,100,100));
    image.setDotsPerMeterX(qRound(300 / 2.54 * 100)); // 300 dpi
    image.setDotsPerMeterY(qRound(300 / 2.54 * 100)); // 300 dpi

    qApp->clipboard()->setImage(image);

    image.save(outputFilePath, "PNG");
    qDebug() << ppVar(outputFilePath);

    KisImageSP loadedImage = KisClipboard::instance()->clipImageFromBoardContents(false, KisClipboard::PASTE_ASSUME_WEB, qMakePair(true, KisClipboard::PASTE_FORMAT_CLIP));
    QVERIFY(loadedImage);

    QCOMPARE(qRound(loadedImage->xRes() * 72.0), 300); // 300 dpi
    QCOMPARE(qRound(loadedImage->yRes() * 72.0), 300); // 300 dpi
    QCOMPARE(loadedImage->size(), imageSize);

    KisNodeSP paintLayer = loadedImage->root()->firstChild();
    QVERIFY(paintLayer);

    KisPaintDeviceSP device = paintLayer->paintDevice();

    QCOMPARE(device->defaultBounds()->bounds(), loadedImage->bounds());
    QCOMPARE(device->exactBounds(), fillRect);
    QVERIFY(checkHole(device, QRect(QPoint(), imageSize), w.defaultPixel, fillRect, w.fillPixel));
}

void KisClipboardTest::testLoadFromCloneLayer_data()
{
    QTest::addColumn<QString>("defaultPixelName");
    QTest::addColumn<QSize>("srcImageSize");
    QTest::addColumn<QSize>("dstImageSize");
    QTest::addColumn<QRect>("srcFillRect");

    QTest::addRow("transparent-copy") << "transparent" << QSize(100, 100) << QSize(100, 100) <<  QRect(10,10,20,20);
    QTest::addRow("opaque-copy") << "blue" << QSize(100, 100) << QSize(100, 100) <<  QRect(10,10,20,20);

    // clone layer's reincarnation should preserve the default pixel
    // of the original, so the pasted layer will grow together with
    // the destination image!
    QTest::addRow("transparent-copy-grow") << "transparent" << QSize(100, 100) << QSize(150, 150) <<  QRect(10,10,20,20);
    QTest::addRow("opaque-copy-grow") << "blue" << QSize(100, 100) << QSize(150, 150) <<  QRect(10,10,20,20);

}

void KisClipboardTest::testLoadFromCloneLayer()
{
    QFETCH(QString, defaultPixelName);
    QFETCH(QSize, srcImageSize);
    QFETCH(QSize, dstImageSize);
    QFETCH(QRect, srcFillRect);

    const KoColor defaultPixel = TestImageWrapper::getPixel(defaultPixelName);
    const KoColor fillPixel = TestImageWrapper::getFillPixel();

    const QRect expectedFillCloneLayerRect = srcFillRect.translated(5, 5);
    const QRect expectedSrcCloneExactBounds =
         defaultPixel.opacityU8() > 0 ? QRect(QPoint(), srcImageSize) : expectedFillCloneLayerRect;

    {
        TestImageWrapper w({srcImageSize, srcFillRect, defaultPixelName});
        KisImageSP srcImage = w.image;
        KisPaintLayerSP paintLayer = w.paintLayer;
        KisPaintDeviceSP dev = paintLayer->paintDevice();

        const QRect expectedSrcExactBounds = w.defaultPixel.opacityU8() > 0 ? srcImage->bounds() : srcFillRect;

        QCOMPARE(dev->exactBounds(), expectedSrcExactBounds);
        QVERIFY(checkHole(dev, srcImage->bounds(), w.defaultPixel, srcFillRect, w.fillPixel));

        KisCloneLayerSP cloneLayer = new KisCloneLayer(paintLayer, srcImage, "clonelayer", OPACITY_OPAQUE_U8);
        cloneLayer->setX(5);
        cloneLayer->setY(5);
        srcImage->addNode(cloneLayer);
        srcImage->initialRefreshGraph();

        QCOMPARE(cloneLayer->projection()->exactBounds(), expectedSrcCloneExactBounds);
        QVERIFY(checkHole(cloneLayer->projection(),
                          srcImage->bounds(),
                          w.defaultPixel,
                          expectedFillCloneLayerRect,
                          w.fillPixel));

        KisClipboard::instance()->setLayers({cloneLayer}, srcImage, QRect(), false);
    }

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP dstImage = new KisImage(nullptr, dstImageSize.width(), dstImageSize.height(), cs, "dst test image");
    KisDefaultBoundsSP dstDefaultBounds = new KisDefaultBounds(dstImage);

    KisPaintDeviceSP newDev = KisClipboard::instance()->clipFromKritaLayers(nullptr, dstDefaultBounds);

    // reincarnation preserves the default pixel
    QCOMPARE(newDev->defaultPixel(), defaultPixel);

    const QRect expectedDstCloneExactBounds =
         defaultPixel.opacityU8() > 0 ? QRect(QPoint(), dstImageSize) : expectedFillCloneLayerRect;

    QCOMPARE(newDev->exactBounds(), expectedDstCloneExactBounds);
    QVERIFY(checkHole(newDev, dstImage->bounds(), defaultPixel, expectedFillCloneLayerRect, fillPixel));
}

void KisClipboardTest::testCopyLayerWithMask_data()
{
    QTest::addColumn<QString>("testMode");
    QTest::addRow("single-layer") << "one-layer";
    QTest::addRow("multi-layer") << "two-layers";
    QTest::addRow("single-mask") << "one-mask";
    QTest::addRow("multi-mask") << "two-masks";
}

void KisClipboardTest::testCopyLayerWithMask()
{
    QFETCH(QString, testMode);

    TestImageWrapper w({QSize(100,100), QRect(10,10,20,20), "transparent"});
    KisImageSP srcImage = w.image;
    KisPaintLayerSP paintLayer = w.paintLayer;
    KisPaintDeviceSP dev = paintLayer->paintDevice();

    KisNodeList nodes;

    KisTransparencyMaskSP mask = new KisTransparencyMask(srcImage, "mask");
    mask->setSelection(new KisSelection());
    mask->selection()->pixelSelection()->select(QRect(20,20,20,20), OPACITY_OPAQUE_U8);
    srcImage->addNode(mask, paintLayer);

    if (testMode == "one-layer") {
        nodes << paintLayer;
    } else if (testMode == "two-layers") {
        KisPaintLayerSP paintLayer2 = new KisPaintLayer(srcImage, "paintlayer2", OPACITY_OPAQUE_U8);
        srcImage->addNode(paintLayer2);

        nodes << paintLayer;
        nodes << paintLayer2;
    } else if (testMode == "one-mask") {
        nodes << mask;
    } else if (testMode == "two-masks") {
        // the second mask is fully contained in the first mask, so it
        // does not change the projection
        KisTransparencyMaskSP mask2 = new KisTransparencyMask(srcImage, "mask");
        mask2->setSelection(new KisSelection());
        mask2->selection()->pixelSelection()->select(QRect(20,20,10,10), OPACITY_OPAQUE_U8);
        srcImage->addNode(mask2, paintLayer);

        nodes << mask;
        nodes << mask2;

    } else {
        qFatal("Unknown test mode: %s", testMode.toLatin1().data());
    }

    srcImage->initialRefreshGraph();

    const QRect expectedMaskClippedRect(20,20,10,10);

    QCOMPARE(srcImage->projection()->exactBounds(), expectedMaskClippedRect);

    KisClipboard::instance()->setLayers(nodes, srcImage, srcImage->bounds());

    if (testMode == "one-layer" || testMode == "two-layers") {
        KisImageSP loadedImage = KisClipboard::instance()->clipImageFromKritaLayers(nullptr);
        QVERIFY(loadedImage);
        // when copying layers we copy them together with all the attached masks,
        // so the cropped area will persist!
        QCOMPARE(loadedImage->projection()->exactBounds(), expectedMaskClippedRect);
    } else if (testMode == "one-mask") {
        KisImageSP loadedImage = KisClipboard::instance()->clipImageFromKritaLayers(nullptr);
        QVERIFY(loadedImage);

        loadedImage->convertImageColorSpace(srcImage->colorSpace(), KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
        loadedImage->waitForDone();

        QCOMPARE(loadedImage->projection()->exactBounds(), srcImage->bounds());

        const KoColor blackPixel(Qt::black, srcImage->colorSpace());
        const KoColor whitePixel(Qt::white, srcImage->colorSpace());

        QVERIFY(checkHole(loadedImage->projection(), loadedImage->bounds(), blackPixel, QRect(20,20,20,20), whitePixel));
    } else if (testMode == "two-masks") {
        KisImageSP loadedImage = KisClipboard::instance()->clipImageFromKritaLayers(nullptr);
        QVERIFY(!loadedImage);
    }
}

KISTEST_MAIN(KisClipboardTest)
