/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisDitherOpImpl.h"

template<class srcCSTraits> inline void addStandardDitherOps(KoColorSpace *cs)
{
    static_assert(std::is_same<srcCSTraits, KoXyzU8Traits>::value || std::is_same<srcCSTraits, KoXyzU16Traits>::value ||
#ifdef HAVE_OPENEXR
                      std::is_same<srcCSTraits, KoXyzF16Traits>::value ||
#endif
                      std::is_same<srcCSTraits, KoXyzF32Traits>::value,
                  "Missing colorspace, add a transform case!");

    addDitherOpsByDepth<srcCSTraits, KoXyzU8Traits>(cs, Integer8BitsColorDepthID);
    addDitherOpsByDepth<srcCSTraits, KoXyzU16Traits>(cs, Integer16BitsColorDepthID);
#ifdef HAVE_OPENEXR
    addDitherOpsByDepth<srcCSTraits, KoXyzF16Traits>(cs, Float16BitsColorDepthID);
#endif
    addDitherOpsByDepth<srcCSTraits, KoXyzF32Traits>(cs, Float32BitsColorDepthID);
}
