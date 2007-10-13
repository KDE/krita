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

#include <kdebug.h>

#include <KoColorSpaceRegistry.h>
#include <KoColorConversionSystem.h>
#include <KoColorModelStandardIds.h>

TestColorConversionSystem::TestColorConversionSystem()
{
    foreach( KoID modelId, KoColorSpaceRegistry::instance()->colorModelsList())
    {
        foreach( KoID depthId, KoColorSpaceRegistry::instance()->colorDepthList(modelId))
        {
            listModels.append( pStrStr( modelId.id(), depthId.id() ) );
        }
    }
    listModels.append( pStrStr(AlphaColorModelID.id(), Integer8BitsColorDepthID.id() ) );
}

void TestColorConversionSystem::testConnections()
{
    foreach( pStrStr srcCS, listModels)
    {
        foreach( pStrStr dstCS, listModels)
        {
            QVERIFY2( KoColorSpaceRegistry::instance()->colorConversionSystem()->existsPath(srcCS.first, srcCS.second , dstCS.first, dstCS.second) , QString("No path between %1 / %2 and %3 / %4").arg(srcCS.first).arg(srcCS.second).arg(dstCS.first).arg(dstCS.second).latin1() );
        }
    }
}

void TestColorConversionSystem::testGoodConnections()
{
    foreach( pStrStr srcCS, listModels)
    {
        foreach( pStrStr dstCS, listModels)
        {
            QVERIFY2( KoColorSpaceRegistry::instance()->colorConversionSystem()->existsGoodPath(srcCS.first, srcCS.second , dstCS.first, dstCS.second) , QString("No good path between %1 / %2 and %3 / %4").arg(srcCS.first).arg(srcCS.second).arg(dstCS.first).arg(dstCS.second).latin1() );
        }
    }
}

QTEST_KDEMAIN(TestColorConversionSystem, NoGUI)
#include "TestColorConversionSystem.moc"
