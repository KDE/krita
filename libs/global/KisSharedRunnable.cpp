/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisSharedRunnable.h"

#include "KisSharedThreadPoolAdapter.h"
#include "kis_assert.h"

void KisSharedRunnable::run()
{
    runShared();

    if (m_adapter) {
        m_adapter->notifyJobCompleted();
    }
}

void KisSharedRunnable::setSharedThreadPoolAdapter(KisSharedThreadPoolAdapter *adapter)
{
    m_adapter = adapter;
}

