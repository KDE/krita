/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_IMAGE_COMMAND_H_
#define KIS_IMAGE_COMMAND_H_

#include <kritaimage_export.h>

#include <kundo2command.h>

#include <QSize>
#include <QRect>
#include "kis_types.h"
#include "kis_paint_device.h"


/// the base command for commands altering a KisImage
class KRITAIMAGE_EXPORT KisImageCommand : public KUndo2Command
{

public:
    /**
     * Constructor
     * @param name The name that will be shown in the ui
     * @param image The image the command will be working on.
     * @param parent The parent command.
     */
    KisImageCommand(const KUndo2MagicString& name, KisImageWSP image, KUndo2Command *parent = 0);
    ~KisImageCommand() override;

protected:

    /**
     * Used for performing the smallest update
     * after a node has been removed from stack.
     * First tries to setDirty() node's siblings.
     * If it doesn't help, performs full refresh.
     */
    class UpdateTarget
    {
    public:
        UpdateTarget(KisImageWSP image, KisNodeSP removedNode, const QRect &updateRect);
        void update();

    private:
        KisImageWSP m_image;
        QRect m_updateRect;
        int m_removedNodeIndex;
        KisNodeSP m_removedNodeParent;
    };

protected:
    KisImageWSP m_image;
};

#endif // KIS_IMAGE_COMMAND_H_
