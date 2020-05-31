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

#include "storyboardModelTest.h"

#include <QTest>
#include <QWidget>
#include "storyboardModel.h"
#include "commentModel.h"

void StoryboardModelTest::init()
{
    m_commentModel = new CommentModel(0);
    m_storyboardModel = new StoryboardModel(0);

    QCOMPARE(m_storyboardModel->columnCount(), 1);
    m_commentModel->insertRows(m_storyboardModel->rowCount(),1);
    QCOMPARE(m_commentModel->rowCount(), 1);
}

void StoryboardModelTest::cleanup()
{
    delete m_storyboardModel;
    delete m_commentModel;
}

void StoryboardModelTest::testAddComment()
{
    int commentStoryboard = m_storyboardModel->commentCount();
    int rowsComment = m_commentModel->rowCount();

    QCOMPARE(commentStoryboard, rowsComment);

    m_commentModel->insertRows(m_commentModel->rowCount(),1);
    auto tester = new QAbstractItemModelTester(m_storyboardModel, 0);
    auto tester = new QAbstractItemModelTester(m_commentModel, 0);

    QCOMPARE(rowsComment + 1, m_commentModel->rowCount());

    QCOMPARE(m_storyboardModel->commentCount(), m_commentModel->rowCount());
}

void StoryboardModelTest::testRemoveComment()
{
    int commentStoryboard = m_storyboardModel->commentCount();
    int rowsComment = m_commentModel->rowCount();

    QCOMPARE(commentStoryboard, rowsComment);

    m_commentModel->removeRows(m_commentModel->rowCount(),1);

    auto tester = new QAbstractItemModelTester(m_storyboardModel, 0);
    auto tester = new QAbstractItemModelTester(m_commentModel, 0);

    QCOMPARE(rowsComment - 1, m_commentModel->rowCount());
    QCOMPARE(m_storyboardModel->commentCount(), m_commentModel->rowCount());
}

void StoryboardModelTest::testCommentNameChanged()
{
    QModelIndex index = m_commentModel->createIndex(m_commentModel->rowCount(),m_commentModel->columnCount());
    QVariant value = QVariant(QString("newValue"));
    m_commentModel->setData(index, value);
    auto tester = new QAbstractItemModelTester(m_commentModel, 0);
}

void StoryboardModelTest::testCommentVisibilityChanged()
{

}

void StoryboardModelTest::testFrameAdded()
{
    m_storyboardModel->insertRows(m_storyboardModel->rowCount(),1);
    auto tester = new QAbstractItemModelTester(m_storyboardModel, 0);
}

void StoryboardModelTest::testFrameRemoved()
{
    m_storyboardModel->removeRows(m_storyboardModel->rowCount(),1);
    auto tester = new QAbstractItemModelTester(m_storyboardModel, 0);
}

void StoryboardModelTest::testFrameChanged()
{
    QModelIndex index = m_commentModel->createIndex(m_storyboardModel->rowCount(),m_storyboardModel->columnCount());
    QVariant value = QVariant(100);
    m_stroyboardModel->setData(index, value, StoryboardModel::FrameRole);
    auto tester = new QAbstractItemModelTester(m_storyboardModel, 0);
    //should we have multiple custom roles to differentiate between what data is coming in??
}

void StoryboardModelTest::testDurationChanged()
{
    QModelIndex index = m_commentModel->createIndex(m_storyboardModel->rowCount(),m_storyboardModel->columnCount());
    QVariant value = QVariant(100);
    m_stroyboardModel->setData(index, value, StoryboardModel::DurationRole);
    auto tester = new QAbstractItemModelTester(m_storyboardModel, 0);
}

void StoryboardModelTest::testCommentChanged()
{
    QModelIndex index = m_commentModel->createIndex(m_storyboardModel->rowCount(),m_storyboardModel->columnCount());
    QVariant value = QVariant(100);
    m_stroyboardModel->setData(index, value, StoryboardModel::CommentRole);
    auto tester = new QAbstractItemModelTester(m_storyboardModel, 0);
    //should we store different comments in different columns??
}

QTEST_MAIN(StoryboardModelTest)
