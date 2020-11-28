/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISRESELECTACTIVESELECTIONCOMMAND_H
#define KISRESELECTACTIVESELECTIONCOMMAND_H

#include "kis_reselect_global_selection_command.h"


class KRITAIMAGE_EXPORT KisReselectActiveSelectionCommand : public KisReselectGlobalSelectionCommand
{
public:
    KisReselectActiveSelectionCommand(KisNodeSP activeNode, KisImageWSP image, KUndo2Command * parent = 0);

    void redo() override;
    void undo() override;

private:
    KisNodeSP m_activeNode;
    KisSelectionMaskSP m_reselectedMask;
};

#endif // KISRESELECTACTIVESELECTIONCOMMAND_H
