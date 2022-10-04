/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISLIGHTNESSSTRENGTHOPTIONDATA_H
#define KISLIGHTNESSSTRENGTHOPTIONDATA_H

#include <KisCurveOptionData.h>

class PAINTOP_EXPORT KisLightnessStrengthOptionData : public KisCurveOptionData
{
public:
    KisLightnessStrengthOptionData(const QString &prefix = QString());

    // TODO: bake stuff!
};

#endif // KISLIGHTNESSSTRENGTHOPTIONDATA_H
