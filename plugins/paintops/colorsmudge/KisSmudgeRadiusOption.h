/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSMUDGERADIUSOPTION_H
#define KISSMUDGERADIUSOPTION_H

#include <KisCurveOption2.h>

class KisPropertiesConfiguration;
class KisSmudgeRadiusOptionData;

class KisSmudgeRadiusOption2 : public KisCurveOption2
{
public:
    KisSmudgeRadiusOption2(const KisPropertiesConfiguration *setting);

private:
    static KisSmudgeRadiusOptionData initializeData(const KisPropertiesConfiguration *setting);
};

#endif // KISSMUDGERADIUSOPTION_H
