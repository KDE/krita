/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_image_commands.h"
#include "kis_image.h"

#include <klocalizedstring.h>


KisImageLockCommand::KisImageLockCommand(KisImageWSP image, bool lockImage)
    : KisImageCommand(kundo2_noi18n("lock image"), image),
      m_lockImage(lockImage)
{
}

void KisImageLockCommand::redo()
{
    KisImageSP image = m_image.toStrongRef();
    if (!image) {
        return;
    }

    if (!m_lockImage) {
        image->refreshGraph();
    }
}

void KisImageLockCommand::undo()
{
    KisImageSP image = m_image.toStrongRef();
    if (!image) {
        return;
    }

    if (m_lockImage) {
        image->refreshGraph();
    }
}

