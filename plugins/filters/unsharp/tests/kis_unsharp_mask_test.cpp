/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_unsharp_mask_test.h"

#include <QTest>

#include "kis_transaction.h"
#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "testutil.h"


void KisUnsharpMaskTest::testUnsharpWithTransparency()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    QImage srcImage(TestUtil::fetchDataFileLazy("source_with_transparency.png"));
    QVERIFY(!srcImage.isNull());

    QRect imageRect = srcImage.rect();

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(srcImage, 0, 0, 0);

    KisFilterSP f = KisFilterRegistry::instance()->value("unsharp");
    Q_ASSERT(f);

    KisFilterConfiguration * kfc = f->defaultConfiguration(0);
    Q_ASSERT(kfc);

    kfc->setProperty("halfSize", 3);
    kfc->setProperty("amount", 2.5);
    kfc->setProperty("threshold", 0);

    KisTransaction t(dev);
    f->process(dev, QRect(imageRect.topLeft(), imageRect.size()), kfc);
    t.end();

    QImage resultImage =
        dev->convertToQImage(0,
                             imageRect.x(), imageRect.y(),
                             imageRect.width(), imageRect.height());

    TestUtil::checkQImage(resultImage,
                          "unsharp_mask_test",
                          "with_transparency",
                          "unsharp_with_transparency");
}

QTEST_MAIN(KisUnsharpMaskTest)
