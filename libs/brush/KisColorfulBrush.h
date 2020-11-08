/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISCOLORFULBRUSH_H
#define KISCOLORFULBRUSH_H

#include "kis_scaling_size_brush.h"


class BRUSH_EXPORT KisColorfulBrush : public KisScalingSizeBrush
{
public:
    KisColorfulBrush() = default;
    KisColorfulBrush(const QString& filename);
    KisColorfulBrush(const KisColorfulBrush &rhs) = default;

    QImage brushTipImage() const override;

    virtual void setAdjustmentMidPoint(quint8 value);
    virtual void setBrightnessAdjustment(qreal value);
    virtual void setContrastAdjustment(qreal value);

    virtual bool hasColor() const;

    quint8 adjustmentMidPoint() const;
    qreal brightnessAdjustment() const;
    qreal contrastAdjustment() const;

    void toXML(QDomDocument& d, QDomElement& e) const override;

private:
    quint8 m_adjustmentMidPoint = 127;
    qreal m_brightnessAdjustment = 0.0;
    qreal m_contrastAdjustment = 0.0;
};

#endif // KISCOLORFULBRUSH_H
