/*
 *  Copyright (c) 2010 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_image_set_resolution_command.h"

#include <klocale.h>
#include <kis_image.h>


KisImageSetResolutionCommand::KisImageSetResolutionCommand(KisImageWSP image, qreal newXRes, qreal newYRes)
    : KUndo2Command(i18nc("(qtundo-format)", "Set Image Resolution"))
    , m_image(image)
    , m_newXRes(newXRes)
    , m_newYRes(newYRes)
    , m_oldXRes(m_image->xRes())
    , m_oldYRes(m_image->yRes())
{
}

void KisImageSetResolutionCommand::undo()
{
    m_image->setResolution(m_oldXRes, m_oldYRes);
}

void KisImageSetResolutionCommand::redo()
{
    m_image->setResolution(m_newXRes, m_newYRes);
}

