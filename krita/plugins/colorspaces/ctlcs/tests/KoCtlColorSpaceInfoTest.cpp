/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include "KoCtlColorSpaceInfoTest.h"

#include <qtest_kde.h>
#include "../KoCtlColorSpaceInfo.h"
#include <KoID.h>
#include <GTLCore/PixelDescription.h>
#include <GTLCore/Type.h>

void KoCtlColorSpaceInfoTest::testCreation()
{
    KoCtlColorSpaceInfo info(FILES_DATA_DIR + QString("rgba32f.ctlcs"));
    QVERIFY(info.load());
    QCOMPARE(info.colorDepthId().id(), QString("F32"));
    QCOMPARE(info.colorModelId().id(), QString("RGBA"));
    QCOMPARE(info.colorModelId().name(), QString("Red Green Blue"));
    QCOMPARE(info.colorSpaceId(), QString("RgbAF32"));
    QCOMPARE(info.name(), QString("RGB (32-bit float/channel) for High Dynamic Range imaging"));
    QCOMPARE(info.defaultProfile(), QString("Standard Linear RGB (scRGB/sRGB64)"));
    QVERIFY(info.isHdr());
    QCOMPARE(info.colorChannelCount(), 3U);
    QCOMPARE(info.pixelSize(), 16U);

    QCOMPARE(info.channels().size(), 4);
    const KoCtlColorSpaceInfo::ChannelInfo* redChannel = info.channels()[0];
    QCOMPARE(redChannel->name(), QString("Red"));
    QCOMPARE(redChannel->shortName(), QString("r"));
    QCOMPARE(redChannel->index(), 2);
    QCOMPARE(redChannel->position(), 8);
    QCOMPARE(redChannel->channelType(), KoChannelInfo::COLOR);
    QCOMPARE(redChannel->valueType(), KoChannelInfo::FLOAT32);
    QCOMPARE(redChannel->size(), 4);
    QCOMPARE(redChannel->color(), QColor(255, 0, 0));

    const KoCtlColorSpaceInfo::ChannelInfo* alphaChannel = info.channels()[3];
    QCOMPARE(alphaChannel->name(), QString("Alpha"));
    QCOMPARE(alphaChannel->shortName(), QString("a"));
    QCOMPARE(alphaChannel->index(), 3);
    QCOMPARE(alphaChannel->position(), 12);
    QCOMPARE(alphaChannel->channelType(), KoChannelInfo::ALPHA);
    QCOMPARE(alphaChannel->valueType(), KoChannelInfo::FLOAT32);
    QCOMPARE(alphaChannel->size(), 4);
    QCOMPARE(alphaChannel->color(), QColor(0, 0, 0));


    const GTLCore::PixelDescription& pd = info.pixelDescription();
    QCOMPARE(qint32(pd.channels()), 4);
    QCOMPARE(qint32(pd.alphaPos()), 3);
    QCOMPARE(pd.channelTypes()[0], GTLCore::Type::Float32);
    QCOMPARE(pd.channelTypes()[1], GTLCore::Type::Float32);
    QCOMPARE(pd.channelTypes()[2], GTLCore::Type::Float32);
    QCOMPARE(pd.channelTypes()[3], GTLCore::Type::Float32);
    QCOMPARE(pd.sameTypeChannels(), true);
    QCOMPARE(qint32(pd.bitsSize()), qint32(8 * info.pixelSize()));
}



QTEST_KDEMAIN(KoCtlColorSpaceInfoTest, NoGUI)
#include "KoCtlColorSpaceInfoTest.moc"
