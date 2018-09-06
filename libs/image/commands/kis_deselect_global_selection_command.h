/*
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
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
#ifndef KIS_DESELECT_GLOBAL_SELECTION_COMMAND_H_
#define KIS_DESELECT_GLOBAL_SELECTION_COMMAND_H_

#include <kritaimage_export.h>
#include <kundo2command.h>
#include "kis_types.h"


/// The command for deselection the global selection of KisImage
class KRITAIMAGE_EXPORT KisDeselectGlobalSelectionCommand : public KUndo2Command
{

public:
    /**
     * Constructor
     * @param image the image
     * @param parent the parent command
     */
    KisDeselectGlobalSelectionCommand(KisImageWSP image, KUndo2Command * parent = 0);
    ~KisDeselectGlobalSelectionCommand() override;

    void redo() override;
    void undo() override;

protected:
    KisImageWSP m_image;
    KisSelectionSP m_oldSelection;
};

#endif
