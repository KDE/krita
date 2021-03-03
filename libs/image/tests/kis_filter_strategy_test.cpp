/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_filter_strategy_test.h"

#include <simpletest.h>
#include "kis_filter_strategy.h"

void KisFilterStrategyTest::testCreation()
{
    KisFilterStrategyRegistry *reg = KisFilterStrategyRegistry::instance();
    Q_UNUSED(reg);
    KisHermiteFilterStrategy hermite;
    KisBoxFilterStrategy box;
    KisBilinearFilterStrategy triangle;
    KisBellFilterStrategy bell;
    KisBSplineFilterStrategy bspline;
    KisLanczos3FilterStrategy  lanczos3;
    KisMitchellFilterStrategy  mitchell;
}

SIMPLE_TEST_MAIN(KisFilterStrategyTest)
