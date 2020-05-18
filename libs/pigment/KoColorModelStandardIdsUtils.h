/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KOCOLORMODELSTANDARDIDSUTILS_H
#define KOCOLORMODELSTANDARDIDSUTILS_H

#include <KoColorModelStandardIds.h>

#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>
#endif


template <typename channel_type>
KoID colorDepthIdForChannelType();

template<> inline KoID colorDepthIdForChannelType<quint8>() {
    return Integer8BitsColorDepthID;
}

template<> inline KoID colorDepthIdForChannelType<quint16>() {
    return Integer16BitsColorDepthID;
}

#ifdef HAVE_OPENEXR
template<> inline KoID colorDepthIdForChannelType<half>() {
    return Float16BitsColorDepthID;
}
#endif

template<> inline KoID colorDepthIdForChannelType<float>() {
    return Float32BitsColorDepthID;
}

template<> inline KoID colorDepthIdForChannelType<double>() {
    return Float64BitsColorDepthID;
}

template <template <typename T> class Functor,
          typename... Args,
          typename Result = decltype(std::declval<Functor<quint8>>()(std::declval<Args>()...))>
Result channelTypeForColorDepthId(const KoID &depthId, Args... args)
{
    if (depthId == Integer8BitsColorDepthID) {
        return Functor<quint8>()(args...);
    } else if (depthId == Integer16BitsColorDepthID) {
        return Functor<quint16>()(args...);
#ifdef HAVE_OPENEXR
    } else if (depthId == Float16BitsColorDepthID) {
        return Functor<half>()(args...);
#endif
    } else if (depthId == Float32BitsColorDepthID) {
        return Functor<float>()(args...);
    }

    throw std::runtime_error("Invalid bit depth!");
}


#endif // KOCOLORMODELSTANDARDIDSUTILS_H

