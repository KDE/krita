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

KisDeselectGlobalSelectionCommand::KisDeselectGlobalSelectionCommand(KisImageWSP image, KUndo2Command * parent) :
    KUndo2Command(kundo2_i18n("Deselect"), parent)
  , m_image(image)
{
}

KisDeselectGlobalSelectionCommand::~KisDeselectGlobalSelectionCommand()
{
}

void KisDeselectGlobalSelectionCommand::redo()
{
    KisImageSP image = m_image.toStrongRef();
    if (image) {
        m_oldSelection = image->globalSelection();
        image->deselectGlobalSelection();
    }
}

void KisDeselectGlobalSelectionCommand::undo()
{
    KisImageSP image = m_image.toStrongRef();
    if (image) {
        image->setGlobalSelection(m_oldSelection);
    }
}
