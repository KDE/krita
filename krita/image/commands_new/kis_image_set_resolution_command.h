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

#ifndef KIS_IMAGE_SET_RESOLUTION_COMMAND_H
#define KIS_IMAGE_SET_RESOLUTION_COMMAND_H

#include "krita_export.h"
#include "kis_types.h"

#include <kundo2command.h>


class KRITAIMAGE_EXPORT KisImageSetResolutionCommand : public KUndo2Command
{
public:
    KisImageSetResolutionCommand(KisImageWSP image, qreal newXRes, qreal newYRes, KUndo2Command *parent = 0);
    void undo();
    void redo();

private:
    KisImageWSP m_image;

    qreal m_newXRes;
    qreal m_newYRes;
    qreal m_oldXRes;
    qreal m_oldYRes;
};

/**
 * A special workaround command for updating the shapes.  It resets
 * shapes always (for both undo() and redo() actions) after all the
 * child commands are finished. Usually, it should have the only child
 * KisImageSetResolutionCommand.
 *
 * Usecase: When you change the resolution of the image, the
 * projection of the vector layer is still rendered in old
 * resolution. So you should reset it and render again.
 */
class KRITAIMAGE_EXPORT KisResetShapesCommand : public KUndo2Command
{
public:
    KisResetShapesCommand(KisNodeSP rootNode);

    void undo();
    void redo();

private:
    void resetNode(KisNodeSP node);

private:
    KisNodeSP m_rootNode;
};

#endif // KIS_IMAGE_SET_RESOLUTION_COMMAND_H
