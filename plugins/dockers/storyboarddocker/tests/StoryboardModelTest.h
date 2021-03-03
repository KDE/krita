/*
 *  SPDX-FileCopyrightText: 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __STORYBOARD_MODEL_TEST_H
#define __STORYBOARD_MODEL_TEST_H

#include <simpletest.h>

class StoryboardCommentModel;
class StoryboardModel;

class StoryboardModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init();
    void cleanup();

    //interaction with comment model
    void testAddComment();
    void testRemoveComment();
    void testCommentNameChanged();

    //"storyboard model only" tests
    void testFrameAdded();
    void testFrameRemoved();
    void testFrameChanged();
    void testDurationChanged();
    void testCommentChanged();

private:
    StoryboardCommentModel *m_commentModel;
    StoryboardModel *m_storyboardModel;

};

#endif /* __STORYBOARD_MODEL_TEST_H */
