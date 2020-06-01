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

    m_commentModel->insertRows(m_commentModel->rowCount(),1);
    QCOMPARE(m_commentModel->rowCount(), 1);
}

void StoryboardModelTest::cleanup()
{
    delete m_storyboardModel;
    delete m_commentModel;
}

void StoryboardModelTest::testAddComment()
{

    auto tester = new QAbstractItemModelTester(m_storyboardModel, 0);
    auto tester = new QAbstractItemModelTester(m_commentModel, 0);

    int commentStoryboard = m_storyboardModel->commentCount();
    int rowsComment = m_commentModel->rowCount();

    QCOMPARE(commentStoryboard, rowsComment);

    m_commentModel->insertRows(m_commentModel->rowCount(),1);

    QCOMPARE(rowsComment + 1, m_commentModel->rowCount());
    QCOMPARE(m_storyboardModel->commentCount(), m_commentModel->rowCount());

    //add at an invalid position
    m_commentModel->insertRows(-1, 1);

    QCOMPARE(rowsComment + 1, m_commentModel->rowCount());
    QCOMPARE(m_storyboardModel->commentCount(), m_commentModel->rowCount());

}

void StoryboardModelTest::testRemoveComment()
{

    auto tester = new QAbstractItemModelTester(m_storyboardModel, 0);
    auto tester = new QAbstractItemModelTester(m_commentModel, 0);
    int commentStoryboard = m_storyboardModel->commentCount();
    int rowsComment = m_commentModel->rowCount();

    QCOMPARE(commentStoryboard, rowsComment);

    m_commentModel->removeRows(m_commentModel->rowCount(),1);

    QCOMPARE(rowsComment - 1, m_commentModel->rowCount());
    QCOMPARE(m_storyboardModel->commentCount(), m_commentModel->rowCount());

    m_commentModel->removeRows(-1,1);

    QCOMPARE(rowsComment - 1, m_commentModel->rowCount());
    QCOMPARE(m_storyboardModel->commentCount(), m_commentModel->rowCount());
}

void StoryboardModelTest::testCommentNameChanged()
{
    auto tester = new QAbstractItemModelTester(m_commentModel, 0);
    QModelIndex index = m_commentModel->createIndex(m_commentModel->rowCount(),m_commentModel->columnCount());
    QVariant value = QVariant(QString("newValue"));
    m_commentModel->setData(index, value);

    QCOMPARE(QString("newValue"), m_commentModel->data(index));
}

void StoryboardModelTest::testCommentVisibilityChanged()
{
    QModelIndex index = m_commentModel->createIndex(m_commentModel->rowCount(),m_commentModel->columnCount());
    QIcon prevIcon = m_commentModel->data(index, Qt::DecorationRole);
    m_commentModel->setData(index, true, Qt::DecorationRole);
    QIcon currIcon = m_commentModel->data(index, Qt::DecorationRole);

    QVERIFY(prevIcon != currIcon);
}

void StoryboardModelTest::testFrameAdded()
{
    int rows = m_storyboardModel->rowCount();
    auto tester = new QAbstractItemModelTester(m_storyboardModel, 0);
    m_storyboardModel->insertRows(m_storyboardModel->rowCount(),1);

    QCOMPARE(rows + 1, m_storyboardModel->rowCount());
}

void StoryboardModelTest::testFrameRemoved()
{
    int rows = m_storyboardModel->rowCount();
    auto tester = new QAbstractItemModelTester(m_storyboardModel, 0);
    m_storyboardModel->removeRows(m_storyboardModel->rowCount(),1);

    QCOMPARE(rows-1, m_storyboardModel->rowCount());
}

void StoryboardModelTest::testFrameChanged()
{

    auto tester = new QAbstractItemModelTester(m_storyboardModel, 0);
    QModelIndex index = m_commentModel->createIndex(m_storyboardModel->rowCount(), 1);
    QVariant value = QVariant(100);
    m_stroyboardModel->setData(index, value, Qt::EditRole);

    QCOMPARE(m_stroyboardModel->data(index), 100);

    //invalid value shouldn't change anything
    QVariant value = QVariant(-100);
    m_stroyboardModel->setData(index, value, Qt::EditRole);

    QVERIFY(m_stroyboardModel->data(index), 100);


}

void StoryboardModelTest::testDurationChanged()
{
    auto tester = new QAbstractItemModelTester(m_storyboardModel, 0);
    QModelIndex index = m_commentModel->createIndex(m_storyboardModel->rowCount(), 2);
    QVariant value = QVariant(100);
    m_stroyboardModel->setData(index, value, Qt::EditRole);

    QCOMPARE(m_stroyboardModel->data(index), 100);

    //invalid value shouldn't change anything
    QVariant value = QVariant(-1);
    m_stroyboardModel->setData(index, value, Qt::EditRole);

    QVERIFY(m_stroyboardModel->data(index), 100);
}

void StoryboardModelTest::testCommentChanged()
{
    auto tester = new QAbstractItemModelTester(m_storyboardModel, 0);
    QModelIndex index = m_commentModel->createIndex(m_storyboardModel->rowCount(),4);
    QVariant value = QVariant(QString("newComment"));
    m_stroyboardModel->setData(index, value, Qt::EditRole);

    QCOMPARE(m_stroyboardModel->data(index,Qt::EditRole), QString("newComment"));
}

QTEST_MAIN(StoryboardModelTest)
