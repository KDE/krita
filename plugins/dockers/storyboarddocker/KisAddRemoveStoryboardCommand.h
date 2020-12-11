/*
 *  Copyright (c) 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISADDREMOVESTORYBOARDCOMMAND_H
#define KISADDREMOVESTORYBOARDCOMMAND_H

#include "kundo2command.h"

class StoryboardModel;

//For addition of Storyboardds.
class KisAddStoryboardCommand : public KUndo2Command
{
public:
    KisAddStoryboardCommand(int position, StoryboardItemSP item, StoryboardModel *model, KUndo2Command *parent = 0);
    ~KisAddStoryboardCommand();

    void redo() override;
    void undo() override;
    void updateItem();

private:
    int m_position;
    StoryboardItemSP m_item;
    StoryboardItemSP m_modelItem;
    StoryboardModel *m_model;
};

//For removal of Storyboardds.
class KisRemoveStoryboardCommand : public KUndo2Command
{
public:
    KisRemoveStoryboardCommand(int position, StoryboardItemSP item, StoryboardModel *model, KUndo2Command *parent = 0);
    ~KisRemoveStoryboardCommand();
    void redo() override;
    void undo() override;

private:
    int m_position;
    StoryboardItemSP m_item;
    StoryboardModel *m_model;
};

#endif