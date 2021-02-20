/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.bet
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <KoColorSpaceMaths.h>

#include <cfloat>

#include <QtGlobal>

#ifdef HAVE_OPENEXR
const half KoColorSpaceMathsTraits<half>::zeroValue = 0.0;
const half KoColorSpaceMathsTraits<half>::unitValue = 1.0;
const half KoColorSpaceMathsTraits<half>::halfValue = 0.5;
const half KoColorSpaceMathsTraits<half>::max = HALF_MAX;
const half KoColorSpaceMathsTraits<half>::min = -HALF_MAX;
const half KoColorSpaceMathsTraits<half>::epsilon = HALF_EPSILON;
const KoChannelInfo::enumChannelValueType KoColorSpaceMathsTraits<half>::channelValueType = KoChannelInfo::FLOAT16;
#endif

const float KoColorSpaceMathsTraits<float>::zeroValue = 0.0;
const float KoColorSpaceMathsTraits<float>::unitValue = 1.0;
const float KoColorSpaceMathsTraits<float>::halfValue = 0.5;
const float KoColorSpaceMathsTraits<float>::max = FLT_MAX;
const float KoColorSpaceMathsTraits<float>::min = -FLT_MAX;
const float KoColorSpaceMathsTraits<float>::epsilon = FLT_EPSILON;
const KoChannelInfo::enumChannelValueType KoColorSpaceMathsTraits<float>::channelValueType = KoChannelInfo::FLOAT32;

const double KoColorSpaceMathsTraits<double>::zeroValue = 0.0;
const double KoColorSpaceMathsTraits<double>::unitValue = 1.0;
const double KoColorSpaceMathsTraits<double>::halfValue = 0.5;
const double KoColorSpaceMathsTraits<double>::max = DBL_MAX;
const double KoColorSpaceMathsTraits<double>::min = -DBL_MAX;
const double KoColorSpaceMathsTraits<double>::epsilon = DBL_EPSILON;

const KoChannelInfo::enumChannelValueType KoColorSpaceMathsTraits<double>::channelValueType = KoChannelInfo::FLOAT64;
const KoChannelInfo::enumChannelValueType KoColorSpaceMathsTraits<quint8>::channelValueType = KoChannelInfo::UINT8;
const KoChannelInfo::enumChannelValueType KoColorSpaceMathsTraits<quint16>::channelValueType = KoChannelInfo::UINT16;
const KoChannelInfo::enumChannelValueType KoColorSpaceMathsTraits<qint16>::channelValueType = KoChannelInfo::INT16;
const KoChannelInfo::enumChannelValueType KoColorSpaceMathsTraits<quint32>::channelValueType = KoChannelInfo::UINT32;

const Ko::FullLut< KoIntegerToFloat<quint16>, float, quint16> KoLuts::Uint16ToFloat;
const Ko::FullLut< KoIntegerToFloat<quint8>, float, quint8> KoLuts::Uint8ToFloat;
