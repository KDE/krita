/*
 *  SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOCLIPMASKAPPLICATORBASE_H
#define KOCLIPMASKAPPLICATORBASE_H

#include <KoStreamedMath.h>
#include <QDebug>

/** ClipMaskApplicator allows us to use xsimd functionality to speed up clipmask painting **/

struct KoClipMaskApplicatorBase {

    /**
     * @brief applyLuminanceMask
     * This applies an ARGB32 mask to an ARGB32 image as per w3c specs. Both the alpha channel as well
     * as the rec709 luminance of the mask will be taken into account to calculate the
     * final alpha.
     * @param pixels -- pointer to the image pixels.
     * @param maskPixels -- pointer to the mask pixels.
     * @param nPixels -- total amount of pixels to manipulate, typical width*height.
     */
    virtual void applyLuminanceMask(quint8 *pixels,
                                    quint8 *maskPixels,
                                    const int nPixels) const = 0;

    /**
     * @brief fallbackLuminanceMask
     * This is the fallback algorithm for leftover pixels that for whatever reason cannot be processed via xsimd.
     * @see applyLuminanceMask
     */
    virtual void fallbackLuminanceMask(quint8 *pixels,
                               quint8 *maskPixels,
                               const int nPixels) const;

    virtual ~KoClipMaskApplicatorBase() = default;
};

#endif // KOCLIPMASKAPPLICATORBASE_H
