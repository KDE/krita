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


#ifndef KOALPHAMASKAPPLICATORFACTORY_H
#define KOALPHAMASKAPPLICATORFACTORY_H

#include "kritapigment_export.h"
#include <KoVcMultiArchBuildSupport.h>
#include <KoAlphaMaskApplicatorBase.h>

#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>
#endif

template<typename _channels_type_,
         int _channels_nb_,
         int _alpha_pos_>
class KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorFactory
{
public:
    typedef int ParamType;
    typedef KoAlphaMaskApplicatorBase* ReturnType;

    template<Vc::Implementation _impl>
    static KoAlphaMaskApplicatorBase* create(int);
};


extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<quint8,  4, 3>::create<Vc::CurrentImplementation::current()>(int);
extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<quint16, 4, 3>::create<Vc::CurrentImplementation::current()>(int);
#ifdef HAVE_OPENEXR
extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<half,    4, 3>::create<Vc::CurrentImplementation::current()>(int);
#endif
extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<float,   4, 3>::create<Vc::CurrentImplementation::current()>(int);

extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<quint8,  5, 4>::create<Vc::CurrentImplementation::current()>(int);
extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<quint16, 5, 4>::create<Vc::CurrentImplementation::current()>(int);
#ifdef HAVE_OPENEXR
extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<half,    5, 4>::create<Vc::CurrentImplementation::current()>(int);
#endif
extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<float,   5, 4>::create<Vc::CurrentImplementation::current()>(int);

extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<quint8,  2, 1>::create<Vc::CurrentImplementation::current()>(int);
extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<quint16, 2, 1>::create<Vc::CurrentImplementation::current()>(int);
#ifdef HAVE_OPENEXR
extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<half,    2, 1>::create<Vc::CurrentImplementation::current()>(int);
#endif
extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<float,   2, 1>::create<Vc::CurrentImplementation::current()>(int);

extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<quint8,  1, 0>::create<Vc::CurrentImplementation::current()>(int);
extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<quint16, 1, 0>::create<Vc::CurrentImplementation::current()>(int);
#ifdef HAVE_OPENEXR
extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<half,    1, 0>::create<Vc::CurrentImplementation::current()>(int);
#endif
extern template KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase* KoAlphaMaskApplicatorFactory<float,   1, 0>::create<Vc::CurrentImplementation::current()>(int);


#endif // KOALPHAMASKAPPLICATORFACTORY_H
