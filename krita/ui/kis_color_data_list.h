/* This file is part of the KDE project
   Copyright 2009 Vera Lukman <shichan.karachu@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KIS_COLOR_DATA_LIST_H
#define KIS_COLOR_DATA_LIST_H

#include "kis_min_heap.h"
#include <QColor>
#include <QList>

class KisColorDataList
{
public:
    static const int MAX_RECENT_COLOR = 12;

    inline KisColorDataList() { m_key = 0; };
    inline int size () { return m_guiList.size(); };
    inline void printPriorityList () { m_priorityList.printHeap(); };
    inline int leastUsedGuiPos() { return findPos(m_priorityList.valueAt(0)); };

    void printGuiList();
    const QColor& guiColor (int pos);
    void append(const QColor&);
    void appendNew(const QColor&);
    void removeLeastUsed();
    void updateKey (int guiPos);

    /*find position of the color on the gui list*/
    int findPos (const QColor&);

private:
    MinHeap <QColor, MAX_RECENT_COLOR> m_priorityList;
    QList <PriorityNode <QColor>*> m_guiList;
    int m_key;

    int guiInsertPos(const QColor&);

    /*compares c1 and c2 based on HSV.
      c1 < c2, returns -1
      c1 = c2, returns 0
      c1 > c2, returns 1 */
    int hsvComparison (const QColor& c1, const QColor& c2);
};

#endif // KIS_COLOR_DATA_LIST_H
