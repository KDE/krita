/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "TestKoColorSpaceRegistry.h"

#include <simpletest.h>

#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>

TestBaseColorSpaceRegistry::TestBaseColorSpaceRegistry()
{
}

void TestBaseColorSpaceRegistry::testLab16()
{
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->lab16();
    QCOMPARE(cs->colorModelId().id(), LABAColorModelID.id());
    QCOMPARE(cs->colorDepthId().id(), Integer16BitsColorDepthID.id());
    QVERIFY(*cs == *KoColorSpaceRegistry::instance()->colorSpace(LABAColorModelID.id(), Integer16BitsColorDepthID.id(), 0));
}

void TestBaseColorSpaceRegistry::testRgb8()
{
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
    QCOMPARE(cs->colorModelId().id(), RGBAColorModelID.id());
    QCOMPARE(cs->colorDepthId().id(), Integer8BitsColorDepthID.id());
    QVERIFY(*cs == *KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Integer8BitsColorDepthID.id(), 0));
}

void TestBaseColorSpaceRegistry::testRgb16()
{
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb16();
    QCOMPARE(cs->colorModelId().id(), RGBAColorModelID.id());
    QCOMPARE(cs->colorDepthId().id(), Integer16BitsColorDepthID.id());
    QVERIFY(*cs == *KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Integer16BitsColorDepthID.id(), 0));
}

void TestBaseColorSpaceRegistry::testProfileByUniqueId()
{
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb16();
    const KoColorProfile *profile = cs->profile();
    QVERIFY(profile);

    const KoColorProfile *fetchedProfile =
        KoColorSpaceRegistry::instance()->profileByUniqueId(profile->uniqueId());

    QCOMPARE(*fetchedProfile, *profile);
}

QTEST_GUILESS_MAIN(TestBaseColorSpaceRegistry)
