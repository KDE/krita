/*
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_reselect_global_selection_command.h"
#include <klocalizedstring.h>

#include "kis_image.h"
#include "kis_group_layer.h"
#include "kis_selection_mask.h"
#include "KisImageGlobalSelectionManagementInterface.h"
#include "KisChangeDeselectedMaskCommand.h"
#include "kis_image_layer_remove_command.h"
#include "kis_image_layer_add_command.h"
#include "KisNotifySelectionChangedCommand.h"


KisReselectGlobalSelectionCommand::KisReselectGlobalSelectionCommand(KisImageWSP image, KUndo2Command * parent)
    : KisCommandUtils::AggregateCommand(kundo2_i18n("Reselect"), parent)
    , m_image(image)
{
}

KisReselectGlobalSelectionCommand::~KisReselectGlobalSelectionCommand()
{
}

void KisReselectGlobalSelectionCommand::populateChildCommands()
{
    KisImageSP image = m_image.toStrongRef();
    KIS_SAFE_ASSERT_RECOVER_RETURN(image);

    addCommand(new KisNotifySelectionChangedCommand(image, KisNotifySelectionChangedCommand::INITIALIZING));

    KisSelectionMaskSP selectionMask = image->globalSelectionManagementInterface()->deselectedGlobalSelection();
    if (selectionMask) {
        KisSelectionMaskSP activeSelectionMask = image->rootLayer()->selectionMask();
        if (activeSelectionMask) {
            addCommand(new KisImageLayerRemoveCommand(image, activeSelectionMask, false, false));
        }

        addCommand(new KisChangeDeselectedMaskCommand(image, nullptr));
        addCommand(new KisImageLayerAddCommand(image, selectionMask, image->root(), image->root()->lastChild(), false, false));
    }

    addCommand(new KisNotifySelectionChangedCommand(image, KisNotifySelectionChangedCommand::FINALIZING));
}

