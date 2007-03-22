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

#ifndef KIS_IMAGE_COMMANDS_H_
#define KIS_IMAGE_COMMANDS_H_

#include <krita_export.h>

#include <QUndoCommand>
#include <QSize>
#include <QBitArray>
#include "kis_types.h"

class KoCompositeOp;

/// the base command for commands altering a KisImage
class KisImageCommand : public QUndoCommand {
    typedef QUndoCommand super;

public:
    /**
     * Constructor
     * @param name The name that will be shown in the ui
     * @param image The image the command will be working on.
     */
    KisImageCommand(const QString& name, KisImageSP image);
    virtual ~KisImageCommand() {}

protected:
    /**
     * Lock/Unlocks the undoAdapter for undo operations.
     * Set this at start and end of commands, which use recursion.
     * @param undo State the undoAdapter will be set to.
     */
    void setUndo(bool undo);

    KisImageSP m_image;
};


/**
  * The command for image locking inside macro commands.
  *
  * It will ensurce that the image is properly locked during the execution
  * of macro commands. Place it at the start and end of the macro command.
  */
class KisImageLockCommand : public KisImageCommand {
    typedef KisImageCommand super;

public:
    /**
     * Constructor
     * @param image The image the command will be working on.
     * @param lockImage Locking state of the image, while redo.
     */
    KisImageLockCommand(KisImageSP image, bool lockImage);
    virtual ~KisImageLockCommand() {}

    virtual void redo();
    virtual void undo();

private:
    bool m_lockImage;
};


class KisImageResizeCommand : public KisImageCommand {
    typedef KisImageCommand super;

public:
    KisImageResizeCommand(KisImageSP image, qint32 width, qint32 height, qint32 oldWidth, qint32 oldHeight);
    virtual ~KisImageResizeCommand() {}

    virtual void redo();
    virtual void undo();

private:
    QSize m_before;
    QSize m_after;
};


class KisImageConvertTypeCommand : public KisImageCommand {
    typedef KisImageCommand super;

public:
    KisImageConvertTypeCommand(KisImageSP image, KoColorSpace * beforeColorSpace, KoColorSpace * afterColorSpace);
    virtual ~KisImageConvertTypeCommand() {}

    virtual void redo();
    virtual void undo();

private:
    KoColorSpace * m_beforeColorSpace;
    KoColorSpace * m_afterColorSpace;
};


/// The command for image property changes
class KRITAIMAGE_EXPORT KisImagePropsCommand : public KisImageCommand {
    typedef KisImageCommand super;

public:
    /**
     * Command for image property changes
     *
     * This command stores the current image properties and set the new propertys.
     *
     * @param image the image
     * @param newName the new image name
     * @param newDescription the new image description
     * @param newColorSpace the new image color space
     * @param newProfile the new image profile
     * @param newResolution the new image resolution which will be used for xRes and yRes
     */
    KisImagePropsCommand(KisImageSP image, const QString& newName, const QString& newDescription,
                      KoColorSpace* newColorSpace, KoColorProfile* newProfile, double newResolution);
    virtual ~KisImagePropsCommand() {}

    virtual void redo();
    virtual void undo();

private:
    QString m_newName;
    QString m_oldName;
    QString m_newDescription;
    QString m_oldDescription;
    KoColorSpace * m_newColorSpace;
    KoColorSpace * m_oldColorSpace;
    KoColorProfile* m_newProfile;
    KoColorProfile* m_oldProfile;
    double m_newResolution;
    double m_oldXRes;
    double m_oldYRes;
};


class KisImageChangeLayersCommand : public KisImageCommand {
    typedef KisImageCommand super;

public:
    KisImageChangeLayersCommand(KisImageSP image, KisGroupLayerSP oldRootLayer, KisGroupLayerSP newRootLayer, const QString& name);
    virtual ~KisImageChangeLayersCommand() {}

    virtual void redo();
    virtual void undo();

private:
    KisGroupLayerSP m_oldRootLayer;
    KisGroupLayerSP m_newRootLayer;
};

/// The command for adding a layer
class KisImageLayerAddCommand : public KisImageCommand {
    typedef KisImageCommand super;

public:
    /**
     * Constructor
     * @param image The image the command will be working on.
     * @param layer the layer to add
     */
    KisImageLayerAddCommand(KisImageSP image, KisLayerSP layer);
    virtual ~KisImageLayerAddCommand() {}

    virtual void redo();
    virtual void undo();

private:
    KisLayerSP m_layer;
    KisGroupLayerSP m_parent;
    KisLayerSP m_aboveThis;
};


/// The command for removing a layer
class KisImageLayerRemoveCommand : public KisImageCommand {
    typedef KisImageCommand super;

public:
    /**
     * Constructor
     * @param image The image the command will be working on.
     * @param layer the layer to remove
     * @param wasParent the parent of the layer
     * @param wasAbove the layer above the layer
     */
    KisImageLayerRemoveCommand(KisImageSP image, KisLayerSP layer, KisGroupLayerSP wasParent, KisLayerSP wasAbove);
    virtual ~KisImageLayerRemoveCommand() {}

    virtual void redo();
    virtual void undo();

private:
    KisLayerSP m_layer;
    KisGroupLayerSP m_prevParent;
    KisLayerSP m_prevAbove;
};

/// The command for layer moves inside the layer stack
class KisImageLayerMoveCommand: public KisImageCommand {
    typedef KisImageCommand super;

public:
    /**
     * Command for layer moves inside the layer stack
     *
     * @param image the image
     * @param layer the moved layer
     * @param wasParent the previous parent of the layer
     * @param wasAbove the layer that was above the layer before the move
     */
    KisImageLayerMoveCommand(KisImageSP image, KisLayerSP layer, KisGroupLayerSP wasParent, KisLayerSP wasAbove);
    virtual ~KisImageLayerMoveCommand() {}

    virtual void redo();
    virtual void undo();

private:
    KisLayerSP m_layer;
    KisGroupLayerSP m_prevParent;
    KisLayerSP m_prevAbove;
    KisGroupLayerSP m_newParent;
    KisLayerSP m_newAbove;
};

/// The command for image property changes
class KRITAIMAGE_EXPORT KisImageLayerPropsCommand : public KisImageCommand {
    typedef KisImageCommand super;

public:
    /**
     * Command for image property changes
     *
     * This command stores the current image properties and set the new propertys.
     *
     * @param image the image
     * @param layer the layer whose propertys will be changed
     * @param opacity the new layer opacity
     * @param compositeOp the new layer composite op
     * @param name the new layer name
     */
    KisImageLayerPropsCommand(KisImageSP image, KisLayerSP layer, qint32 opacity, const KoCompositeOp* compositeOp, const QString& name, QBitArray channelFlags);
    virtual ~KisImageLayerPropsCommand() {}

    virtual void redo();
    virtual void undo();

private:
    KisLayerSP m_layer;
    QString m_name;
    qint32 m_opacity;
    const KoCompositeOp * m_compositeOp;
    QBitArray m_channelFlags;
};

#endif // KIS_IMAGE_COMMANDS_H_
