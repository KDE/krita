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
#include <QString>
#include <QBitArray>

#include <klocale.h>

#include "KoColorSpaceRegistry.h"
#include "KoColor.h"
#include "KoColorProfile.h"
#include "KoColorProfile.h"

#include "kis_image_commands.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_undo_adapter.h"

KisImageCommand::KisImageCommand(const QString& name, KisImageSP image) :
    super(name), m_image(image)
{
}

void KisImageCommand::setUndo(bool undo)
{
    if (m_image->undoAdapter()) {
        m_image->undoAdapter()->setUndo(undo);
    }
}



KisImageLockCommand::KisImageLockCommand(KisImageSP image, bool lockImage)
    : super("lock image", image)  // Not for translation, this is only ever used inside a macro command.
{
    m_lockImage = lockImage;
}

void KisImageLockCommand::redo()
{
    setUndo(false);
    if (m_lockImage) {
        m_image->lock();
    } else {
        m_image->unlock();
    }
    setUndo(true);
}

void KisImageLockCommand::undo()
{
    setUndo(false);
    if (m_lockImage) {
        m_image->unlock();
    } else {
        m_image->lock();
    }
    setUndo(true);
}



KisImageResizeCommand::KisImageResizeCommand(KisImageSP image, qint32 width, qint32 height, qint32 oldWidth, qint32 oldHeight)
    : super(i18n("Resize Image"), image)
{
    m_before = QSize(oldWidth, oldHeight);
    m_after = QSize(width, height);
}

void KisImageResizeCommand::redo()
{
    setUndo(false);
    m_image->resize(m_after.width(), m_after.height());
    setUndo(true);
}

void KisImageResizeCommand::undo()
{
    setUndo(false);
    m_image->resize(m_before.width(), m_before.height());
    setUndo(true);
}



KisImageConvertTypeCommand::KisImageConvertTypeCommand(KisImageSP image, KoColorSpace * beforeColorSpace, KoColorSpace * afterColorSpace)
    : super(i18n("Convert Image Type"), image)
{
    m_beforeColorSpace = beforeColorSpace;
    m_afterColorSpace = afterColorSpace;
}

void KisImageConvertTypeCommand::redo()
{
    setUndo(false);
    m_image->setColorSpace(m_afterColorSpace);
    m_image->setProfile(m_afterColorSpace->profile());
    setUndo(true);
}

void KisImageConvertTypeCommand::undo()
{
    setUndo(false);
    m_image->setColorSpace(m_beforeColorSpace);
    m_image->setProfile(m_beforeColorSpace->profile());
    setUndo(true);
}



KisImagePropsCommand::KisImagePropsCommand(KisImageSP image, const QString& newName, const QString& newDescription,
                                           KoColorSpace* newColorSpace, KoColorProfile* newProfile, double newResolution)
    : super(i18n("Property Changes"), image)
    , m_newName(newName), m_newDescription(newDescription), m_newColorSpace(newColorSpace), m_newProfile(newProfile)
    , m_newResolution(newResolution)
{
    m_oldName = m_image->name();
    m_oldDescription = m_image->description();
    m_oldColorSpace = m_image->colorSpace();
    m_oldProfile = m_image->profile();
    m_oldXRes = m_image->xRes();
    m_oldYRes = m_image->yRes();
}

void KisImagePropsCommand::redo()
{
    setUndo(false);
    m_image->setName(m_newName);
    m_image->setColorSpace(m_newColorSpace);
    m_image->setResolution(m_newResolution, m_newResolution);
    m_image->setDescription(m_newDescription);
    m_image->setProfile(m_newProfile);
    setUndo(true);
}

void KisImagePropsCommand::undo()
{
    setUndo(false);
    m_image->setName(m_oldName);
    m_image->setColorSpace(m_oldColorSpace);
    m_image->setResolution(m_oldXRes, m_oldYRes);
    m_image->setDescription(m_oldDescription);
    m_image->setProfile(m_oldProfile);
    setUndo(true);
}



KisImageChangeLayersCommand::KisImageChangeLayersCommand(KisImageSP image, KisGroupLayerSP oldRootLayer, KisGroupLayerSP newRootLayer, const QString& name)
    : super(name, image)
{
    m_oldRootLayer = oldRootLayer;
    m_newRootLayer = newRootLayer;
}

void KisImageChangeLayersCommand::redo()
{
    setUndo(false);
    m_image->setRootLayer(m_newRootLayer);
    m_image->notifyLayersChanged();
    setUndo(true);
}

void KisImageChangeLayersCommand::undo()
{
    setUndo(false);
    m_image->setRootLayer(m_oldRootLayer);
    m_image->notifyLayersChanged();
    setUndo(true);
}



KisImageLayerAddCommand::KisImageLayerAddCommand(KisImageSP image, KisLayerSP layer)
    : super(i18n("Add Layer"), image)
{
    m_layer = layer;
    m_parent = layer->parent();
    m_aboveThis = layer->nextSibling();
}

void KisImageLayerAddCommand::redo()
{
    m_image->addLayer(m_layer, m_parent, m_aboveThis);
}

void KisImageLayerAddCommand::undo()
{
    m_image->removeLayer(m_layer);
}



KisImageLayerRemoveCommand::KisImageLayerRemoveCommand(KisImageSP image, KisLayerSP layer, KisGroupLayerSP wasParent, KisLayerSP wasAbove)
    : super(i18n("Remove Layer"), image)
{
    m_layer = layer;
    m_prevParent = wasParent;
    m_prevAbove = wasAbove;
}


void KisImageLayerRemoveCommand::redo()
{
    m_image->removeLayer(m_layer);
}

void KisImageLayerRemoveCommand::undo()
{
    m_image->addLayer(m_layer, m_prevParent, m_prevAbove);
}



KisImageLayerMoveCommand::KisImageLayerMoveCommand(KisImageSP image, KisLayerSP layer, KisGroupLayerSP wasParent, KisLayerSP wasAbove)
    : super(i18n("Move Layer"), image)
{
    m_layer = layer;
    m_prevParent = wasParent;
    m_prevAbove = wasAbove;
    m_newParent = layer->parent();
    m_newAbove = layer->nextSibling();
}

void KisImageLayerMoveCommand::redo()
{
    m_image->moveLayer(m_layer, m_newParent, m_newAbove);
}

void KisImageLayerMoveCommand::undo()
{
    m_image->moveLayer(m_layer, m_prevParent, m_prevAbove);
}




KisImageLayerPropsCommand::KisImageLayerPropsCommand(KisImageSP image, KisLayerSP layer, qint32 opacity, const KoCompositeOp* compositeOp, const QString& name, QBitArray channelFlags )
    : super(i18n("Property Changes"), image)
{
    m_layer = layer;
    m_name = name;
    m_opacity = opacity;
    m_compositeOp = compositeOp;
    m_channelFlags = channelFlags;
}

void KisImageLayerPropsCommand::redo()
{
    QString name = m_layer->name();
    qint32 opacity = m_layer->opacity();
    const KoCompositeOp* compositeOp = m_layer->compositeOp();
    QBitArray channelFlags = m_layer->channelFlags();

    m_image->setLayerProperties(m_layer, m_opacity, m_compositeOp, m_name, m_channelFlags);

    m_compositeOp = compositeOp;
    m_channelFlags = channelFlags;
    m_name = name;
    m_opacity = opacity;

    m_layer->setDirty();

}

void KisImageLayerPropsCommand::undo()
{
    redo();
}
