/*
 *  Copyright (c) 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <StoryboardItem.h>

#include "StoryboardModel.h"
#include "KisAddRemoveStoryboardCommand.h"

KisAddRemoveStoryboardCommand::KisAddRemoveStoryboardCommand(int position,
                                                            StoryboardItemSP item,
                                                            StoryboardModel *model,
                                                            KisCommandUtils::FlipFlopCommand::State state,
                                                            KUndo2Command *parent)
    : KisCommandUtils::FlipFlopCommand(state, parent)
    , m_position(position)
    , m_item(item)
    , m_model(model)
{
}

KisAddRemoveStoryboardCommand::~KisAddRemoveStoryboardCommand()
{
}

//insert and clone data
void KisAddRemoveStoryboardCommand::partA()
{
    ThumbnailData th = qvariant_cast<ThumbnailData>(m_item->child(0)->data());
    m_model->insertRow(m_position);
    m_model->insertChildRows(m_position, m_item);
}

//removing
void KisAddRemoveStoryboardCommand::partB()
{
    m_model->removeRows(m_position, 1);
}


KisAddStoryboardCommand::KisAddStoryboardCommand(int position, StoryboardItemSP item, StoryboardModel *model, KUndo2Command *parent)
    : KisAddRemoveStoryboardCommand(position, item, model, INITIALIZING, parent)
{
}

KisRemoveStoryboardCommand::KisRemoveStoryboardCommand(int position, StoryboardItemSP item, StoryboardModel *model, KUndo2Command *parent)
    : KisAddRemoveStoryboardCommand(position, item, model, FINALIZING, parent)
{
}
