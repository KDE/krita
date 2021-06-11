/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisDisableDirtyRequestsCommand.h"

#include "kis_image_interfaces.h"

KisDisableDirtyRequestsCommand::KisDisableDirtyRequestsCommand(KisUpdatesFacade *updatesFacade, State state)
    : KisCommandUtils::FlipFlopCommand(state),
      m_updatesFacade(updatesFacade)
{
}

void KisDisableDirtyRequestsCommand::partA()
{
    m_updatesFacade->disableDirtyRequests();
}

void KisDisableDirtyRequestsCommand::partB()
{
    m_updatesFacade->enableDirtyRequests();
}
