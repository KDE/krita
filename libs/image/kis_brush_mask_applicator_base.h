/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_BRUSH_MASK_APPLICATOR_BASE_H
#define __KIS_BRUSH_MASK_APPLICATOR_BASE_H

#include "kis_types.h"
#include "kis_fixed_paint_device.h"
#include "math.h"


struct MaskProcessingData {
    MaskProcessingData(KisFixedPaintDeviceSP _device,
                       const KoColorSpace* _colorSpace,
                       const quint8* _color,
                       qreal _randomness,
                       qreal _density,
                       double _centerX,
                       double _centerY,
                       double _angle)
        {
            device = _device;
            colorSpace = _colorSpace;
            color = _color;
            randomness = _randomness;
            density = _density;
            centerX = _centerX;
            centerY = _centerY;
            cosa = cos(_angle);
            sina = sin(_angle);
            pixelSize = colorSpace->pixelSize();
        }



    KisFixedPaintDeviceSP device;
    const KoColorSpace* colorSpace;
    const quint8* color;
    qreal randomness;
    qreal density;
    double centerX;
    double centerY;

    double cosa;
    double sina;

    qint32 pixelSize;
};

class KisBrushMaskApplicatorBase
{
public:
    virtual ~KisBrushMaskApplicatorBase() {}
    virtual void process(const QRect &rect) = 0;

    inline void initializeData(const MaskProcessingData *data) {
        m_d = data;
    }

protected:
    const MaskProcessingData *m_d;
};

struct OperatorWrapper {
    OperatorWrapper(KisBrushMaskApplicatorBase *applicator)
        : m_applicator(applicator) {}

    inline void operator() (const QRect& rect) {
        m_applicator->process(rect);
    }

    KisBrushMaskApplicatorBase *m_applicator;
};

#endif /* __KIS_BRUSH_MASK_APPLICATOR_BASE_H */
