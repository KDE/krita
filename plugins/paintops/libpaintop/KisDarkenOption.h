/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISDARKENOPTION_H
#define KISDARKENOPTION_H

#include <KisCurveOption.h>

struct KisDarkenOptionData;
class KisPainter;
class KisColorSource;


class PAINTOP_EXPORT KisDarkenOption : public KisCurveOption
{
public:
    KisDarkenOption(const KisPropertiesConfiguration *setting);

    KoColor apply(KisPainter * painter, const KisPaintInformation& info) const;
    void apply(KisColorSource* colorSource, const KisPaintInformation& info) const;
};

#endif // KISDARKENOPTION_H
