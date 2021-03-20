/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2011 Geoffry Song <goffrie@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_GAUSS_RECT_MASK_GENERATOR_H_
#define _KIS_GAUSS_RECT_MASK_GENERATOR_H_

#include "kritaimage_export.h"

#include "kis_mask_generator.h"

/**
 * This mask generator uses a Gaussian-blurred rectangle
 */
class KRITAIMAGE_EXPORT KisGaussRectangleMaskGenerator : public KisMaskGenerator
{
public:
    struct FastRowProcessor;
public:

    KisGaussRectangleMaskGenerator(qreal diameter, qreal ratio, qreal fh, qreal fv, int spikes, bool antialiasEdges);
    KisGaussRectangleMaskGenerator(const KisGaussRectangleMaskGenerator &rhs);
    ~KisGaussRectangleMaskGenerator() override;
    KisMaskGenerator* clone() const override;

    quint8 valueAt(qreal x, qreal y) const override;
    void setScale(qreal scaleX, qreal scaleY) override;

    bool shouldVectorize() const override;
    KisBrushMaskApplicatorBase* applicator() override;
    void resetMaskApplicator(bool forceScalar);

private:
    struct Private;
    const QScopedPointer<Private> d;
};

#endif
