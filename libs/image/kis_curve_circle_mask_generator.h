/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_CURVE_CIRCLE_MASK_GENERATOR_H_
#define _KIS_CURVE_CIRCLE_MASK_GENERATOR_H_

#include <QList>
#include <QVector>
#include <QScopedPointer>

#include "kritaimage_export.h"
#include "kis_mask_generator.h"

class KisCubicCurve;
class QDomElement;
class QDomDocument;

class QPointF;


/**
 * This mask generator use softness/hardness defined by user curve
 * It used to be soft brush paintop. 
 */
class KRITAIMAGE_EXPORT KisCurveCircleMaskGenerator : public KisMaskGenerator
{
public:
    struct FastRowProcessor;
public:

    KisCurveCircleMaskGenerator(qreal radius, qreal ratio, qreal fh, qreal fv, int spikes,const KisCubicCurve& curve, bool antialiasEdges);
    KisCurveCircleMaskGenerator(const KisCurveCircleMaskGenerator &rhs);
    ~KisCurveCircleMaskGenerator() override;
    KisMaskGenerator* clone() const override;

    quint8 valueAt(qreal x, qreal y) const override;

    void setScale(qreal scaleX, qreal scaleY) override;

    void toXML(QDomDocument& , QDomElement&) const override;
    void setSoftness(qreal softness) override;

    bool shouldVectorize() const override;
    KisBrushMaskApplicatorBase* applicator() override;

    void resetMaskApplicator(bool forceScalar);

    static void transformCurveForSoftness(qreal softness,const QList<QPointF> &points, int curveResolution, QVector<qreal> &result);

private:

    qreal norme(qreal a, qreal b) const {
        return a*a + b*b;
    }

private:
    struct Private;
    const QScopedPointer<Private> d;
};

#endif
