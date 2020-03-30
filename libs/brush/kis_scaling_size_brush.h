/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISSCALINGSIZEBRUSH_H
#define KISSCALINGSIZEBRUSH_H

#include "kritabrush_export.h"
#include "kis_brush.h"


class BRUSH_EXPORT KisScalingSizeBrush : public KisBrush
{
public:

    KisScalingSizeBrush();
    KisScalingSizeBrush(const QString& filename);
    KisScalingSizeBrush(const KisScalingSizeBrush &rhs);

    qreal userEffectiveSize() const override;
    void setUserEffectiveSize(qreal value) override;

    /**
     * If the brush image data are colorful (e.g. you created the brush from the canvas with custom brush)
     * and you want to paint with it as with masks, set to true.
     */
    virtual void setUseColorAsMask(bool useColorAsMask);
    bool useColorAsMask() const;

    QImage brushTipImage() const override;

    void setBrightnessAdjustment(qreal value);
    void setContrastAdjustment(qreal value);

    qreal brightnessAdjustment() const;
    qreal contrastAdjustment() const;

    void toXML(QDomDocument& d, QDomElement& e) const override;

private:
    bool m_useColorAsMask = false;
    qreal m_brightnessAdjustment = 0.0;
    qreal m_contrastAdjustment = 0.0;
};

#endif // KISSCALINGSIZEBRUSH_H
