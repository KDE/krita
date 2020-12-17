/*
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_SET_GLOBAL_SELECTION_COMMAND_H
#define KIS_SET_GLOBAL_SELECTION_COMMAND_H

#include <kritaimage_export.h>
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

    void redo() override;
    void undo() override;

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
