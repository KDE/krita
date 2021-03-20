/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_boundary_test.h"

#include <simpletest.h>
#include "kis_boundary.h"
#include "kis_types.h"
#include "kis_fixed_paint_device.h"
#include "KoColorSpace.h"
#include "KoColorSpaceRegistry.h"

void KisBoundaryTest::testCreation()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisFixedPaintDeviceSP dev = new KisFixedPaintDevice(cs);
    KisBoundary test(dev);
}


SIMPLE_TEST_MAIN(KisBoundaryTest)
