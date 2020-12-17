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

KisReselectGlobalSelectionCommand::KisReselectGlobalSelectionCommand(KisImageWSP image, KUndo2Command * parent) :
        KUndo2Command(kundo2_i18n("Reselect"), parent)
        , m_image(image)
{
}

KisReselectGlobalSelectionCommand::~KisReselectGlobalSelectionCommand()
{
}

void KisReselectGlobalSelectionCommand::redo()
{
    KisImageSP image = m_image.toStrongRef();
    if (!image) {
        return;
    }
    m_canReselect = image->canReselectGlobalSelection();

    if (m_canReselect) {
        image->reselectGlobalSelection();
    }
}

void KisReselectGlobalSelectionCommand::undo()
{
    KisImageSP image = m_image.toStrongRef();
    if (!image) {
        return;
    }
    if (m_canReselect) {
        image->deselectGlobalSelection();
    }
}

