/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2011 Geoffry Song <goffrie@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_GAUSS_MASK_GENERATOR_H_
#define _KIS_GAUSS_MASK_GENERATOR_H_

#include "kritaimage_export.h"

#include "kis_mask_generator.h"
#include <QScopedPointer>

/**
 * This mask generator uses a Gaussian-blurred circle
 */
class KRITAIMAGE_EXPORT KisGaussCircleMaskGenerator : public KisMaskGenerator
{
public:
    struct FastRowProcessor;
public:

    KisGaussCircleMaskGenerator(qreal diameter, qreal ratio, qreal fh, qreal fv, int spikes, bool antialiasEdges);
    KisGaussCircleMaskGenerator(const KisGaussCircleMaskGenerator &rhs);
    ~KisGaussCircleMaskGenerator() override;
    KisMaskGenerator* clone() const override;

    quint8 valueAt(qreal x, qreal y) const override;

    void setScale(qreal scaleX, qreal scaleY) override;

    bool shouldVectorize() const override;
    KisBrushMaskApplicatorBase* applicator() override;

    void resetMaskApplicator(bool forceScalar);

private:

    qreal norme(qreal a, qreal b) const {
        return a*a + b*b;
    }

private:
    struct Private;
    const QScopedPointer<Private> d;
};

#endif
