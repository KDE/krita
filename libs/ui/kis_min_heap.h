/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2009 Vera Lukman <shicmap@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KIS_MIN_HEAP_H
#define KIS_MIN_HEAP_H

#include <kis_debug.h>
#include <QList>

template <typename T> struct PriorityNode {
    T data;
    int key;
    int pos;
};

template <typename T, int N> class KisMinHeap
{
public:

    KisMinHeap()
        : m_last(0),
          m_size(N),
          m_list(new PriorityNode <T>* [N])
    {
    }

    inline KisMinHeap(const T& data, int key)
        : KisMinHeap()
    {
        append(data, key);
    }

    inline KisMinHeap(PriorityNode<T>* node)
        : KisMinHeap()
    {
        append(node);
    }

    ~KisMinHeap() {
        delete[] m_list;
    }

    inline void changeKey(int pos, int newKey) {
        m_list[pos]->key = newKey;
        heapifyUp(pos);
        heapifyDown(pos);
    }

    inline int size() {
        return m_last;
    }

    inline T valueAt(int pos) {
        return m_list[pos]->data;
    }

    void append(PriorityNode<T>* node) {
        node->pos = m_last;
        m_list[m_last] = node;
        ++m_last;
        heapifyUp(node->pos);

        node = 0;
    }

    void append(const T& data, int key) {
        if (m_last >= m_size) return;
        PriorityNode <T>* node = new PriorityNode<T>;
        node->data = data;
        node->key = key;

        append(node);
    }

    void remove(int pos) {
        if (pos < 0) return;
        swap(pos, m_last - 1);
        --m_last;
        delete m_list[m_last];
        m_list[m_last] = 0;
        heapifyUp(pos);
        heapifyDown(pos);
    }

    void remove(const T& data) {
        int pos = find(data);
        if (pos >= 0) remove(pos);
    }

    int find(const T& data) {
        for (int pos = 0; pos < m_last; pos++) {
            if (m_list[pos]->data == data) return pos;
        }
        return -1;
    }

private:

    int m_last;
    int m_size;
    PriorityNode <T>* *m_list;

    void swap(int pos1, int pos2) {
        PriorityNode <T>* temp(m_list[pos1]);
        m_list[pos1] = m_list[pos2];
        m_list[pos1]->pos = pos1;
        m_list[pos2] = temp;
        m_list[pos2]->pos = pos2;
        temp = 0;
    }

    void heapifyUp(int pos) {
        while (pos > 0 && m_list[pos]->key < m_list[parentPos(pos)]->key) {
            swap(pos, parentPos(pos));
            pos = parentPos(pos);
        }
    }

    void heapifyDown(int pos) {
        if (leftChildPos(pos) >= m_last) return; //no children
        else {
            int childPos = 0;

            if (rightChildPos(pos) >= m_last) { //1 child
                childPos = leftChildPos(pos);
            } else { //2 children
                m_list[leftChildPos(pos)]->key < m_list[rightChildPos(pos)]->key ? childPos = leftChildPos(pos) :
                        childPos = rightChildPos(pos);
            }

            if (m_list[childPos]->key < m_list[pos]->key) {
                swap(pos, childPos);
                heapifyDown(childPos);
            } else return;
        }
    }

    inline int leftChildPos(int x) {
        return 2 * x + 1;
    }

    inline int rightChildPos(int x) {
        return 2 * x + 2;
    }

    inline int parentPos(int x) {
        return (x - 1) / 2;
    }

};

#endif // HEAP_H
