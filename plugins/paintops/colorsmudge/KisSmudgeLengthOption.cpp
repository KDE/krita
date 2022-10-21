/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSmudgeLengthOption.h"

#include <kis_painter.h>


KisSmudgeLengthOption::KisSmudgeLengthOption(const KisPropertiesConfiguration *setting)
    : KisCurveOption2(initializeFromData(setting))
{
}

KisSmudgeLengthOptionData KisSmudgeLengthOption::initializeFromData(const KisPropertiesConfiguration *setting)
{
    KisSmudgeLengthOptionData data;
    data.read(setting);

    m_smearAlpha = data.smearAlpha;
    m_useNewEngine = data.useNewEngine;
    m_mode = data.mode;

    return data;
}

KisSmudgeLengthOptionData::Mode KisSmudgeLengthOption::mode() const
{
    return m_mode;
}

bool KisSmudgeLengthOption::smearAlpha() const
{
    return m_smearAlpha;
}

bool KisSmudgeLengthOption::useNewEngine() const
{
    return m_useNewEngine;
}
