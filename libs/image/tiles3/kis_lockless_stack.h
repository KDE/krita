/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  References:
 *    * Maged M. Michael, Safe memory reclamation for dynamic
 *      lock-free objects using atomic reads and writes,
 *      Proceedings of the twenty-first annual symposium on
 *      Principles of distributed computing, July 21-24, 2002,
 *      Monterey, California
 *
 *    * Idea of m_deleteBlockers is taken from Andrey Gulin's blog
 *      http://users.livejournal.com/_foreseer/34284.html
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

#ifndef __KIS_LOCKLESS_STACK_H
#define __KIS_LOCKLESS_STACK_H

#include <QAtomicPointer>

template<class T>
class KisLocklessStack
{
private:
    struct Node {
        Node *next;
        T data;
    };

public:
    KisLocklessStack() { }
    ~KisLocklessStack() {

        freeList(m_top.fetchAndStoreOrdered(0));
        freeList(m_freeNodes.fetchAndStoreOrdered(0));
    }

    void push(T data) {
        Node *newNode = new Node();
        newNode->data = data;

        Node *top;

        do {
            top = m_top;
            newNode->next = top;
        } while (!m_top.testAndSetOrdered(top, newNode));

        m_numNodes.ref();
    }

    bool pop(T &value) {
        bool result = false;

        m_deleteBlockers.ref();

        while(1) {
            Node *top = (Node*) m_top;
            if(!top) break;

            // This is safe as we ref'ed m_deleteBlockers
            Node *next = top->next;

            if(m_top.testAndSetOrdered(top, next)) {
                m_numNodes.deref();
                result = true;

                value = top->data;

                /**
                 * Test if we are the only delete blocker left
                 * (it means that we are the only owner of 'top')
                 * If there is someone else in "delete-blocked section",
                 * then just add the struct to the list of free nodes.
                 */
                if (m_deleteBlockers == 1) {
                    cleanUpNodes();
                    delete top;
                }
                else {
                    releaseNode(top);
                }

                break;
            }
        }

        m_deleteBlockers.deref();

        return result;
    }

    void clear() {
        // a fast-path without write ops
        if(!m_top) return;

        m_deleteBlockers.ref();

        Node *top = m_top.fetchAndStoreOrdered(0);

        int removedChunkSize = 0;
        Node *tmp = top;
        while(tmp) {
            removedChunkSize++;
            tmp = tmp->next;
        }
        m_numNodes.fetchAndAddOrdered(-removedChunkSize);

        while(top) {
            Node *next = top->next;

            if (m_deleteBlockers == 1) {
                /**
                 * We  are the only owner of top contents.
                 * So we can delete it freely.
                 */
                cleanUpNodes();
                freeList(top);
                next = 0;
            }
            else {
                releaseNode(top);
            }

            top = next;
        }

        m_deleteBlockers.deref();
    }

    void mergeFrom(KisLocklessStack<T> &other) {
        Node *otherTop = other.m_top.fetchAndStoreOrdered(0);
        if (!otherTop) return;

        int removedChunkSize = 1;
        Node *last = otherTop;
        while(last->next) {
            removedChunkSize++;
            last = last->next;
        }
        other.m_numNodes.fetchAndAddOrdered(-removedChunkSize);

        Node *top;

        do {
            top = m_top;
            last->next = top;
        } while (!m_top.testAndSetOrdered(top, otherTop));

        m_numNodes.fetchAndAddOrdered(removedChunkSize);
    }

    /**
     * This is impossible to measure the size of the stack
     * in highly concurrent environment. So we return approximate
     * value! Do not rely on this value much!
     */
    qint32 size() {
        return m_numNodes;
    }

    bool isEmpty() {
        return !m_numNodes;
    }

private:

    inline void releaseNode(Node *node) {
        Node *top;
        do {
            top = m_freeNodes;
            node->next = top;
        } while (!m_freeNodes.testAndSetOrdered(top, node));
    }

    inline void cleanUpNodes() {
        Node *cleanChain = m_freeNodes.fetchAndStoreOrdered(0);
        if (!cleanChain) return;

        /**
         * If we are the only users of the objects is cleanChain,
         * then just free it. Otherwise, push them back into the
         * recycling list and keep them there till another
         * chance comes.
         */
        if (m_deleteBlockers == 1) {
            freeList(cleanChain);
        } else {
            Node *last = cleanChain;
            while (last->next) last = last->next;

            Node *freeTop;

            do {
                freeTop = m_freeNodes;
                last->next = freeTop;
            } while (!m_freeNodes.testAndSetOrdered(freeTop, cleanChain));
        }
    }

    inline void freeList(Node *first) {
        Node *next;
        while (first) {
            next = first->next;
            delete first;
            first = next;
        }
    }


private:
    Q_DISABLE_COPY(KisLocklessStack)

    QAtomicPointer<Node> m_top;
    QAtomicPointer<Node> m_freeNodes;

    QAtomicInt m_deleteBlockers;
    QAtomicInt m_numNodes;
};

#endif /* __KIS_LOCKLESS_STACK_H */

