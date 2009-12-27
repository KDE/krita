/* This file is part of the KDE project
   Copyright (C) 2001 Werner Trobin <trobin@kde.org>
   Copyright (C) 2009 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "priorityqueue_test.h"
#include <PriorityQueue_p.h>
#include <kdebug.h>
#include <QList>
#include <ctime>

struct Node
{
    Node(unsigned int key) : m_key(key), m_index(0) {}

    unsigned int key() const {
        return m_key;
    }
    void setKey(unsigned int key) {
        m_key = key;
    }

    int index() const {
        return m_index;
    }
    void setIndex(int i) {
        m_index = i;
    }
private:
    unsigned int m_key;
    int m_index;
};

static const char* const keys[] =
{
    "one",  "two", "three",  "four", "five",
    "six", "seven", "eight", "nine", "ten",
    "eleven", "twelve", 0
};


void PriorityQueue_test::testQueue()
{
    QList<Node*> list;
    QHash<QByteArray, Node*> dict;

    KOfficeFilter::PriorityQueue<Node> queue;

    srand(time(0));
    for (int i = 0; i < 12; ++i) {
        Node *n = new Node(rand() % 20);
        list.append(n);
        queue.insert(n);
        Node *n2 = new Node(*n);
        dict.insert(keys[i], n2);
    }

    kDebug() << "##### Queue 1:";
    queue.dump();
    QCOMPARE((int) queue.count(), list.count());
    QCOMPARE(queue.isEmpty(), false);
    QCOMPARE(queue.extractMinimum()->index(), 0);


    kDebug() << "##### Queue 2:";
    KOfficeFilter::PriorityQueue<Node> queue2(dict);
    //queue2.dump();

    Node *n = list.at(6);
    kDebug() << "##### Decreasing node:" << n->key() << " at" << n->index();
    n->setKey(2);
    queue.keyDecreased(n);
    queue.dump();

    n = list.at(2);
    kDebug() << "##### Decreasing node:" << n->key() << " at" << n->index();
    n->setKey(0);
    queue.keyDecreased(n);
    queue.dump();

    n = queue.extractMinimum();
    while (n) {
        queue.dump();
        n = queue.extractMinimum();
    }
}

QTEST_MAIN(PriorityQueue_test)
#include <priorityqueue_test.moc>
