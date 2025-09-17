/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOCOLORSPACEBLENDINGPOLICY_H
#define KOCOLORSPACEBLENDINGPOLICY_H

#include <kritapigment_export.h>

#include <KisQStringListFwd.h>

/**
 * @brief default blending policy used in additive color spaces
 */
template<typename Traits>
struct KoAdditiveBlendingPolicy
{
    using channels_type = typename Traits::channels_type;
    inline static channels_type toAdditiveSpace(channels_type value) {
        return value;
    }

    inline static channels_type fromAdditiveSpace(channels_type value) {
        return value;
    }
};

/**
 * @brief a plending policy used for subtractive color spaces (e.g. CMYK)
 *
 * In CMYK we should first invert the colors to make them "additive",
 * and then blend.
 */
template<typename Traits>
struct KoSubtractiveBlendingPolicy
{
    using channels_type = typename Traits::channels_type;

    inline static channels_type toAdditiveSpace(channels_type value) {
        return Traits::math_trait::unitValue - value;
    }

    inline static channels_type fromAdditiveSpace(channels_type value) {
        return Traits::math_trait::unitValue - value;
    }
};

/**
 * @return false if the user selected the legacy behavior of the blendmodes in CMYK color spaces
 */
KRITAPIGMENT_EXPORT
bool useSubtractiveBlendingForCmykColorSpaces();

/**
 * @brief the list of blendmodes that perform channel-inversion in CMYK color space
 */
KRITAPIGMENT_EXPORT
QStringList subtractiveBlendingModesInCmyk();

#endif // KOCOLORSPACEBLENDINGPOLICY_H
