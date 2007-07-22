#ifndef KIS_RGB_FLOAT_HDR_COLORSPACE_TEST_H
#define KIS_RGB_FLOAT_HDR_COLORSPACE_TEST_H

#include <QtTest/QtTest>

#include "config-openexr.h"

#include "KoChannelInfo.h"

class KisRgbFloatHDRColorSpaceTest : public QObject
{
    Q_OBJECT

private:
    template <class ColorSpaceTraits>
    void testChannels(const QString &colorSpaceId, const KoChannelInfo::enumChannelValueType channelValueType);

private slots:
    void testFactory();
    void testProfile();

    void testF32Channels();

#ifdef HAVE_OPENEXR
    void testF16Channels();
#endif
};

#endif

