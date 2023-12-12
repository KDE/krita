/*
 *  SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOCLIPMASKAPPLICATORFACTORYIMPL_H
#define KOCLIPMASKAPPLICATORFACTORYIMPL_H

#include <KoClipMaskApplicator.h>
#include <KoMultiArchBuildSupport.h>

struct KoClipMaskApplicatorFactoryImpl {
    template<typename _impl>
    static KoClipMaskApplicatorBase* create();
};

#endif // KOCLIPMASKAPPLICATORFACTORYIMPL_H
