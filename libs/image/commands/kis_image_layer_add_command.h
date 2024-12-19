/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_IMAGE_LAYER_ADD_COMMAND_H_
#define KIS_IMAGE_LAYER_ADD_COMMAND_H_

#include <kritaimage_export.h>

#include "kis_types.h"
#include "kis_image_command.h"
#include "KisNodeAdditionFlags.h"


/// The command for adding a layer
class KRITAIMAGE_EXPORT KisImageLayerAddCommand : public KisImageCommand
{

public:
    enum Flag {
        None = 0x0,
        DoRedoUpdates = 0x1,
        DoUndoUpdates = 0x2,
        DontActivateOnAddition = 0x4,
    };

    Q_DECLARE_FLAGS(Flags, Flag)

public:
    /**
     * Constructor
     * @param image The image the command will be working on.
     * @param layer The layer to add
     * @param parent The parent node
     * @param aboveThis The node above this
     * @param doRedoUpdates Whether to make the redo updates
     * @param doUndoUpdates Whether to make the undo updates
     */
    KisImageLayerAddCommand(KisImageWSP image, KisNodeSP layer, KisNodeSP parent, KisNodeSP aboveThis, bool doRedoUpdates = true, bool doUndoUpdates = true);
    KisImageLayerAddCommand(KisImageWSP image, KisNodeSP layer, KisNodeSP parent, quint32 index, bool doRedoUpdates = true, bool doUndoUpdates = true);

    KisImageLayerAddCommand(KisImageWSP image, KisNodeSP layer, KisNodeSP parent, KisNodeSP aboveThis, Flags flags);
    KisImageLayerAddCommand(KisImageWSP image, KisNodeSP layer, KisNodeSP parent, quint32 index, Flags flags);

    void redo() override;
    void undo() override;

private:
    KisNodeSP m_layer;
    KisNodeSP m_parent;
    KisNodeSP m_aboveThis;
    quint32 m_index;
    bool m_doRedoUpdates;
    bool m_doUndoUpdates;
    KisNodeAdditionFlags m_additionFlags;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KisImageLayerAddCommand::Flags)

#endif
