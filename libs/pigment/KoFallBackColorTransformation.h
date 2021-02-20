/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _KO_FALL_BACK_COLOR_TRANSFORMATION_H_
#define _KO_FALL_BACK_COLOR_TRANSFORMATION_H_

#include "KoColorTransformation.h"

#include "kritapigment_export.h"

class KoColorSpace;
class KoColorConversionTransformation;

/**
 * Use this color transformation to encapsulate another KoColorTransformation
 * and perform a color conversion before and after using that KoColorTransformation.
 */
class KRITAPIGMENT_EXPORT KoFallBackColorTransformation : public KoColorTransformation
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
    ~KoFallBackColorTransformation() override;
    void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const override;
    QList<QString> parameters() const override;
    int parameterId(const QString& name) const override;
    void setParameter(int id, const QVariant& parameter) override;
private:
    struct Private;
    Private* const d;
};

#endif
