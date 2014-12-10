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

#include "kis_layer_styles_test.h"

#include <qtest_kde.h>

#include "kis_transaction.h"
#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "testutil.h"


void KisLayerStylesTest::testLayerStylesFull()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    QRect srcRect(50, 50, 100, 100);
    QRect dstRect(0, 0, 200, 200);

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->fill(srcRect, KoColor(Qt::red, cs));

    KisFilterSP f = KisFilterRegistry::instance()->value("lsdropshadow");
    Q_ASSERT(f);

    KisFilterConfiguration * kfc = f->defaultConfiguration(0);
    Q_ASSERT(kfc);

    kfc->setProperty("drop_shadow/angle", 115);
    kfc->setProperty("context/global_angle", 125);
    kfc->setProperty("drop_shadow/spread", 50);
    kfc->setProperty("drop_shadow/size", 10);
    kfc->setProperty("drop_shadow/noise", 30);
    kfc->setProperty("drop_shadow/knocks_out", false);
    kfc->setProperty("drop_shadow/opacity", 50);

    KisTransaction t(dev);
    f->process(dev, srcRect, kfc);
    t.end();

    QImage resultImage =
        dev->convertToQImage(0, dstRect);

    TestUtil::checkQImage(resultImage,
                          "layer_styles_test",
                          "common",
                          "full_update");
}

QTEST_KDEMAIN(KisLayerStylesTest, GUI)
