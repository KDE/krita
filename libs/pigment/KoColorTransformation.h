/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KO_COLOR_TRANSFORMATION_H_
#define _KO_COLOR_TRANSFORMATION_H_

#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <qglobal.h>

#include "pigment_export.h"


/**
 * This is the base class of all color transform that takes n pixels in input
 * and n pixels in output.
 *
 * They are created by color spaces.
 *
 * For instance:
 * @code
 * KoColorSpace* cs = KoColorSpaceRegistry::rgb8();
 * quint8 pixelsSrc[ nbpixels * cs->pixelSize() ];
 * quint8 pixelsDst[ nbpixels * cs->pixelSize() ];
 * KoColorTransformation* transfo = cs->createInvertTransformation();
 * transfo->transform( pixelsSrc, pixelsDst, nbpixels );
 * @endcode
 */
class PIGMENTCMS_EXPORT KoColorTransformation
{
public:
    virtual ~KoColorTransformation();
    /**
     * This function apply the transformation on a given number of pixels.
     *
     * @param src a pointer to the source pixels
     * @param dst a pointer to the destination pixels
     * @param nPixels the number of pixels
     *
     * This function may or may not be thread safe. You need to create one
     * KoColorTransformation per thread.
     */
    virtual void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const = 0;

    /**
     * Update the parameters of a cached transformation object.
     * The default implementation recursively calls \ref setParameter
     */
    virtual void setParameters(const QHash<QString, QVariant> & parameters);

    /**
     * Update one parameter of a cached transformation object.
     *
     */
    virtual void setParameter(const QString& name, const QVariant& parameter);
};

#endif
