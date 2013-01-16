/*
 *  tool_transform_commands.h - part of Krita
 *
 *  Copyright (c) 2010 Marc Pegon <pe.marc@free.fr>
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

#ifndef TOOL_TRANSFORM_COMMANDS_H_
#define TOOL_TRANSFORM_COMMANDS_H_

#include "kis_tool_transform.h"
#include <kis_transaction.h>
#include <KoToolManager.h>

/**
 * There are 2 command classes associated with the transform tool,
 * because there are 2 different actions which should be committed
 * to the Undo Stack : when the user changes the arguments of
 * the transformation, i.e. the preview is modified (TransformCmd),
 * and when the user applies the transformation, i.e. the paintDevice
 * is modified (ApplyTransformCmd)
*/

class ApplyTransformCmdData : public KisSelectedTransactionData
{
public:
    ApplyTransformCmdData(KisToolTransform *tool, ToolTransformArgs::TransformMode mode, KisNodeSP node);
    virtual ~ApplyTransformCmdData();

public:
    virtual void redo();
    virtual void undo();
    ToolTransformArgs::TransformMode mode() const;
private:
    KisToolTransform *m_tool;
    ToolTransformArgs::TransformMode m_mode;
};


class ApplyTransformCmd : public KisTransaction
{
public:
    ApplyTransformCmd(KisToolTransform *tool, ToolTransformArgs::TransformMode mode, KisNodeSP node);
};

class TransformCmd : public KUndo2Command
{
public:
    TransformCmd(KisToolTransform *tool, const ToolTransformArgs &args, KisSelectionSP origSel, QPoint startPos, QPoint endPos, const QImage &origImg);
    virtual ~TransformCmd();

public:
    virtual void redo();
    virtual void undo();
    void transformArgs(ToolTransformArgs &args) const;
    KisSelectionSP origSelection(QPoint &startPos, QPoint &endPos) const;

    const QImage &originalImage() const;

private:
    ToolTransformArgs m_args;
    KisToolTransform *m_tool;
    KisSelectionSP m_origSelection;
    QPoint m_originalTopLeft;
    QPoint m_originalBottomRight;
    QImage m_origImg;
};

#endif // TOOL_TRANSFORM_COMMANDS_H_
