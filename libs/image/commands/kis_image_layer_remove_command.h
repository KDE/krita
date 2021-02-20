/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_IMAGE_LAYER_REMOVE_COMMAND_H
#define __KIS_IMAGE_LAYER_REMOVE_COMMAND_H

#include "kritaimage_export.h"
#include "kis_types.h"
#include "kis_image_command.h"


class KRITAIMAGE_EXPORT KisImageLayerRemoveCommand : public KisImageCommand
{
public:
    KisImageLayerRemoveCommand(KisImageWSP image,
                               KisNodeSP node,
                               bool doRedoUpdates = true,
                               bool doUndoUpdates = true);
    ~KisImageLayerRemoveCommand() override;

    void redo() override;
    void undo() override;

private:
    void addSubtree(KisImageWSP image, KisNodeSP node);

private:
    KisNodeSP m_node;
    bool m_doRedoUpdates;
    bool m_doUndoUpdates;
};

#endif /* __KIS_IMAGE_LAYER_REMOVE_COMMAND_H */
