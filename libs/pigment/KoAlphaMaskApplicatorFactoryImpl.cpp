/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KoAlphaMaskApplicatorFactoryImpl.h"
#include "KoAlphaMaskApplicator.h"

#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>
#endif

template<typename _channels_type_,
         int _channels_nb_,
         int _alpha_pos_>
template<Vc::Implementation _impl>
KoAlphaMaskApplicatorBase*
KoAlphaMaskApplicatorFactoryImpl<_channels_type_, _channels_nb_, _alpha_pos_>::create(int)
{
    return new KoAlphaMaskApplicator<_channels_type_,
                                     _channels_nb_,
                                     _alpha_pos_,
                                     _impl>();
}

template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<quint8,  4, 3>::create<Vc::CurrentImplementation::current()>(int);
template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<quint16, 4, 3>::create<Vc::CurrentImplementation::current()>(int);
#ifdef HAVE_OPENEXR
template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<half,    4, 3>::create<Vc::CurrentImplementation::current()>(int);
#endif
template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<float,   4, 3>::create<Vc::CurrentImplementation::current()>(int);

template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<quint8,  5, 4>::create<Vc::CurrentImplementation::current()>(int);
template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<quint16, 5, 4>::create<Vc::CurrentImplementation::current()>(int);
#ifdef HAVE_OPENEXR
template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<half,    5, 4>::create<Vc::CurrentImplementation::current()>(int);
#endif
template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<float,   5, 4>::create<Vc::CurrentImplementation::current()>(int);

template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<quint8,  2, 1>::create<Vc::CurrentImplementation::current()>(int);
template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<quint16, 2, 1>::create<Vc::CurrentImplementation::current()>(int);
#ifdef HAVE_OPENEXR
template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<half,    2, 1>::create<Vc::CurrentImplementation::current()>(int);
#endif
template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<float,   2, 1>::create<Vc::CurrentImplementation::current()>(int);

template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<quint8,  1, 0>::create<Vc::CurrentImplementation::current()>(int);
template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<quint16, 1, 0>::create<Vc::CurrentImplementation::current()>(int);
#ifdef HAVE_OPENEXR
template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<half,    1, 0>::create<Vc::CurrentImplementation::current()>(int);
#endif
template KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactoryImpl<float,   1, 0>::create<Vc::CurrentImplementation::current()>(int);
