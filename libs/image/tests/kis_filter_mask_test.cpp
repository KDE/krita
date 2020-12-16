/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_filter_mask_test.h"
#include <QTest>

#include <KoColorSpaceRegistry.h>

#include "kis_selection.h"
#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "kis_filter_mask.h"
#include "filter/kis_filter_registry.h"
#include "kis_group_layer.h"
#include "kis_paint_device.h"
#include "kis_paint_layer.h"
#include "kis_types.h"
#include "kis_image.h"
#include <KisGlobalResourcesInterface.h>


#include <testutil.h>

#define IMAGE_WIDTH 1000
#define IMAGE_HEIGHT 1000

void KisFilterMaskTest::testProjectionNotSelected()
{
    TestUtil::MaskParent p(QRect(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT));
    KisImageSP image = p.image;
    KisPaintLayerSP layer = p.layer;
    KisPaintDeviceSP projection = layer->paintDevice();

    QImage qimage(QString(FILES_DATA_DIR) + '/' + "hakonepa.png");
    QImage inverted(QString(FILES_DATA_DIR) + '/' + "inverted_hakonepa.png");
    projection->convertFromQImage(qimage, 0, 0, 0);

    KisFilterSP f = KisFilterRegistry::instance()->value("invert");
    Q_ASSERT(f);
    KisFilterConfigurationSP  kfc = f->defaultConfiguration(KisGlobalResourcesInterface::instance());
    Q_ASSERT(kfc);

    KisFilterMaskSP mask = new KisFilterMask(image, "mask");
    image->addNode(mask, layer);

    mask->setFilter(kfc->cloneWithResourcesSnapshot());

    // Check basic apply(). Shouldn't do anything, since nothing is selected yet.

    mask->initSelection(layer);
    mask->createNodeProgressProxy();
    mask->select(qimage.rect(), MIN_SELECTED);

    mask->apply(projection, qimage.rect(), qimage.rect(), KisNode::N_FILTHY);

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, qimage, projection->convertToQImage(0, 0, 0, qimage.width(), qimage.height()))) {
        projection->convertToQImage(0, 0, 0, qimage.width(), qimage.height()).save("filtermasktest1.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

void KisFilterMaskTest::testProjectionSelected()
{
    TestUtil::MaskParent p(QRect(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT));
    KisImageSP image = p.image;
    KisPaintLayerSP layer = p.layer;
    KisPaintDeviceSP projection = layer->paintDevice();

    QImage qimage(QString(FILES_DATA_DIR) + '/' + "hakonepa.png");
    QImage inverted(QString(FILES_DATA_DIR) + '/' + "inverted_hakonepa.png");
    projection->convertFromQImage(qimage, 0, 0, 0);

    KisFilterSP f = KisFilterRegistry::instance()->value("invert");
    Q_ASSERT(f);
    KisFilterConfigurationSP  kfc = f->defaultConfiguration(KisGlobalResourcesInterface::instance());
    Q_ASSERT(kfc);

    KisFilterMaskSP mask = new KisFilterMask(image, "mask");
    image->addNode(mask, layer);

    mask->setFilter(kfc->cloneWithResourcesSnapshot());
    mask->createNodeProgressProxy();

    mask->initSelection(layer);
    mask->select(qimage.rect(), MAX_SELECTED);
    mask->apply(projection, qimage.rect(), qimage.rect(), KisNode::N_FILTHY);
    QCOMPARE(mask->exactBounds(), QRect(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT));

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, inverted, projection->convertToQImage(0, 0, 0, qimage.width(), qimage.height()))) {
        projection->convertToQImage(0, 0, 0, qimage.width(), qimage.height()).save("filtermasktest2.png");
        QFAIL(QString("Failed to create inverted image, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }

}

QTEST_MAIN(KisFilterMaskTest)
