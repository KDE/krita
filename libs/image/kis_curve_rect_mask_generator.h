/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
