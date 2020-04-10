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
#ifndef KOALPHAMASKAPPLICATORFACTORYIMPL_H
#define KOALPHAMASKAPPLICATORFACTORYIMPL_H

#include <KoAlphaMaskApplicatorBase.h>
#include <KoVcMultiArchBuildSupport.h>

template<typename _channels_type_,
         int _channels_nb_,
         int _alpha_pos_>
class KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorFactoryImpl
{
public:
    typedef int ParamType;
    typedef KoAlphaMaskApplicatorBase* ReturnType;

    template<Vc::Implementation _impl>
    static KoAlphaMaskApplicatorBase* create(int);
};


#endif // KOALPHAMASKAPPLICATORFACTORYIMPL_H
