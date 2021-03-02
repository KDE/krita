/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_dummies_facade_test.h"

#include <simpletest.h>
#include "kistest.h"

#include "kis_dummies_facade.h"


KisDummiesFacadeTest::~KisDummiesFacadeTest()
{
}

KisDummiesFacadeBase* KisDummiesFacadeTest::dummiesFacadeFactory()
{
    return new KisDummiesFacade();
}

void KisDummiesFacadeTest::destroyDummiesFacade(KisDummiesFacadeBase *dummiesFacade)
{
    delete dummiesFacade;
}

KISTEST_MAIN(KisDummiesFacadeTest)
