/*
 *  Copyright (c) 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISADDREMOVESTORYBOARDCOMMAND_H
#define KISADDREMOVESTORYBOARDCOMMAND_H

#include "kis_command_utils.h"

class StoryboardModel;

//this class should handle undo/redo for deletion and addition of Storyboardds.

class KisAddRemoveStoryboardCommand : public KisCommandUtils::FlipFlopCommand
{
public:
    KisAddRemoveStoryboardCommand(int position, StoryboardItemSP item, StoryboardModel *model, KisCommandUtils::FlipFlopCommand::State state, KUndo2Command *parent = 0);
    ~KisAddRemoveStoryboardCommand();

    void partA() override;
    void partB() override;

private:
    int m_position;
    StoryboardItemSP m_item;
    StoryboardModel *m_model;
};

struct KisAddStoryboardCommand : KisAddRemoveStoryboardCommand
{
    KisAddStoryboardCommand(int position, StoryboardItemSP item, StoryboardModel *model, KUndo2Command *parent = 0);
};

struct KisRemoveStoryboardCommand : KisAddRemoveStoryboardCommand
{
    KisRemoveStoryboardCommand(int position, StoryboardItemSP item, StoryboardModel *model, KUndo2Command *parent = 0);
};

#endif