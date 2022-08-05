/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_BRUSH_MASK_APPLICATOR_BASE_H
#define __KIS_BRUSH_MASK_APPLICATOR_BASE_H

#include <cmath>

#include "kis_fixed_paint_device.h"
#include "kis_types.h"

struct MaskProcessingData {
    MaskProcessingData(KisFixedPaintDeviceSP _device,
                       const KoColorSpace *_colorSpace,
                       const quint8 *_color,
                       qreal _randomness,
                       qreal _density,
                       double _centerX,
                       double _centerY,
                       double _angle)
        : device(_device)
        , colorSpace(_colorSpace)
        , color(_color)
        , randomness(_randomness)
        , density(_density)
        , centerX(_centerX)
        , centerY(_centerY)
        , cosa(std::cos(_angle))
        , sina(std::sin(_angle))
        , pixelSize(colorSpace->pixelSize())
    {
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

    quint32 pixelSize;
};

class KisBrushMaskApplicatorBase
{
public:
    virtual ~KisBrushMaskApplicatorBase() = default;
    virtual void process(const QRect &rect) = 0;

    inline void initializeData(const MaskProcessingData *data) {
        m_d = data;
    }

protected:
    const MaskProcessingData *m_d = nullptr;
};

struct OperatorWrapper {
    OperatorWrapper(KisBrushMaskApplicatorBase *applicator)
        : m_applicator(applicator) {}

    inline void operator()(const QRect &rect) const
    {
        m_applicator->process(rect);
    }

    KisBrushMaskApplicatorBase *m_applicator;
};

#endif /* __KIS_BRUSH_MASK_APPLICATOR_BASE_H */
