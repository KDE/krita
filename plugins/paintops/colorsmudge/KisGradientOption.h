/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISGRADIENTOPTION_H
#define KISGRADIENTOPTION_H

#include <KisCurveOption.h>

class KoAbstractGradient;
typedef QSharedPointer<KoAbstractGradient> KoAbstractGradientSP;

struct KisGradientOptionData;

class KisGradientOption : public KisCurveOption
{
public:
    KisGradientOption(const KisPropertiesConfiguration *setting);

    void apply(KoColor& color, const KoAbstractGradientSP gradient, const KisPaintInformation& info) const;
};

#endif // KISGRADIENTOPTION_H
