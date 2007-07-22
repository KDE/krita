/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.bet
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

#include <cfloat>

#include <qglobal.h>

#include <KoColorSpaceMaths.h>

#ifdef HAVE_OPENEXR
const half KoColorSpaceMathsTraits<half>::zeroValue = 0.0;
const half KoColorSpaceMathsTraits<half>::unitValue = 1.0;
const half KoColorSpaceMathsTraits<half>::max = HALF_MAX;
const half KoColorSpaceMathsTraits<half>::min = -HALF_MAX;
const half KoColorSpaceMathsTraits<half>::epsilon = HALF_EPSILON;
const KoChannelInfo::enumChannelValueType KoColorSpaceMathsTraits<half>::channelValueType = KoChannelInfo::FLOAT16;
#endif

const float KoColorSpaceMathsTraits<float>::zeroValue = 0.0;
const float KoColorSpaceMathsTraits<float>::unitValue = 1.0;
const float KoColorSpaceMathsTraits<float>::max = FLT_MAX;
const float KoColorSpaceMathsTraits<float>::min = -FLT_MAX;
const float KoColorSpaceMathsTraits<float>::epsilon = FLT_EPSILON;
const KoChannelInfo::enumChannelValueType KoColorSpaceMathsTraits<float>::channelValueType = KoChannelInfo::FLOAT32;

const KoChannelInfo::enumChannelValueType KoColorSpaceMathsTraits<quint8>::channelValueType = KoChannelInfo::UINT8;
const KoChannelInfo::enumChannelValueType KoColorSpaceMathsTraits<quint16>::channelValueType = KoChannelInfo::UINT16;
const KoChannelInfo::enumChannelValueType KoColorSpaceMathsTraits<qint16>::channelValueType = KoChannelInfo::INT16;
const KoChannelInfo::enumChannelValueType KoColorSpaceMathsTraits<quint32>::channelValueType = KoChannelInfo::UINT32;

