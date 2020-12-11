/*
 *  Copyright (c) 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <StoryboardItem.h>
#include <kis_time_span.h>

#include "StoryboardModel.h"
#include "KisAddRemoveStoryboardCommand.h"

KisAddStoryboardCommand::KisAddStoryboardCommand(int position,
                                                StoryboardItemSP item,
                                                StoryboardModel *model,
                                                KUndo2Command *parent)
    : KUndo2Command(kundo2_i18n("Add Storyboard Scene"), parent)
    , m_position(position)
    , m_item(new StoryboardItem(*item))
    , m_modelItem(item)
    , m_model(model)
{
}

KisAddStoryboardCommand::~KisAddStoryboardCommand()
{
}

void KisAddStoryboardCommand::redo()
{
    QModelIndex nextIndex = m_model->index(m_position, 0);
    if (nextIndex.isValid()){
        const int firstFrameOfScene = m_model->data(m_model->index(StoryboardItem::FrameNumber, 0, nextIndex)).toInt();
        int durationDeletedScene = m_item->child(StoryboardItem::DurationSecond)->data().toInt() * m_model->getFramesPerSecond()
                                + m_item->child(StoryboardItem::DurationFrame)->data().toInt();
        m_model->shiftKeyframes(KisTimeSpan::infinite(firstFrameOfScene), durationDeletedScene);
    }
    KUndo2Command::redo();
    m_model->insertRow(m_position);
    m_model->insertChildRows(m_position, m_item);
}

void KisAddStoryboardCommand::undo()
{
    updateItem();
    KUndo2Command::undo();

    QModelIndex nextIndex = m_model->index(m_position, 0);
    if (nextIndex.isValid()){
        const int firstFrameOfScene = m_model->data(m_model->index(StoryboardItem::FrameNumber, 0, nextIndex)).toInt();
        int durationDeletedScene = m_item->child(StoryboardItem::DurationSecond)->data().toInt() * m_model->getFramesPerSecond()
                                + m_item->child(StoryboardItem::DurationFrame)->data().toInt();
        m_model->shiftKeyframes(KisTimeSpan::infinite(firstFrameOfScene), -durationDeletedScene);
    }
    m_model->removeItem(m_model->index(m_position, 0));
}

void KisAddStoryboardCommand::updateItem()
{
    m_item->cloneChildrenFrom(*m_modelItem);
}


//remove command
KisRemoveStoryboardCommand::KisRemoveStoryboardCommand(int position,
                                                StoryboardItemSP item,
                                                StoryboardModel *model,
                                                KUndo2Command *parent)
    : KUndo2Command(kundo2_i18n("Remove Storyboard Scene"), parent)
    , m_position(position)
    , m_item(new StoryboardItem(*item))
    , m_model(model)
{
}

KisRemoveStoryboardCommand::~KisRemoveStoryboardCommand()
{
}

void KisRemoveStoryboardCommand::redo()
{
    KUndo2Command::redo();
    m_model->removeItem(m_model->index(m_position, 0));
}

void KisRemoveStoryboardCommand::undo()
{
    KUndo2Command::undo();
    m_model->insertRow(m_position);
    m_model->insertChildRows(m_position, m_item);
}

KisMoveStoryboardCommand::KisMoveStoryboardCommand(int from,
                                                    int count,
                                                    int to,
                                                    StoryboardModel *model,
                                                    KUndo2Command *parent)
    : KUndo2Command(kundo2_i18n("Move Storyboard Scene"), parent)
    , m_from(from)
    , m_count(count)
    , m_to(to)
    , m_model(model)
{
}

KisMoveStoryboardCommand::~KisMoveStoryboardCommand()
{
}

void KisMoveStoryboardCommand::redo()
{
    m_model->moveRows(QModelIndex(), m_from, m_count, QModelIndex(), m_to);
}

void KisMoveStoryboardCommand::undo()
{
    m_model->moveRows(QModelIndex(), m_to, m_count, QModelIndex(), m_from + 1);      
}
