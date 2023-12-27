/*
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef INTEGRAL_FRAME_DURATION_H
#define INTEGRAL_FRAME_DURATION_H

#include <cmath>

#include <klocalizedstring.h>

#include <KoID.h>
#include <kis_image.h>
#include <kis_image_animation_interface.h>

#include "KisExportCheckRegistry.h"
#include "kritaimpex_export.h"

class KRITAIMPEX_EXPORT IntegralFrameDurationCheck : public KisExportCheckBase
{
public:
    IntegralFrameDurationCheck(const QString &id,
                               Level level,
                               const QString &customWarning = QString())
        : KisExportCheckBase(id, level, customWarning)
    {
        if (customWarning.isEmpty()) {
            m_warning =
                i18nc("image conversion warning",
                      "The image is animated with a frame duration in "
                      "<b>fractions of a millisecond</b>. The "
                      "format cannot represent this, and the frame "
                      "duration will be rounded to the nearest millisecond.");
        }
    }

    bool checkNeeded(KisImageSP image) const override
    {
        if (!image->animationInterface()->hasAnimation()) {
            return false;
        }

        const auto frameDuration = 1000.0
            / static_cast<double>(image->animationInterface()->framerate());

        return std::round(frameDuration) != frameDuration;
    }

    Level check(KisImageSP /*image*/) const override
    {
        return m_level;
    }
};

class KRITAIMPEX_EXPORT IntegralFrameDurationCheckFactory
    : public KisExportCheckFactory
{
public:
    IntegralFrameDurationCheckFactory() = default;

    ~IntegralFrameDurationCheckFactory() override = default;

    KisExportCheckBase *
    create(KisExportCheckBase::Level level,
           const QString &customWarning = QString()) override
    {
        return new IntegralFrameDurationCheck(id(), level, customWarning);
    }

    QString id() const override
    {
        return "IntegralFrameDurationCheck";
    }
};

#endif // INTEGRAL_FRAME_DURATION_H
