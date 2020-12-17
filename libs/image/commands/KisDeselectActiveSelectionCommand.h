/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISDESELECTACTIVESELECTIONCOMMAND_H
#define KISDESELECTACTIVESELECTIONCOMMAND_H

#include "kis_deselect_global_selection_command.h"

class KRITAIMAGE_EXPORT KisDeselectActiveSelectionCommand : public KisDeselectGlobalSelectionCommand
{
public:
    KisDeselectActiveSelectionCommand(KisSelectionSP activeSelection, KisImageWSP image, KUndo2Command * parent = 0);
    ~KisDeselectActiveSelectionCommand() override;

    void redo() override;
    void undo() override;

private:
    KisSelectionSP m_activeSelection;
    KisSelectionMaskSP m_deselectedMask;
};

#endif // KISDESELECTACTIVESELECTIONCOMMAND_H
