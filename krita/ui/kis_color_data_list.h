#ifndef KIS_COLOR_DATA_LIST_H
#define KIS_COLOR_DATA_LIST_H

#include "heap.h"
#include <QColor>
#include <QList>

class KisColorDataList
{
public:
    static const int MAX_RECENT_COLOR = 3;

    inline KisColorDataList() { m_key = 0; };
    inline int size () { return m_guiList.size(); };
    inline void printPriorityList () { m_priorityList.printHeap(); };
    inline void updateKey (int guiPos) { m_priorityList.changeKey(m_guiList.at(guiPos)->pos, m_key++); };

    void printGuiList();
    const QColor& guiColor (int pos);
    void append(const QColor&);
    void appendNew(const QColor&);
    void removeLeastUsed();

private:
    MinHeap <QColor, MAX_RECENT_COLOR> m_priorityList;
    QList <PriorityNode <QColor>*> m_guiList;
    int m_key;

    int guiInsertPos(const QColor&);
    int findPos (const QColor&);
    /*compares c1 and c2 based on HSV.
      c1 < c2, returns -1
      c1 = c2, returns 0
      c1 > c2, returns 1 */
    int hsvComparison (const QColor& c1, const QColor& c2);
};

#endif // KIS_COLOR_DATA_LIST_H
