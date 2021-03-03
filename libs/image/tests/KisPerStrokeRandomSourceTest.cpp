/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisPerStrokeRandomSourceTest.h"

#include "brushengine/KisPerStrokeRandomSource.h"

#include <simpletest.h>

void KisPerStrokeRandomSourceTest::testIndependent()
{
    bool sourcesDiffer = false;

    /**
     * Theoretically, the we can get two equal 1000-pcs sequences, but it is highly improbable
     */
    for (int i = 0; i < 1000; i++) {
        KisPerStrokeRandomSource s1;
        KisPerStrokeRandomSource s2;

        if (s1.generate("mykey", 0, 1000) != s2.generate("mykey", 0, 1000)) {
            sourcesDiffer = true;
            break;
        }
    }

    QVERIFY(sourcesDiffer);
}

void KisPerStrokeRandomSourceTest::testDependent()
{
    bool allSame = true;

    for (int i = 0; i < 1000; i++) {
        KisPerStrokeRandomSource s1;
        KisPerStrokeRandomSource s2(s1);

        if (s1.generate("mykey", 0, 1000) != s2.generate("mykey", 0, 1000)) {
            allSame = false;
            break;
        }
    }

    QVERIFY(allSame);
}

void KisPerStrokeRandomSourceTest::testDifferentKeys()
{
    bool sourcesDiffer = false;

    /**
     * Theoretically, the we can get two equal 1000-pcs sequences, but it is highly improbable
     */
    for (int i = 0; i < 1000; i++) {
        KisPerStrokeRandomSource s1;
        KisPerStrokeRandomSource s2(s1);

        if (s1.generate("mykey1", 0, 1000) != s2.generate("mykey2", 0, 1000)) {
            sourcesDiffer = true;
            break;
        }
    }

    QVERIFY(sourcesDiffer);
}

SIMPLE_TEST_MAIN(KisPerStrokeRandomSourceTest)
