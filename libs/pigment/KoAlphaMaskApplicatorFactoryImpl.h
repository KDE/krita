/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
