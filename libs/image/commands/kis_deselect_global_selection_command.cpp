/*
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_deselect_global_selection_command.h"

#include <klocalizedstring.h>

#include "kis_image.h"
#include "kis_group_layer.h"
#include "kis_selection_mask.h"
#include "KisImageGlobalSelectionManagementInterface.h"
#include "KisChangeDeselectedMaskCommand.h"
#include "kis_image_layer_remove_command.h"
#include "KisNotifySelectionChangedCommand.h"


KisDeselectGlobalSelectionCommand::KisDeselectGlobalSelectionCommand(KisImageWSP image, KUndo2Command * parent)
    : KisCommandUtils::AggregateCommand(kundo2_i18n("Deselect"), parent)
    , m_image(image)
{
}

KisDeselectGlobalSelectionCommand::~KisDeselectGlobalSelectionCommand()
{
}

void KisDeselectGlobalSelectionCommand::populateChildCommands()
{
    KisImageSP image = m_image.toStrongRef();
    KIS_SAFE_ASSERT_RECOVER_RETURN(image);

    KisSelectionMaskSP selectionMask = image->rootLayer()->selectionMask();
    if (selectionMask) {
        addCommand(new KisNotifySelectionChangedCommand(image, KisNotifySelectionChangedCommand::INITIALIZING));
        addCommand(new KisChangeDeselectedMaskCommand(image, selectionMask));
        addCommand(new KisImageLayerRemoveCommand(image, selectionMask, false, false));
        addCommand(new KisNotifySelectionChangedCommand(image, KisNotifySelectionChangedCommand::FINALIZING));
    }
}
