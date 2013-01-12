/*
 *  tool_transform_commands.cc -- part of Krita
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

#include "tool_transform_commands.h"
#include <kis_image.h>

ApplyTransformCmdData::ApplyTransformCmdData(KisToolTransform *tool, ToolTransformArgs::TransformMode mode, KisNodeSP node)
        : KisSelectedTransactionData(i18n("Apply transformation"), node)
        , m_tool(tool)
{
    m_mode = mode;
}

ApplyTransformCmdData::~ApplyTransformCmdData()
{
}

ToolTransformArgs::TransformMode ApplyTransformCmdData::mode() const
{
    return m_mode;
}

void ApplyTransformCmdData::redo()
{
    KisSelectedTransactionData::redo();
}

void ApplyTransformCmdData::undo()
{
    KisSelectedTransactionData::undo();
}

ApplyTransformCmd::ApplyTransformCmd(KisToolTransform *tool, ToolTransformArgs::TransformMode mode, KisNodeSP node)
{
    m_transactionData = new ApplyTransformCmdData(tool, mode, node);
}

TransformCmd::TransformCmd(KisToolTransform *tool, const ToolTransformArgs &args, KisSelectionSP origSel, QPoint startPos, QPoint endPos, const QImage &origImg, const QImage &origSelectionImg)
        : KUndo2Command(i18nc("(qtundo-format)", "Transform"))
{
    m_args = args;
    m_tool = tool;
    m_origSelection = origSel;
    m_originalTopLeft = startPos;
    m_originalBottomRight = endPos;
    m_origImg = origImg;
    m_origSelectionImg = origSelectionImg;
}

TransformCmd::~TransformCmd()
{
}

void TransformCmd::transformArgs(ToolTransformArgs &args) const
{
    args = m_args;
}

KisSelectionSP TransformCmd::origSelection(QPoint &originalTopLeft, QPoint &originalBottomRight) const
{
    originalTopLeft = m_originalTopLeft;
    originalBottomRight = m_originalBottomRight;
    return m_origSelection;
}

const QImage &TransformCmd::originalImage() const
{
    return m_origImg;
}

const QImage &TransformCmd::originalSelectionImage() const
{
    return m_origSelectionImg;
}

void TransformCmd::redo()
{
    // the preview is modified
    // we activate the transform tool, because if it isn't activated
    // the preview isn't displayed (the user doesn't know what has happened)
    KoToolManager *manager = KoToolManager::instance();
    if (manager && m_tool && manager->activeToolId() != m_tool->toolId())
        manager->switchToolRequested(m_tool->toolId());
}

void TransformCmd::undo()
{
    // same thing as redo
    KoToolManager *manager = KoToolManager::instance();
    if (manager && m_tool && manager->activeToolId() != m_tool->toolId())
        manager->switchToolRequested(m_tool->toolId());
}
