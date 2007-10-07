/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _KO_FALL_BACK_COLOR_TRANSFORMATION_H_
#define _KO_FALL_BACK_COLOR_TRANSFORMATION_H_

#include "KoColorTransformation.h"

#include <pigment_export.h>

class KoColorSpace;
class KoColorTransformation;

/**
 * Use this color transformation to encapsulate an other KoColorTransformation
 * and perform a color conversion before and after using that KoColorTransformation.
 */
class PIGMENT_EXPORT KoFallBackColorTransformation : public KoColorTransformation {
    public:
        /**
         * @param cs color space of the source and destination pixels
         * @param fallBackCS color space use natively by the color transformation
         * @param transfo the color transformation
         */
        KoFallBackColorTransformation(const KoColorSpace* cs, const KoColorSpace* fallBackCS, KoColorTransformation* transfo);
        virtual ~KoFallBackColorTransformation();
        virtual void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const;
    private:
        struct Private;
        Private* const d;
};

#endif
