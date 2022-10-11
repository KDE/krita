/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSHARPNESSOPTION_H
#define KISSHARPNESSOPTION_H

#include <KisCurveOption2.h>

class KisSharpnessOptionData;

class PAINTOP_EXPORT KisSharpnessOption : public KisCurveOption2
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

private:
    KisSharpnessOptionData initializeFromData(const KisPropertiesConfiguration *setting);

private:
    bool m_alignOutlinePixels;
    int m_softness;
};

#endif // KISSHARPNESSOPTION_H
