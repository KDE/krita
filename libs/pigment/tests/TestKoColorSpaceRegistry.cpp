/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "TestKoColorSpaceRegistry.h"

#include <QTest>

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
