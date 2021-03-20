/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

