/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef _KIS_TEXT_BRUSH_H_
#define _KIS_TEXT_BRUSH_H_

#include <QFont>

#include "kis_brush.h"
#include "krita_export.h"
#include "kis_gbr_brush.h"

class KisTextBrushesPipe;


class BRUSH_EXPORT KisTextBrush : public KisBrush
{

public:
    KisTextBrush();
    KisTextBrush(const KisTextBrush &rhs);
    virtual ~KisTextBrush();

    void notifyCachedDabPainted();

    void generateMaskAndApplyMaskOrCreateDab(KisFixedPaintDeviceSP dst, KisBrush::ColoringInformation* coloringInformation,
            double scaleX, double scaleY, double angle,
            const KisPaintInformation& info,
            double subPixelX = 0, double subPixelY = 0, qreal softnessFactor = DEFAULT_SOFTNESS_FACTOR) const;

    KisFixedPaintDeviceSP paintDevice(const KoColorSpace * colorSpace, double scale, double angle, const KisPaintInformation& info, double subPixelX, double subPixelY) const;

    virtual bool load() {
        return false;
    }

    void setText(const QString& txt);

    QFont font();
    void setFont(const QFont& font);

    void setPipeMode(bool pipe);
    bool pipeMode() const;

    void updateBrush();
    void toXML(QDomDocument& , QDomElement&) const;

    quint32 brushIndex(const KisPaintInformation& info) const;
    qint32 maskWidth(double scale, double angle, const KisPaintInformation& info) const;
    qint32 maskHeight(double scale, double angle, const KisPaintInformation& info) const;
    void setAngle(qreal _angle);
    void setScale(qreal _scale);
    void setSpacing(double _spacing);

    KisBrush* clone() const;

private:
    QFont m_font;
    QString m_text;

private:
    KisTextBrushesPipe *m_brushesPipe;
};

#endif
