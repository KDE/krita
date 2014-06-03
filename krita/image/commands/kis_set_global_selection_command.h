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
#ifndef KIS_SET_GLOBAL_SELECTION_COMMAND_H
#define KIS_SET_GLOBAL_SELECTION_COMMAND_H

#include <krita_export.h>
#include <kundo2command.h>
#include "kis_types.h"

/**
 * This command sets the global selection of the image. No saving
 * of the previous selection for "Reselect" action happens
 */
class KRITAIMAGE_EXPORT KisSetGlobalSelectionCommand : public KUndo2Command
{

public:
    /**
     * Constructor
     * @param image the image to set the global selection on
     * @param selection the selection that will be set a global selection,
     *        null selection will remove the selection
     */
    KisSetGlobalSelectionCommand(KisImageWSP image, KisSelectionSP selection);

    virtual void redo();
    virtual void undo();

private:
    KisImageWSP m_image;
    KisSelectionSP m_newSelection;
    KisSelectionSP m_oldSelection;
};

/**
 * Sets initial selection for the image. Nothing is selected,
 * but the defaultBounds are set properly
 */
class KRITAIMAGE_EXPORT KisSetEmptyGlobalSelectionCommand : public KisSetGlobalSelectionCommand
{
public:
    KisSetEmptyGlobalSelectionCommand(KisImageWSP image);
};

#endif //KIS_SET_GLOBAL_SELECTION_COMMAND_H
