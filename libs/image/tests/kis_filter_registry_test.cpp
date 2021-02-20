/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "filter/kis_filter.h"
#include "filter/kis_filter_registry.h"
#include "kis_filter_registry_test.h"

#include <QTest>

void KisFilterRegistryTest::testCreation()
{
    KisFilterRegistry * test = KisFilterRegistry::instance();
    Q_UNUSED(test);
}


QTEST_MAIN(KisFilterRegistryTest)
