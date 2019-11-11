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

#include "kis_scaling_size_brush.h"
#include "kritabrush_export.h"

class KisTextBrushesPipe;


class BRUSH_EXPORT KisTextBrush : public KisScalingSizeBrush
{

public:
    KisTextBrush();
    KisTextBrush(const KisTextBrush &rhs);
    ~KisTextBrush() override;

    KisTextBrush &operator=(const KisTextBrush &rhs);

    KoResourceSP clone() const override;


    void notifyStrokeStarted() override;
    void notifyCachedDabPainted(const KisPaintInformation& info) override;
    void prepareForSeqNo(const KisPaintInformation& info, int seqNo) override;

    void generateMaskAndApplyMaskOrCreateDab(KisFixedPaintDeviceSP dst, KisBrush::ColoringInformation* coloringInformation,
            KisDabShape const&,
            const KisPaintInformation& info,
            double subPixelX = 0, double subPixelY = 0, qreal softnessFactor = DEFAULT_SOFTNESS_FACTOR) const override;

    KisFixedPaintDeviceSP paintDevice(const KoColorSpace * colorSpace,
        KisDabShape const&, const KisPaintInformation& info, double subPixelX, double subPixelY) const override;

    bool load() override {
        return false;
    }

    bool loadFromDevice(QIODevice *) override {
        return false;
    }

    bool save() override {
        return false;
    }

    bool saveToDevice(QIODevice* ) const override {
        return false;
    }

    void setText(const QString& txt);
    QString text(void) const;

    QFont font();
    void setFont(const QFont& font);

    void setPipeMode(bool pipe);
    bool pipeMode() const;

    void updateBrush();
    void toXML(QDomDocument& , QDomElement&) const override;

    quint32 brushIndex(const KisPaintInformation& info) const override;
    qint32 maskWidth(KisDabShape const&, double subPixelX, double subPixelY, const KisPaintInformation& info) const override;
    qint32 maskHeight(KisDabShape const&, double subPixelX, double subPixelY, const KisPaintInformation& info) const override;
    void setAngle(qreal _angle) override;
    void setScale(qreal _scale) override;
    void setSpacing(double _spacing) override;

private:
    QFont m_font;
    QString m_text;

private:
    KisTextBrushesPipe *m_brushesPipe;
};

typedef QSharedPointer<KisTextBrush> KisTextBrushSP;

#endif
