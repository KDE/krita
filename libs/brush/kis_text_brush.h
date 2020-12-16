/*
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_TEXT_BRUSH_H_
#define _KIS_TEXT_BRUSH_H_

#include <QFont>

#include <KoEphemeralResource.h>

#include "kis_scaling_size_brush.h"
#include "kritabrush_export.h"

class KisTextBrushesPipe;


class BRUSH_EXPORT KisTextBrush : public KoEphemeralResource<KisScalingSizeBrush>
{

public:
    KisTextBrush();
    KisTextBrush(const KisTextBrush &rhs);
    ~KisTextBrush() override;

    KisTextBrush &operator=(const KisTextBrush &rhs) = delete;

    KoResourceSP clone() const override;


    void notifyStrokeStarted() override;
    void prepareForSeqNo(const KisPaintInformation& info, int seqNo) override;

    void generateMaskAndApplyMaskOrCreateDab(KisFixedPaintDeviceSP dst, KisBrush::ColoringInformation* coloringInformation,
            KisDabShape const&,
            const KisPaintInformation& info,
            double subPixelX = 0, double subPixelY = 0, qreal softnessFactor = DEFAULT_SOFTNESS_FACTOR, qreal lightnessStrength = 1.0) const override;

    KisFixedPaintDeviceSP paintDevice(const KoColorSpace * colorSpace,
        KisDabShape const&, const KisPaintInformation& info, double subPixelX, double subPixelY) const override;

    void setText(const QString& txt);
    QString text(void) const;

    QFont font();
    void setFont(const QFont& font);

    void setPipeMode(bool pipe);
    bool pipeMode() const;

    void updateBrush();
    void toXML(QDomDocument& , QDomElement&) const override;

    quint32 brushIndex() const override;
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
