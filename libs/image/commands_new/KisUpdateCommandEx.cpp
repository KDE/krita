/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisUpdateCommandEx.h"

#include "kis_image_interfaces.h"
#include "kis_node.h"

KisUpdateCommandEx::KisUpdateCommandEx(KisBatchNodeUpdateSP updateData,
                                       KisUpdatesFacade *updatesFacade,
                                       State state,
                                       QWeakPointer<boost::none_t> blockUpdatesCookie)
    : FlipFlopCommand(state),
      m_updateData(updateData),
      m_blockUpdatesCookie(blockUpdatesCookie),
      m_updatesFacade(updatesFacade)
{
}

KisUpdateCommandEx::~KisUpdateCommandEx()
{
}

KisUpdateCommandEx::KisUpdateCommandEx(KisBatchNodeUpdateSP updateData, KisUpdatesFacade *updatesFacade, State state)
    : KisUpdateCommandEx(updateData, updatesFacade, state, QWeakPointer<boost::none_t>())
{
}

void KisUpdateCommandEx::partB() {
    if (m_blockUpdatesCookie) return;

    for (auto it = m_updateData->begin(); it != m_updateData->end(); ++it) {
        m_updatesFacade->refreshGraphAsync(it->first, it->second);
    }
}
