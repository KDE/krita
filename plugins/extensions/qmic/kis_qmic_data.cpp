/*
 *  SPDX-FileCopyrightText: 2015 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_qmic_data.h"
#include <kis_debug.h>

const float KisQmicData::INVALID_PROGRESS_VALUE = -2.0f;

KisQmicData::KisQmicData()
    : m_progress(INVALID_PROGRESS_VALUE)
    , m_cancel(false)
{
}

KisQmicData::~KisQmicData()
{
}

void KisQmicData::reset()
{
    m_progress = INVALID_PROGRESS_VALUE;
    m_cancel = false;
}
