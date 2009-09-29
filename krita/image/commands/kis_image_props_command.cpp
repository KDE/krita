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

#include "kis_image_commands.h"
#include <QString>
#include <QBitArray>

#include <klocale.h>

#include "KoColorSpaceRegistry.h"
#include "KoColor.h"
#include "KoColorProfile.h"
#include "KoColorSpace.h"


#include "kis_image.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_undo_adapter.h"

KisImagePropsCommand::KisImagePropsCommand(KisImageWSP image, const KoColorSpace* newColorSpace)
        : KisImageCommand(i18n("Property Changes"), image)
        , m_newColorSpace(newColorSpace)
{
    m_oldColorSpace = m_image->colorSpace();
}

void KisImagePropsCommand::redo()
{
    setUndo(false);
    m_image->setColorSpace(m_newColorSpace);
    m_image->rootLayer()->setDirty();
    setUndo(true);
}

void KisImagePropsCommand::undo()
{
    setUndo(false);
    m_image->setColorSpace(m_oldColorSpace);
    m_image->rootLayer()->setDirty();
    setUndo(true);
}
