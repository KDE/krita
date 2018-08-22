/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
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
