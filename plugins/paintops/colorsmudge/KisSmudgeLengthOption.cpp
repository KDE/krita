/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSmudgeLengthOption.h"

#include <kis_painter.h>

#include <KisPaintOpOptionUtils.h>
namespace kpou = KisPaintOpOptionUtils;

KisSmudgeLengthOption::KisSmudgeLengthOption(const KisPropertiesConfiguration *setting)
    : KisSmudgeLengthOption(kpou::loadOptionData<KisSmudgeLengthOptionData>(setting))
{
}

KisSmudgeLengthOption::KisSmudgeLengthOption(const KisSmudgeLengthOptionData &data)
    : KisCurveOption(data)
    , m_useNewEngine(data.useNewEngine)
    , m_smearAlpha(data.smearAlpha)
    , m_mode(data.mode)
{
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
