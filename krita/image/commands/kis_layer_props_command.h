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

#ifndef KIS_LAYER_PROPS_COMMAND_H
#define KIS_LAYER_PROPS_COMMAND_H

#include <krita_export.h>

#include <QUndoCommand>
#include <QSize>
#include <QBitArray>
#include "kis_types.h"
#include "kis_layer_command.h"

class KoCompositeOp;
class KoColorSpace;
class KoColorProfile;

/// The command for layer property changes
class KRITAIMAGE_EXPORT KisLayerPropsCommand : public KisLayerCommand
{

public:
    /**
     * Command for layer property changes
     *
     * This command stores the current layer properties and set the new properties
     *
     * @param image the image
     * @param layer the layer whose propertys will be changed
     * @param opacity the new layer opacity
     * @param compositeOp the new layer composite op
     * @param name the new layer name
     */
    KisLayerPropsCommand(KisLayerSP layer,
                         qint32 oldOpacity, qint32 newOpactiy,
                         const QString&  oldCompositeOp, const QString& newCompositeOp,
                         const QString& oldName, const QString& newName,
                         const QBitArray oldChannelFlags, const QBitArray newChannelFlags);

    virtual ~KisLayerPropsCommand();
    virtual void redo();
    virtual void undo();

private:
    QString m_oldName;
    QString m_newName;

    qint32 m_oldOpacity;
    qint32 m_newOpacity;

    QString m_oldCompositeOp;
    QString m_newCompositeOp;

    QBitArray m_oldChannelFlags;
    QBitArray m_newChannelFlags;
};

#endif
