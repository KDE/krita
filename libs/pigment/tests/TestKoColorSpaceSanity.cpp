/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#include "TestKoColorSpaceSanity.h"

#include <qtest_kde.h>
#include <KoColorSpaceRegistry.h>


void TestKoColorSpaceSanity::testChannelsInfo()
{
    foreach(const KoColorSpace* colorSpace, KoColorSpaceRegistry::instance()->allColorSpaces(KoColorSpaceRegistry::AllColorSpaces, KoColorSpaceRegistry::OnlyDefaultProfile))
    {
        qDebug() << colorSpace->id();
        QCOMPARE(colorSpace->channelCount(), quint32(colorSpace->channels().size()));
        QList<int> indexes;
        quint32 colorChannels = 0;
        quint32 size = 0;
        foreach(KoChannelInfo* info, colorSpace->channels())
        {
            if(info->channelType() == KoChannelInfo::COLOR ) {
                ++colorChannels;
            }
            // Check poses
            qint32 pos = info->pos();
            QVERIFY(pos + info->size() <= (qint32)colorSpace->pixelSize());
            foreach(KoChannelInfo* info2, colorSpace->channels())
            {
                if( info != info2 )
                {
                    QVERIFY( pos >= (info2->pos() + info2->size()) || pos + info->size() <= info2->pos());
                }
            }

            // Check index
            quint32 index = info->index();
            QVERIFY(index < colorSpace->channelCount());
            QVERIFY(indexes.indexOf(index) == -1);
            indexes.push_back(index);

            size += info->size();
        }
        QCOMPARE(size, colorSpace->pixelSize());
        QCOMPARE(colorSpace->colorChannelCount(), colorChannels);
    }
}

QTEST_KDEMAIN(TestKoColorSpaceSanity, NoGUI)
#include "TestKoColorSpaceSanity.moc"
