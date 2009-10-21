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

#include "kis_layer_props_command.h"

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

KisLayerPropsCommand::KisLayerPropsCommand(KisLayerSP layer,
        qint32 oldOpacity, qint32 newOpactiy,
        const QString& oldCompositeOp, const QString& newCompositeOp,
        const QString& oldName, const QString& newName,
        const QBitArray oldChannelFlags, const QBitArray newChannelFlags)
        : KisLayerCommand(i18n("Property Changes"), layer)
        , m_oldName(oldName)
        , m_newName(newName)
        , m_oldOpacity(oldOpacity)
        , m_newOpacity(newOpactiy)
        , m_oldCompositeOp(oldCompositeOp)
        , m_newCompositeOp(newCompositeOp)
        , m_oldChannelFlags(oldChannelFlags)
        , m_newChannelFlags(newChannelFlags)
{
}

KisLayerPropsCommand::~KisLayerPropsCommand()
{
}

void KisLayerPropsCommand::redo()
{
    m_layer->setOpacity(m_newOpacity);
    m_layer->setCompositeOp(m_newCompositeOp);
    m_layer->setName(m_newName);
    m_layer->setChannelFlags(m_newChannelFlags);
    m_layer->setDirty();
}

void KisLayerPropsCommand::undo()
{
    m_layer->setOpacity(m_oldOpacity);
    m_layer->setCompositeOp(m_oldCompositeOp);
    m_layer->setName(m_oldName);
    m_layer->setChannelFlags(m_oldChannelFlags);
    m_layer->setDirty();
}
