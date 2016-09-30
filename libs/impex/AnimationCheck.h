/*
 * Copyright (C) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef AnimationCHECK_H
#define AnimationCHECK_H

#include "KisExportCheckRegistry.h"
#include <KoID.h>
#include <klocalizedstring.h>
#include <kis_image.h>
#include <kis_image_animation_interface.h>
#include <KoColorSpace.h>

class AnimationCheck : public KisExportCheckBase
{
public:

    AnimationCheck(const QString &id, Level level, const QString &customWarning = QString())
        : KisExportCheckBase(id, level, customWarning, true)
    {
        if (customWarning.isEmpty()) {
            m_warning = i18nc("image conversion warning", "This image has <b>animated layers</b>. Animation cannot be saved to this format.");
        }
    }

    bool checkNeeded(KisImageSP image) const
    {
        return image->animationInterface()->hasAnimation();
    }

    Level check(KisImageSP /*image*/) const
    {
        return m_level;
    }
};

class AnimationCheckFactory : public KisExportCheckFactory
{
public:

    AnimationCheckFactory() {}

    virtual ~AnimationCheckFactory() {}

    KisExportCheckBase *create(KisExportCheckBase::Level level, const QString &customWarning)
    {
        return new AnimationCheck(id(), level, customWarning);
    }

    QString id() const {
        return "AnimationCheck";
    }
};

#endif // AnimationCHECK_H
