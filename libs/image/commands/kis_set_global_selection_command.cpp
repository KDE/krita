/*
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_selection_commands.h"

#include <klocalizedstring.h>

#include "kis_image.h"
#include "kis_default_bounds.h"
#include "KisImageResolutionProxy.h"
#include "kis_selection.h"
#include "kis_undo_adapter.h"
#include "kis_selection_mask.h"
#include "kis_pixel_selection.h"
#include "KisImageGlobalSelectionManagementInterface.h"
#include "kis_group_layer.h"

#include "kis_image_layer_remove_command.h"
#include "kis_image_layer_add_command.h"
#include "kis_selection_mask.h"
#include "kis_activate_selection_mask_command.h"
#include "KisChangeValueCommand.h"
#include "KisChangeDeselectedMaskCommand.h"
#include "KisNotifySelectionChangedCommand.h"


KisSetGlobalSelectionCommand::KisSetGlobalSelectionCommand(KisImageWSP image, KisSelectionSP selection)
    : m_image(image)
{
    KisImageSP imageSP = m_image.toStrongRef();
    if (!image) {
        return;
    }
    m_oldSelection = imageSP->globalSelection();
    m_newSelection = selection;
}

void KisSetGlobalSelectionCommand::populateChildCommands()
{
    KisImageSP image = m_image.toStrongRef();
    KIS_SAFE_ASSERT_RECOVER_RETURN(image);

    addCommand(new KisNotifySelectionChangedCommand(image, KisNotifySelectionChangedCommand::INITIALIZING));

    KisSelectionMaskSP selectionMask = image->rootLayer()->selectionMask();
    if (selectionMask) {
        addCommand(new KisImageLayerRemoveCommand(image, selectionMask, false, false));
    }

    if (m_newSelection) {
        selectionMask = new KisSelectionMask(image, i18n("Selection Mask"));
        selectionMask->initSelection(image->rootLayer());

        // If we do not set the selection now, the setActive call coming next
        // can be very, very expensive, depending on the size of the image.
        selectionMask->setSelection(m_newSelection);

        addCommand(new KisImageLayerAddCommand(image, selectionMask,
                                               image->root(), image->root()->lastChild(),
                                               false, false));
        addCommand(new KisActivateSelectionMaskCommand(selectionMask, true));

    }


    addCommand(new KisChangeDeselectedMaskCommand(image));
    addCommand(new KisNotifySelectionChangedCommand(image, KisNotifySelectionChangedCommand::FINALIZING));
}

KisSetEmptyGlobalSelectionCommand::KisSetEmptyGlobalSelectionCommand(KisImageWSP image)
    : KisSetGlobalSelectionCommand(image,
                                   new KisSelection(new KisSelectionEmptyBounds(image),
                                                    toQShared(new KisImageResolutionProxy(image))))
{
}
