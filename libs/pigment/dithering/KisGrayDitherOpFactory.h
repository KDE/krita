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
    static_assert(std::is_same<srcCSTraits, KoGrayU8Traits>::value || std::is_same<srcCSTraits, KoGrayU16Traits>::value ||
#ifdef HAVE_OPENEXR
                      std::is_same<srcCSTraits, KoGrayF16Traits>::value ||
#endif
                      std::is_same<srcCSTraits, KoGrayF32Traits>::value,
                  "Missing colorspace, add a transform case!");

    addDitherOpsByDepth<srcCSTraits, KoGrayU8Traits>(cs, Integer8BitsColorDepthID);
    addDitherOpsByDepth<srcCSTraits, KoGrayU16Traits>(cs, Integer16BitsColorDepthID);
#ifdef HAVE_OPENEXR
    addDitherOpsByDepth<srcCSTraits, KoGrayF16Traits>(cs, Float16BitsColorDepthID);
#endif
    addDitherOpsByDepth<srcCSTraits, KoGrayF32Traits>(cs, Float32BitsColorDepthID);
}
