/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_image_resize_command.h"

#include <klocalizedstring.h>
#include "kis_image.h"


KisImageResizeCommand::KisImageResizeCommand(KisImageWSP image,
                                             const QSize& newSize, KUndo2Command *parent)
    : KUndo2Command(kundo2_i18n("Resize Image"), parent),
      m_image(image)
{
    // do we really need a translatable name for the command?
    KisImageSP imageSP = m_image.toStrongRef();
    if (!imageSP) {
        return;
    }
    m_sizeBefore = imageSP->size();
    m_sizeAfter = newSize;
}

void KisImageResizeCommand::redo()
{
    KisImageSP image = m_image.toStrongRef();
    if (!image) {
        return;
    }
    image->setSize(m_sizeAfter);
}

void KisImageResizeCommand::undo()
{
    KisImageSP image = m_image.toStrongRef();
    if (!image) {
        return;
    }
    image->setSize(m_sizeBefore);
}
