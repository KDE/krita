/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisChangeDeselectedMaskCommand.h"
#include "KisImageGlobalSelectionManagementInterface.h"
#include "kis_selection_mask.h"
#include "kis_image.h"

KisChangeDeselectedMaskCommand::KisChangeDeselectedMaskCommand(KisImageWSP image)
    : m_image(image)
{}

KisChangeDeselectedMaskCommand::KisChangeDeselectedMaskCommand(KisImageWSP image, KisSelectionMaskSP newDeselectedMask)
    : m_image(image)
    , m_newDeselectedMask(newDeselectedMask)
{}

void KisChangeDeselectedMaskCommand::undo() {
    KisImageSP image = m_image.toStrongRef();
    KIS_SAFE_ASSERT_RECOVER_RETURN(image);

    KisImageGlobalSelectionManagementInterface *iface = image->globalSelectionManagementInterface();

    iface->setDeselectedGlobalSelection(m_oldDeselectedMask);
    m_oldDeselectedMask.clear();
}

void KisChangeDeselectedMaskCommand::redo() {
    KisImageSP image = m_image.toStrongRef();
    KIS_SAFE_ASSERT_RECOVER_RETURN(image);

    KisImageGlobalSelectionManagementInterface *iface = image->globalSelectionManagementInterface();

    m_oldDeselectedMask = iface->deselectedGlobalSelection();
    iface->setDeselectedGlobalSelection(m_newDeselectedMask);
}
