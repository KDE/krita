/*
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_selection_commands.h"

#include <klocalizedstring.h>

#include "kis_image.h"
#include "kis_selection.h"
#include "kis_undo_adapter.h"
#include "kis_selection_mask.h"
#include "kis_pixel_selection.h"

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

void KisSetGlobalSelectionCommand::redo()
{
    KisImageSP image = m_image.toStrongRef();
    if (!image) {
        return;
    }
    image->setGlobalSelection(m_newSelection);
}

void KisSetGlobalSelectionCommand::undo()
{
    KisImageSP image = m_image.toStrongRef();
    if (!image) {
        return;
    }
    image->setGlobalSelection(m_oldSelection);
}

KisSetEmptyGlobalSelectionCommand::KisSetEmptyGlobalSelectionCommand(KisImageWSP image)
    : KisSetGlobalSelectionCommand(image, KisSelectionSP(new KisSelection(KisSelectionEmptyBoundsSP(new KisSelectionEmptyBounds(image)))))
{
}
