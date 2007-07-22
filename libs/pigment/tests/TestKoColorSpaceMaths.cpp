
#include "TestKoColorSpaceMaths.h"

#include <kdebug.h>

#include "KoColorSpaceMaths.h"

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

QTEST_MAIN(TestKoColorSpaceMaths)
#include "TestKoColorSpaceMaths.moc"

