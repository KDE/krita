/*
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_DESELECT_GLOBAL_SELECTION_COMMAND_H_
#define KIS_DESELECT_GLOBAL_SELECTION_COMMAND_H_

#include <kritaimage_export.h>
#include <kis_command_utils.h>
#include "kis_types.h"


/// The command for deselection the global selection of KisImage
class KRITAIMAGE_EXPORT KisDeselectGlobalSelectionCommand : public KisCommandUtils::AggregateCommand
{
public:
    /**
     * Constructor
     * @param image the image
     * @param parent the parent command
     */
    KisDeselectGlobalSelectionCommand(KisImageWSP image, KUndo2Command * parent = 0);
    ~KisDeselectGlobalSelectionCommand() override;

    void populateChildCommands() override;

protected:
    KisImageWSP m_image;
};

#endif
