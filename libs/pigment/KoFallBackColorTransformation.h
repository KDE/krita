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

#include "pigment_export.h"

class KoColorSpace;
class KoColorTransformation;
class KoColorConversionTransformation;

/**
 * Use this color transformation to encapsulate another KoColorTransformation
 * and perform a color conversion before and after using that KoColorTransformation.
 */
class PIGMENTCMS_EXPORT KoFallBackColorTransformation : public KoColorTransformation
{
public:
    /**
     * Create a fall back color transformation using the given two color
     * spaces. This constructor will initialize his own color conversion
     * objects.
     *
     * The created object takes owner ship of the transormation and will
     * take charge of deleting it.
     *
     * @param cs color space of the source and destination pixels
     * @param fallBackCS color space use natively by the color transformation
     * @param transfo the color transformation (working in the fallback color space)
     */
    KoFallBackColorTransformation(const KoColorSpace* _cs, const KoColorSpace* _fallBackCS, KoColorTransformation* _transfo);
    /**
     * Creates a fall back color transformation using the two transformations
     * given as parameters. The created object take ownership of the
     * conversion and the color transformations and will be in charge of
     * deleting them.
     *
     * @param csToFallBack transformation from the color space to the fallback
     * @param fallBackToCs transformation from the fallback to the color space
     * @param transfo the color transformation (working in the fallback color space)
     */
    KoFallBackColorTransformation(KoColorConversionTransformation* _csToFallBack, KoColorConversionTransformation* _fallBackToCs, KoColorTransformation* _transfo);
    virtual ~KoFallBackColorTransformation();
    virtual void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const;
    virtual QList<QString> parameters() const;
    virtual int parameterId(const QString& name) const;
    virtual void setParameter(int id, const QVariant& parameter);
private:
    struct Private;
    Private* const d;
};

#endif
