/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSMUDGELENGTHOPTION_H
#define KISSMUDGELENGTHOPTION_H

#include <KisCurveOption.h>
#include <KisSmudgeLengthOptionData.h>

class KisSmudgeLengthOption : public KisCurveOption
{
public:
    KisSmudgeLengthOption(const KisPropertiesConfiguration *setting);

    bool useNewEngine() const;
    bool smearAlpha() const;
    KisSmudgeLengthOptionData::Mode mode() const;

private:
    KisSmudgeLengthOption(const KisSmudgeLengthOptionData &data);

private:
    bool m_useNewEngine {false};
    bool m_smearAlpha {true};
    KisSmudgeLengthOptionData::Mode m_mode {KisSmudgeLengthOptionData::SMEARING_MODE};
};

#endif // KISSMUDGELENGTHOPTION_H
