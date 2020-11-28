/*
 *  SPDX-FileCopyrightText: 2008-2009 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2018 Ivan Santa Maria <ghevan@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_RECT_MASK_GENERATOR_H_
#define _KIS_RECT_MASK_GENERATOR_H_

#include <QScopedPointer>
#include "kritaimage_export.h"

#include "kis_mask_generator.h"

/**
 * Represent, serialize and deserialize a rectangular 8-bit mask.
 */
class KRITAIMAGE_EXPORT KisRectangleMaskGenerator : public KisMaskGenerator
{
public:
    struct FastRowProcessor;
public:

    KisRectangleMaskGenerator(qreal radius, qreal ratio, qreal fh, qreal fv, int spikes, bool antialiasEdges);
    KisRectangleMaskGenerator(const KisRectangleMaskGenerator &rhs);
    ~KisRectangleMaskGenerator() override;

    KisMaskGenerator* clone() const override;

    bool shouldSupersample() const override;
    quint8 valueAt(qreal x, qreal y) const override;
    void setScale(qreal scaleX, qreal scaleY) override;
    void setSoftness(qreal softness) override;

    bool shouldVectorize() const override;
    KisBrushMaskApplicatorBase* applicator() override;
    void resetMaskApplicator(bool forceScalar);

private:
    struct Private;
    const QScopedPointer<Private> d;
};

#endif
