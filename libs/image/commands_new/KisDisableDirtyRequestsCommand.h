/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISDISABLEDIRTYREQUESTSCOMMAND_H
#define KISDISABLEDIRTYREQUESTSCOMMAND_H

#include "kritaimage_export.h"
#include "kis_command_utils.h"

class KisUpdatesFacade;

class KRITAIMAGE_EXPORT KisDisableDirtyRequestsCommand : public KisCommandUtils::FlipFlopCommand
{
public:
    KisDisableDirtyRequestsCommand(KisUpdatesFacade *updatesFacade, State state);

    void partA() override;
    void partB() override;

private:
    KisUpdatesFacade *m_updatesFacade;
};

#endif // KISDISABLEDIRTYREQUESTSCOMMAND_H
