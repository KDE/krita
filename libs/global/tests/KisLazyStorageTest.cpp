/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisLazyStorageTest.h"

#include "simpletest.h"

#include "KisLazyStorage.h"

void KisLazyStorageTest::test()
{
    KisLazyStorage<int, int> storage1(42);
    QCOMPARE(*storage1, 42);

    KisLazyStorage<QPair<int, float>, int, float> storage2(13, 42.0f);
    QCOMPARE(*storage2, qMakePair(13, 42.0f));

    KisLazyStorage<float, float> storage3(KisLazyStorage<float, float>::init_value_tag(),
                                          42.0f);
    QCOMPARE(*storage3, 42.0f);

}

SIMPLE_TEST_MAIN(KisLazyStorageTest);
