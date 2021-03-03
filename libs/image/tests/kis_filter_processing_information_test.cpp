/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_filter_processing_information_test.h"

#include <simpletest.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include "kis_paint_device.h"
#include "kis_types.h"
#include "kis_selection.h"
#include "kis_processing_information.h"

void KisProcessingInformationTest::testCreation()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    KisProcessingInformation test(dev, QPoint(0, 0), KisSelectionSP());
}

SIMPLE_TEST_MAIN(KisProcessingInformationTest)
