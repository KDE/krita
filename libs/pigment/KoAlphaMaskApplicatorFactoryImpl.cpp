/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoAlphaMaskApplicatorFactoryImpl.h"

#if XSIMD_UNIVERSAL_BUILD_PASS
#include "KoAlphaMaskApplicator.h"

#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>
#endif

template<typename _channels_type_, int _channels_nb_, int _alpha_pos_>
template<typename _impl>
KoAlphaMaskApplicatorBase *
KoAlphaMaskApplicatorFactoryImpl<_channels_type_, _channels_nb_, _alpha_pos_>::
    create()
{
    return new KoAlphaMaskApplicator<_channels_type_,
                                     _channels_nb_,
                                     _alpha_pos_,
                                     _impl>();
}

template KoAlphaMaskApplicatorBase *
KoAlphaMaskApplicatorFactoryImpl<quint8, 4, 3>::create<xsimd::current_arch>();
template KoAlphaMaskApplicatorBase *
KoAlphaMaskApplicatorFactoryImpl<quint16, 4, 3>::create<xsimd::current_arch>();
#ifdef HAVE_OPENEXR
template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<half,    4, 3>::create<xsimd::current_arch>();
#endif
template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<float,   4, 3>::create<xsimd::current_arch>();

template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<quint8,  5, 4>::create<xsimd::current_arch>();
template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<quint16, 5, 4>::create<xsimd::current_arch>();
#ifdef HAVE_OPENEXR
template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<half,    5, 4>::create<xsimd::current_arch>();
#endif
template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<float,   5, 4>::create<xsimd::current_arch>();

template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<quint8,  2, 1>::create<xsimd::current_arch>();
template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<quint16, 2, 1>::create<xsimd::current_arch>();
#ifdef HAVE_OPENEXR
template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<half,    2, 1>::create<xsimd::current_arch>();
#endif
template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<float,   2, 1>::create<xsimd::current_arch>();

template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<quint8,  1, 0>::create<xsimd::current_arch>();
template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<quint16, 1, 0>::create<xsimd::current_arch>();
#ifdef HAVE_OPENEXR
template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<half,    1, 0>::create<xsimd::current_arch>();
#endif
template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<float,   1, 0>::create<xsimd::current_arch>();

#endif // XSIMD_UNIVERSAL_BUILD_PASS
