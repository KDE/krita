/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _KO_COLOR_TRANSFORMATION_H_
#define _KO_COLOR_TRANSFORMATION_H_

#include <QHash>

#include "kritapigment_export.h"

class QVariant;
class QString;


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
class KRITAPIGMENT_EXPORT KoColorTransformation
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
     * @return the list of parameters
     */
    virtual QList<QString> parameters() const;
    /**
     * Get the parameter id for a parameter name
     */
    virtual int parameterId(const QString& name) const;

    void setParameters(const QHash<QString, QVariant> & parameters);
    /**
     * Update one parameter of a cached transformation object.
     *
     */
    virtual void setParameter(int id, const QVariant& parameter);

    /// @return true
    virtual bool isValid() const { return true; }
};

#endif
