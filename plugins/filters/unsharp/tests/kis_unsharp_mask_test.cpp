/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_unsharp_mask_test.h"

#include <simpletest.h>

#include "kis_transaction.h"
#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include <KisGlobalResourcesInterface.h>
#include "testutil.h"
#include "testing_timed_default_bounds.h"

void KisUnsharpMaskTest::testUnsharpWithTransparency()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    QImage srcImage(TestUtil::fetchDataFileLazy("source_with_transparency.png"));
    QVERIFY(!srcImage.isNull());

    QRect imageRect = srcImage.rect();

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(srcImage, 0, 0, 0);
    dev->setDefaultBounds(new TestUtil::TestingTimedDefaultBounds(srcImage.rect()));

    KisFilterSP f = KisFilterRegistry::instance()->value("unsharp");
    Q_ASSERT(f);

    KisFilterConfigurationSP  kfc = f->defaultConfiguration(KisGlobalResourcesInterface::instance());
    Q_ASSERT(kfc);

    kfc->setProperty("halfSize", 3);
    kfc->setProperty("amount", 2.5);
    kfc->setProperty("threshold", 0);

    KisTransaction t(dev);
    f->process(dev, QRect(imageRect.topLeft(), imageRect.size()), kfc->cloneWithResourcesSnapshot());
    t.end();

    QImage resultImage =
        dev->convertToQImage(0,
                             imageRect.x(), imageRect.y(),
                             imageRect.width(), imageRect.height());

    TestUtil::checkQImagePremultiplied(resultImage,
                                       "unsharp_mask_test",
                                       "with_transparency",
                                       "unsharp_with_transparency", 1, 7);
}

SIMPLE_TEST_MAIN(KisUnsharpMaskTest)
