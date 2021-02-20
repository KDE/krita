/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISHOLDUIUPDATESCOMMAND_H
#define KISHOLDUIUPDATESCOMMAND_H

#include "kis_stroke_strategy_undo_command_based.h"
#include "kis_node.h"
#include "kis_command_utils.h"
#include <QSharedPointer>

class KisUpdatesFacade;


class KRITAIMAGE_EXPORT KisHoldUIUpdatesCommand : public KisCommandUtils::FlipFlopCommand, public KisStrokeStrategyUndoCommandBased::MutatedCommandInterface
{
public:
    KisHoldUIUpdatesCommand(KisUpdatesFacade *updatesFacade, State state);

    void partA() override;
    void partB() override;

private:
    KisUpdatesFacade *m_updatesFacade;
    QSharedPointer<bool> m_batchUpdateStarted;
};

#endif // KISHOLDUIUPDATESCOMMAND_H
