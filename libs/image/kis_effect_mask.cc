/*
 *  SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_effect_mask.h"

#include <KoIcon.h>
#include <kis_icon.h>
#include "kis_image.h"

KisEffectMask::KisEffectMask(KisImageWSP image, const QString &name)
        : KisMask(image, name)
{
}

KisEffectMask::~KisEffectMask()
{
}

KisEffectMask::KisEffectMask(const KisEffectMask& rhs)
        : KisMask(rhs)
{
}

QIcon KisEffectMask::icon() const
{
    return KisIconUtils::loadIcon("bookmarks");
}

