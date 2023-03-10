/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSHARPNESSOPTION_H
#define KISSHARPNESSOPTION_H

#include <KisCurveOption.h>

struct KisSharpnessOptionData;

class PAINTOP_EXPORT KisSharpnessOption : public KisCurveOption
{
public:
    KisSharpnessOption(const KisPropertiesConfiguration *setting);

    /**
    * First part of the sharpness is the coordinates: in pen mode they are integers without fractions
    */
    void apply(const KisPaintInformation &info, const QPointF &pt, qint32 &x, qint32 &y, qreal &xFraction, qreal &yFraction) const;

    /**
    * Apply threshold specified by user
    */
    void applyThreshold(KisFixedPaintDeviceSP dab, const KisPaintInformation &info);

    bool alignOutlineToPixels() const;

private:
    KisSharpnessOption(const KisSharpnessOptionData &data);
private:
    bool m_alignOutlinePixels;
    int m_softness;
};

#endif // KISSHARPNESSOPTION_H
