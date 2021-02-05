/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
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

    virtual bool isImageType() const;

    quint8 adjustmentMidPoint() const;
    qreal brightnessAdjustment() const;
    qreal contrastAdjustment() const;

    void toXML(QDomDocument& d, QDomElement& e) const override;

    void setHasColorAndTransparency(bool value);
    bool hasColorAndTransparency() const;

private:
    quint8 m_adjustmentMidPoint = 127;
    qreal m_brightnessAdjustment = 0.0;
    qreal m_contrastAdjustment = 0.0;
    bool m_hasColorAndTransparency = false;
};

#endif // KISCOLORFULBRUSH_H
