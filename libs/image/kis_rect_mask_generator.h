/*
 *  SPDX-FileCopyrightText: 2008-2009 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2018 Ivan Santa Maria <ghevan@gmail.com>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_RECT_MASK_GENERATOR_H_
#define _KIS_RECT_MASK_GENERATOR_H_

#include "kritaimage_export.h"

#include <QScopedPointer>

#include "kis_base_mask_generator.h"

template<typename V>
class FastRowProcessor;

/**
 * Represent, serialize and deserialize a rectangular 8-bit mask.
 */
class KRITAIMAGE_EXPORT KisRectangleMaskGenerator : public KisMaskGenerator
{
public:

    KisRectangleMaskGenerator(qreal radius, qreal ratio, qreal fh, qreal fv, int spikes, bool antialiasEdges);
    KisRectangleMaskGenerator(const KisRectangleMaskGenerator &rhs);
    ~KisRectangleMaskGenerator() override;

    KisMaskGenerator* clone() const override;

    quint8 valueAt(qreal x, qreal y) const override;
    void setScale(qreal scaleX, qreal scaleY) override;
    void setSoftness(qreal softness) override;

    bool shouldVectorize() const override;
    KisBrushMaskApplicatorBase* applicator() override;
    void resetMaskApplicator(bool forceScalar);

private:
    struct Private;
    const QScopedPointer<Private> d;

    friend class FastRowProcessor<KisRectangleMaskGenerator>;
};

#endif
