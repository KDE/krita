/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2014 Timothée Giet <animtim@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "kis_pressure_flow_option.h"
#include <klocalizedstring.h>
#include <kis_painter.h>
#include <KoColor.h>

KisPressureFlowOption::KisPressureFlowOption()
    : KisCurveOption("Flow", KisPaintOpOption::GENERAL, true)
{
    m_checkable = false;

}


double KisPressureFlowOption::apply(const KisPaintInformation & info) const
{
    if (!isChecked()) return 1.0;
    return computeSizeLikeValue(info);
}
