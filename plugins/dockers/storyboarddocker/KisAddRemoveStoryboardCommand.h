/*
 *  Copyright (c) 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISADDREMOVESTORYBOARDCOMMAND_H
#define KISADDREMOVESTORYBOARDCOMMAND_H

#include "kundo2command.h"
#include "kis_command_ids.h"
#include "kis_types.h"

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

class KisDuplicateStoryboardCommand : public KUndo2Command
{
public:
    KisDuplicateStoryboardCommand(int position, StoryboardModel *model, KUndo2Command *parent = 0);
    ~KisDuplicateStoryboardCommand();

    void redo() override;
    void undo() override;

private:
    int m_position;
    StoryboardItemSP m_duplicate;
    StoryboardModel *m_model;
    QScopedPointer<KUndo2Command> m_addCommand;
    QScopedPointer<KUndo2Command> m_keyframeCommands;
};

//For moving of Storyboardds.
class KisMoveStoryboardCommand : public KUndo2Command
{
public:
    KisMoveStoryboardCommand(int from, int to, int count, StoryboardModel *model, KUndo2Command *parent = 0);
    ~KisMoveStoryboardCommand();
    void redo() override;
    void undo() override;

private:
    int m_from;
    int m_count;
    int m_to;
    StoryboardModel *m_model;
};

class KisVisualizeStoryboardCommand : public KUndo2Command
{
public:
    KisVisualizeStoryboardCommand(int fromTime, int toSceneIndex, StoryboardModel* model, KisImageSP image, KUndo2Command *parent = 0);
    ~KisVisualizeStoryboardCommand();
    void redo() override;
    void undo() override;

private:
    int m_fromTime;
    int m_toSceneIndex;
    StoryboardModel *m_model;
    KisImageSP m_image;

};

class KisStoryboardChildEditCommand : public KUndo2Command
{
public:
    KisStoryboardChildEditCommand(QVariant oldValue, QVariant newValue, int parentRow, int childRow, StoryboardModel *model, KUndo2Command *parent = 0);
    ~KisStoryboardChildEditCommand(){}
    void redo() override;
    void undo() override;

    int id() const override { return KisCommandUtils::ChangeStoryboardChild; }
    bool mergeWith(const KUndo2Command *other) override;

private:
    QVariant m_oldValue;
    QVariant m_newValue;
    int m_parentRow;
    int m_childRow;
    StoryboardModel *m_model;
};

#endif
