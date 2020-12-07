/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "TestKoColorSpaceMaths.h"
#include "KoIntegerMaths.h"
#include "KoColorSpaceMaths.h"

#include <QTest>

void TestKoColorSpaceMaths::testColorSpaceMathsTraits()
{
    QCOMPARE(KoColorSpaceMathsTraits<quint8>::channelValueType, KoChannelInfo::UINT8);
    QCOMPARE(KoColorSpaceMathsTraits<quint16>::channelValueType, KoChannelInfo::UINT16);
    QCOMPARE(KoColorSpaceMathsTraits<qint16>::channelValueType, KoChannelInfo::INT16);
    QCOMPARE(KoColorSpaceMathsTraits<quint32>::channelValueType, KoChannelInfo::UINT32);
    QCOMPARE(KoColorSpaceMathsTraits<float>::channelValueType, KoChannelInfo::FLOAT32);
#ifdef HAVE_OPENEXR
    QCOMPARE(KoColorSpaceMathsTraits<half>::channelValueType, KoChannelInfo::FLOAT16);
#endif
}

void TestKoColorSpaceMaths::testScaleToA()
{
    for (int i = 0; i < 256; ++i) {
        quint16 opacity = KoColorSpaceMaths<quint8, quint16 >::scaleToA(i);
        quint8 opacity8 = UINT16_TO_UINT8(opacity);
        QVERIFY(opacity8 == i);
    }
}

QTEST_GUILESS_MAIN(TestKoColorSpaceMaths)
