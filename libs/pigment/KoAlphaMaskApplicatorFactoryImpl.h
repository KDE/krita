/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOALPHAMASKAPPLICATORFACTORYIMPL_H
#define KOALPHAMASKAPPLICATORFACTORYIMPL_H

#include <KoAlphaMaskApplicatorBase.h>
#include <KoMultiArchBuildSupport.h>

template<typename _channels_type_,
         int _channels_nb_,
         int _alpha_pos_>
class KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorFactoryImpl
{
public:
    using ParamType = int;
    using ReturnType = KoAlphaMaskApplicatorBase *;

    template<typename _impl>
    static KoAlphaMaskApplicatorBase* create(int);
};


#endif // KOALPHAMASKAPPLICATORFACTORYIMPL_H
