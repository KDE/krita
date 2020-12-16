/*
 * SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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

    bool checkNeeded(KisImageSP image) const override
    {
        return image->animationInterface()->hasAnimation();
    }

    Level check(KisImageSP /*image*/) const override
    {
        return m_level;
    }
};

class AnimationCheckFactory : public KisExportCheckFactory
{
public:

    AnimationCheckFactory() {}

    ~AnimationCheckFactory() override {}

    KisExportCheckBase *create(KisExportCheckBase::Level level, const QString &customWarning) override
    {
        return new AnimationCheck(id(), level, customWarning);
    }

    QString id() const override {
        return "AnimationCheck";
    }
};

#endif // AnimationCHECK_H
