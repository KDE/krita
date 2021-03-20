/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_CURVE_RECT_MASK_GENERATOR_H_
#define _KIS_CURVE_RECT_MASK_GENERATOR_H_

#include "kritaimage_export.h"

class KisCubicCurve;
class QDomElement;
class QDomDocument;

#include "kis_mask_generator.h"

/**
 * Curve based softness for this rectangular mask generator
 */
class KRITAIMAGE_EXPORT KisCurveRectangleMaskGenerator : public KisMaskGenerator
{
public:
    struct FastRowProcessor;
public:

    KisCurveRectangleMaskGenerator(qreal radius, qreal ratio, qreal fh, qreal fv, int spikes, const KisCubicCurve& curve, bool antialiasEdges);
    KisCurveRectangleMaskGenerator(const KisCurveRectangleMaskGenerator &rhs);
    ~KisCurveRectangleMaskGenerator() override;
    KisMaskGenerator* clone() const override;

    quint8 valueAt(qreal x, qreal y) const override;

    void setScale(qreal scaleX, qreal scaleY) override;

    void toXML(QDomDocument& , QDomElement&) const override;
    
    void setSoftness(qreal softness) override;

    bool shouldVectorize() const override;
    KisBrushMaskApplicatorBase* applicator() override;
    void resetMaskApplicator(bool forceScalar);

private:
    struct Private;
    const QScopedPointer<Private> d;
};

#endif
