/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisValueCacheTest.h"

#include "simpletest.h"

#include "KisValueCache.h"

void KisValueCacheTest::test()
{
    struct Initializer
    {
        Initializer(int *sourceValue) : m_sourceValue(sourceValue) {}
        int initialize() { return *m_sourceValue; }

    private:
        int *m_sourceValue {nullptr};
    };

    int sourceValue = 1;

    KisValueCache<Initializer> cache(&sourceValue);

    QVERIFY(!cache.isValid());

    QCOMPARE(cache.value(), 1);
    QVERIFY(cache.isValid());

    sourceValue = 2;

    QCOMPARE(cache.value(), 1);

    cache.clear();
    QVERIFY(!cache.isValid());

    QCOMPARE(cache.value(), 2);
}

SIMPLE_TEST_MAIN(KisValueCacheTest);
