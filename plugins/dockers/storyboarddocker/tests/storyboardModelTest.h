/*
 *  Copyright (c) 2020 Saurabh Kumar <saurabhk660@gmail.com>
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

#ifndef __STORYBOARD_MODEL_TEST_H
#define __STORYBOARD_MODEL_TEST_H

#include <QtTest>

class CommentModel;
class StoryboardModel;

class StoryboardModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init();
    void cleanup();

    //interaction with comment model
    void testAddComment()
    void testRemoveComment();
    void testCommentNameChanged();
    void testCommentVisibilityChanged();

    //"storyboard model only" tests
    void testFrameAdded();
    void testFrameRemoved();
    void testFrameChanged();
    void testDurationChanged();
    void testCommentChanged();

private:
    CommentModel *m_commentModel;
    StoryboardModel *m_storyboardModel;

};

#endif /* __STORYBOARD_MODEL_TEST_H */
