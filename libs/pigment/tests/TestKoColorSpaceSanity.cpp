/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "TestKoColorSpaceSanity.h"

#include <QTest>
#include <KoColorSpaceRegistry.h>
#include <KoChannelInfo.h>

#include "sdk/tests/testpigment.h"

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
