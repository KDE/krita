/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
