/*
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_RESELECT_GLOBAL_SELECTION_COMMAND_H
#define KIS_RESELECT_GLOBAL_SELECTION_COMMAND_H

#include <kritaimage_export.h>
#include <kis_command_utils.h>
#include "kis_types.h"

/// The command for deselection the global selection of KisImage
class KRITAIMAGE_EXPORT KisReselectGlobalSelectionCommand : public KisCommandUtils::AggregateCommand
{
public:
    /**
     * Constructor
     * @param image the image
     * @param parent the parent command
     */
    KisReselectGlobalSelectionCommand(KisImageWSP image, KUndo2Command * parent = nullptr);
    ~KisReselectGlobalSelectionCommand() override;

    void populateChildCommands() override;

protected:
    KisImageWSP m_image;
};


#endif
