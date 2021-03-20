/* This file is part of the Calligra libraries
   SPDX-FileCopyrightText: 2001 Werner Trobin <trobin@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef priority_queue_h
#define priority_queue_h

#include <vector>
#include <QString>
#include <QHash>
#include <kis_debug.h>

// Better put all those internal classes in some namespace to avoid clashes
// as the names are quite generic
namespace CalligraFilter
{

/**
 * This PriorityQueue class is implemented as "upside down" heap, i.e. the item
 * with the \b smallest key is at the root of the heap.
 * If you feel like using that class your template parameter has to have a public
 * method "unsigned int key() const" which returns - surprise - the key. The
 * supplied key must not be "negative." We use UINT_MAX as value to represent
 * "infinity" for the shortest path algorithm.
 * As this is a very specialized class we also demand a "void setIndex(int i)"
 * method in order to tell nodes where they are located inside the heap. This is
 * a very ugly approach, but it's the only way I see if you want to avoid a O(n)
 * search for the item where you decreased the key :}
 * Just to make it even worse we also use a "int index() const" method
 * to fetch the index... well, you most likely would need one anyway ;)
 * Note: This class is pointer based (like QPtr*) - if you create PriorityQueue<X>
 * we actually operate on X* ! We don't care about deleting your pointers at all.
 * We don't copy them, we don't create new ones,... - you own them, you have
 * to delete them :)
 *
 * In case you change a key value make sure to call keyDecreased and pass the
 * item you touched. This is running in O(log n) according to Cormen...
 *
 * All the ideas are stol^H^H^H^Hborrowed from "Introduction to Algorithms",
 * Cormen et al
 *
 * @author Werner Trobin <trobin@kde.org>
 */
template<class T> class PriorityQueue
{

public:
    PriorityQueue() {}
    PriorityQueue(const PriorityQueue<T>& rhs) : m_vector(rhs.m_vector) {}
    PriorityQueue(const QHash<QByteArray, T*>& items);
    ~PriorityQueue() {}

    PriorityQueue<T> &operator=(const PriorityQueue<T>& rhs) {
        m_vector = rhs.m_vector; return *this;
    }
    bool operator==(const PriorityQueue<T>& rhs) {
        return m_vector == rhs.m_vector;
    }

    unsigned int count() const {
        return m_vector.size();
    }
    bool isEmpty() const {
        return m_vector.empty();
    }

    void insert(T* item);

    /**
     * Call this method after decreasing the key of the ith item. The heap
     * properties will no longer be valid if you either forget to call that
     * method, or if you \b increase the key.
     */
    void keyDecreased(T* item);

    T* extractMinimum();

    /**
     * For debugging
     */
    void dump() const;

private:
    // Note: We have to use a 1-based index here, and we get/return 0-based ones
    int parent(int i) {
        return ((i + 1) >> 1) - 1;
    }
    int left(int i) {
        return ((i + 1) << 1) - 1;
    }
    int right(int i) {
        return (i + 1) << 1;
    }

    void heapify(int i);
    // item is !=0 for sure
    void bubbleUp(T* item, int i);
    // Builds the heap in the vector in O(n)
    void buildHeap();

    std::vector<T*> m_vector;
};

template<class T>
PriorityQueue<T>::PriorityQueue(const QHash<QByteArray, T*>& items)
    : m_vector(items.count())
{
    // First put all items into the vector
    int i = 0;
    Q_FOREACH (T* item, items) {
        item->setIndex(i);
        m_vector[i] = item;
        ++i;
    }
    // Then build a heap in that vector
    buildHeap();
}

template<class T>
void PriorityQueue<T>::insert(T* item)
{
    if (!item)
        return;
    int i = static_cast<int>(m_vector.size());
    m_vector.push_back(0);   // extend the vector by one item. i == index to the last item
    bubbleUp(item, i);
}

template<class T>
void PriorityQueue<T>::keyDecreased(T* item)
{
    if (!item)
        return;
    bubbleUp(item, item->index());
}

template<class T>
T* PriorityQueue<T>::extractMinimum()
{
    if (m_vector.size() < 1)
        return 0;
    T *min = m_vector[ 0 ];
    m_vector[ 0 ] = m_vector[ m_vector.size() - 1 ];
    // update the index
    m_vector[ 0 ]->setIndex(0);
    m_vector.pop_back();
    heapify(0);
    return min;
}

template<class T>
void PriorityQueue<T>::dump() const
{
    dbgFile << "++++++++++ PriorityQueue::dump ++++++++++";
    QString out;
    int size = static_cast<int>(m_vector.size());
    for (int i = 0; i < size; ++i) {
        if (m_vector[ i ]->index() != i)
            out += " ERROR: index out of sync. Should be " + QString::number(i) + ", is " +
                   QString::number(m_vector[ i ]->index()) + ". ";
        out += QString::number(m_vector[ i ]->key()) + ", ";
    }
    if (out.isEmpty())
        out = "(empty)";
    dbgFile << out;
    dbgFile << "++++++++++ PriorityQueue::dump (done) ++++++++++";
}

template<class T>
void PriorityQueue<T>::heapify(int i)
{
    int l = left(i);
    int r = right(i);
    int size = static_cast<int>(m_vector.size());
    int smallest;

    if (l < size && i < size && m_vector[ l ]->key() < m_vector[ i ]->key())
        smallest = l;
    else
        smallest = i;

    if (r < size && m_vector[ r ]->key() < m_vector[ smallest ]->key())
        smallest = r;

    if (smallest != i) {
        T* tmp = m_vector[ i ];
        m_vector[ i ] = m_vector[ smallest ];
        // update indices
        m_vector[ i ]->setIndex(i);
        tmp->setIndex(smallest);
        m_vector[ smallest ] = tmp;
        heapify(smallest);
    }
}

template<class T>
void PriorityQueue<T>::bubbleUp(T* item, int i)
{
    int p = parent(i);
    while (i > 0 && m_vector[ p ]->key() > item->key()) {
        // update the index first
        m_vector[ p ]->setIndex(i);
        // then move it there
        m_vector[ i ] = m_vector[ p ];
        i = p;
        p = parent(i);
    }
    item->setIndex(i);
    m_vector[ i ] = item;
}

template<class T>
void PriorityQueue<T>::buildHeap()
{
    // from size() / 2 down to 1
    for (int i = (m_vector.size() >> 1) - 1; i >= 0; --i)
        heapify(i);
}

} // namespace Calligra

#endif // priority_queue_h
