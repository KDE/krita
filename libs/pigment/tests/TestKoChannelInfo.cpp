/*
 *  Copyright (C) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "TestKoChannelInfo.h"

#include <QTest>

#include <QDomElement>

#include "KoColorModelStandardIds.h"

#include "KoColor.h"
#include "KoChannelInfo.h"
#include "DebugPigment.h"

void TestKoChannelInfo::testDisplayPositionToChannelIndex()
{
    QList<KoChannelInfo*> channels;
    channels << new KoChannelInfo(i18n("Blue") , 0, 2, KoChannelInfo::COLOR, KoChannelInfo::UINT8, 1, QColor(0, 0, 255))
             << new KoChannelInfo(i18n("Green"), 1, 1, KoChannelInfo::COLOR, KoChannelInfo::UINT8, 1, QColor(0, 255, 0))
             << new KoChannelInfo(i18n("Red")  , 2, 0, KoChannelInfo::COLOR, KoChannelInfo::UINT8, 1, QColor(255, 0, 0))
             << new KoChannelInfo(i18n("Alpha"), 3, 3, KoChannelInfo::ALPHA, KoChannelInfo::UINT8);

    QCOMPARE(KoChannelInfo::displayPositionToChannelIndex(0, channels), 2);
    QCOMPARE(KoChannelInfo::displayPositionToChannelIndex(1, channels), 1);
    QCOMPARE(KoChannelInfo::displayPositionToChannelIndex(2, channels), 0);
    QCOMPARE(KoChannelInfo::displayPositionToChannelIndex(3, channels), 3);
}

void TestKoChannelInfo::testdisplayOrderSorted()
{
    QList<KoChannelInfo*> channels;
    channels << new KoChannelInfo(i18n("Blue") , 0, 2, KoChannelInfo::COLOR, KoChannelInfo::UINT8, 1, QColor(0, 0, 255))
             << new KoChannelInfo(i18n("Green"), 1, 1, KoChannelInfo::COLOR, KoChannelInfo::UINT8, 1, QColor(0, 255, 0))
             << new KoChannelInfo(i18n("Red")  , 2, 0, KoChannelInfo::COLOR, KoChannelInfo::UINT8, 1, QColor(255, 0, 0))
             << new KoChannelInfo(i18n("Alpha"), 3, 3, KoChannelInfo::ALPHA, KoChannelInfo::UINT8);

    QList<KoChannelInfo*> sortedChannels = KoChannelInfo::displayOrderSorted(channels);
    QCOMPARE(sortedChannels[0]->displayPosition(), 0);
    QCOMPARE(sortedChannels[1]->displayPosition(), 1);
    QCOMPARE(sortedChannels[2]->displayPosition(), 2);
    QCOMPARE(sortedChannels[3]->displayPosition(), 3);
}

QTEST_GUILESS_MAIN(TestKoChannelInfo)
