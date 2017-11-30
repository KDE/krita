/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisPerStrokeRandomSourceTest.h"

#include "brushengine/KisPerStrokeRandomSource.h"

#include <QTest>

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

QTEST_MAIN(KisPerStrokeRandomSourceTest)
