/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOCOLORSPACEBLENDINGPOLICY_H
#define KOCOLORSPACEBLENDINGPOLICY_H

#include <kritapigment_export.h>

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

KRITAPIGMENT_EXPORT
bool useSubtractiveBlendingForCmykColorSpaces();

KRITAPIGMENT_EXPORT
QStringList subtractiveBlendingModesInCmyk();

#endif // KOCOLORSPACEBLENDINGPOLICY_H
