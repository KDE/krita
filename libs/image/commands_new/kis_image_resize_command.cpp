/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
