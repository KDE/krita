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

#include "TestKoColorSpaceSanity.h"

#include <QTest>
#include <KoColorSpaceRegistry.h>
#include <KoChannelInfo.h>

#include "sdk/tests/kistest.h"

void TestKoColorSpaceSanity::testChannelsInfo()
{
    Q_FOREACH (const KoColorSpace* colorSpace, KoColorSpaceRegistry::instance()->allColorSpaces(KoColorSpaceRegistry::AllColorSpaces, KoColorSpaceRegistry::OnlyDefaultProfile))
    {

        QCOMPARE(colorSpace->channelCount(), quint32(colorSpace->channels().size()));
        QList<int> displayPositions;
        quint32 colorChannels = 0;
        quint32 size = 0;
        Q_FOREACH (KoChannelInfo* info, colorSpace->channels())
        {
            if(info->channelType() == KoChannelInfo::COLOR ) {
                ++colorChannels;
            }
            // Check poses
            qint32 pos = info->pos();
            QVERIFY(pos + info->size() <= (qint32)colorSpace->pixelSize());
            Q_FOREACH (KoChannelInfo* info2, colorSpace->channels())
            {
                if( info != info2 )
                {
                    QVERIFY( pos >= (info2->pos() + info2->size()) || pos + info->size() <= info2->pos());
                }
            }

            // Check displayPosition
            quint32 displayPosition = info->displayPosition();
            QVERIFY(displayPosition < colorSpace->channelCount());
            QVERIFY(displayPositions.indexOf(displayPosition) == -1);
            displayPositions.push_back(displayPosition);

            size += info->size();
        }
        QCOMPARE(size, colorSpace->pixelSize());
        QCOMPARE(colorSpace->colorChannelCount(), colorChannels);
    }
}

KISTEST_MAIN(TestKoColorSpaceSanity)
