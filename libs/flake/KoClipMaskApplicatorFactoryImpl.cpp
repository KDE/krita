/*
 *  SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoClipMaskApplicatorFactoryImpl.h"

template<>
KoClipMaskApplicatorBase * KoClipMaskApplicatorFactoryImpl::create<xsimd::current_arch>()
{
    return new KoClipMaskApplicator<xsimd::current_arch>();
}
