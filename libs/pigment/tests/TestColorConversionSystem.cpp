/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#include "TestColorConversionSystem.h"

#include <qtest_kde.h>

#include <DebugPigment.h>
#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorConversionSystem.h>
#include <KoColorModelStandardIds.h>

TestColorConversionSystem::TestColorConversionSystem()
{
    foreach(KoID modelId, KoColorSpaceRegistry::instance()->colorModelsList(KoColorSpaceRegistry::AllColorSpaces)) {
        foreach(KoID depthId, KoColorSpaceRegistry::instance()->colorDepthList(modelId, KoColorSpaceRegistry::AllColorSpaces)) {
            QList< const KoColorProfile * > profiles =
                KoColorSpaceRegistry::instance()->profilesFor(
                    KoColorSpaceRegistry::instance()->colorSpaceId(modelId, depthId));
            foreach(const KoColorProfile * profile, profiles) {
                listModels.append(ModelDepthProfile(modelId.id(), depthId.id(), profile->name()));
            }
        }
    }
    listModels.append(ModelDepthProfile(AlphaColorModelID.id(), Integer8BitsColorDepthID.id(), "Dummy profile"));
}

void TestColorConversionSystem::testConnections()
{
    foreach(ModelDepthProfile srcCS, listModels) {
        foreach(ModelDepthProfile dstCS, listModels) {
            QVERIFY2(KoColorSpaceRegistry::instance()->colorConversionSystem()->existsPath(srcCS.model, srcCS.depth, srcCS.profile, dstCS.model, dstCS.depth, dstCS.profile) , QString("No path between %1 / %2 and %3 / %4").arg(srcCS.model).arg(srcCS.depth).arg(dstCS.model).arg(dstCS.depth).toLatin1());
        }
    }
}

void TestColorConversionSystem::testGoodConnections()
{
    int countFail = 0;
    foreach(ModelDepthProfile srcCS, listModels) {
        foreach(ModelDepthProfile dstCS, listModels) {
            if (!KoColorSpaceRegistry::instance()->colorConversionSystem()->existsGoodPath(srcCS.model, srcCS.depth, srcCS.profile , dstCS.model, dstCS.depth, dstCS.profile)) {
                ++countFail;
                dbgPigment << "No good path between \"" << srcCS.model << " " << srcCS.depth << " " << srcCS.profile << "\" \"" << dstCS.model << " " << dstCS.depth << " " << dstCS.profile << "\"";
            }
        }
    }
    int failed = 0;
    if (!KoColorSpaceRegistry::instance()->colorSpace( RGBAColorModelID.id(), Float32BitsColorDepthID.id(), 0) && KoColorSpaceRegistry::instance()->colorSpace( "KS6", Float32BitsColorDepthID.id(), 0) ) {
        failed = 42;
    }
    QVERIFY2(countFail == failed, QString("%1 tests have fails (it should have been %2)").arg(countFail).arg(failed).toLatin1());
}

QTEST_KDEMAIN(TestColorConversionSystem, NoGUI)
#include <TestColorConversionSystem.moc>
