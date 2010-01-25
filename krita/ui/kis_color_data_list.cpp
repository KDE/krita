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

#include "kis_color_data_list.h"

#ifndef _MSC_EXTENSIONS
const int KisColorDataList::MAX_RECENT_COLOR;
#endif

KisColorDataList::KisColorDataList()
    :m_priorityList(0)
{
    m_key = 0;
    m_priorityList = new KisMinHeap <KoColor, MAX_RECENT_COLOR> ();
}

KisColorDataList::~KisColorDataList()
{
    delete m_priorityList;
    this->m_priorityList = 0 ;

//    delete m_guiList;
}

void KisColorDataList::appendNew(const KoColor& data)
{
    if (size() >= KisColorDataList::MAX_RECENT_COLOR) removeLeastUsed();

    PriorityNode<KoColor> * node;
    node = new PriorityNode <KoColor>();
    node->data = data;
    node->key = m_key++;
    m_priorityList->append(node);

    int pos = guiInsertPos(data);
    pos >= m_guiList.size() ? m_guiList.append(node)
                            : m_guiList.insert(pos, node);
    node = 0;
}

void KisColorDataList::append(const KoColor& data)
{
    int pos = findPos(data);
    if (pos > -1) updateKey(pos);
    else appendNew(data);
}

void KisColorDataList::removeLeastUsed()
{
    Q_ASSERT_X(size() >= 0, "KisColorDataList::removeLeastUsed", "index out of bound");
    if (size() <= 0) return;

    int pos = findPos(m_priorityList->valueAt(0));
    m_guiList.removeAt(pos);
    m_priorityList->remove(0);
}

const KoColor& KisColorDataList::guiColor(int pos)
{
    Q_ASSERT_X(pos < size(), "KisColorDataList::guiColor", "index out of bound");
    Q_ASSERT_X(pos >= 0, "KisColorDataList::guiColor", "negative index");

    return m_guiList.at(pos)->data;
}

void KisColorDataList::printGuiList()
{
    qDebug() << "Printing guiList: ";
    QColor* color = new QColor();
    for (int pos = 0; pos < size() ; pos++)
    {
        m_guiList.at(pos)->data.toQColor(color);
        qDebug() << "pos: " << pos << " | data " << *color;
    }
}

int KisColorDataList::guiInsertPos(const KoColor& color)
{
    int low = 0, high = size() - 1, mid = (low + high)/2;
    while (low < high)
    {
        hsvComparison (color, m_guiList[mid]->data) == -1 ? high = mid
                                  : low = mid + 1;
        mid = (low + high)/2;
    }

    if (m_guiList.size() > 0)
    {
        if (hsvComparison (color, m_guiList[mid]->data) == 1) ++mid;
    }
    return mid;
}

int KisColorDataList::hsvComparison(const KoColor& c1, const KoColor& c2)
{
    QColor qc1 = c1.toQColor();
    QColor qc2 = c2.toQColor();

    if (qc1.hue() < qc2.hue()) return -1;
    if (qc1.hue() > qc2.hue()) return 1;

    // hue is the same, ok let's compare saturation
    if (qc1.saturation() < qc2.saturation()) return -1;
    if (qc1.saturation() > qc2.saturation()) return 1;

    // oh, also saturation is same?
    if (qc1.value() < qc2.value()) return -1;
    if (qc1.value() > qc2.value()) return 1;

    // user selected two similar colors
    return 0;
}

int KisColorDataList::findPos (const KoColor& color)
{

    int low = 0, high = size(), mid = 0;
    while (low < high)
    {
        mid = (low + high)/2;
        if (hsvComparison(color,m_guiList.at(mid)->data) == 0) return mid;
        else if (hsvComparison(color,m_guiList.at(mid)->data) < 0) high = mid;
        else low = mid + 1;
    }

    return -1;
}

void KisColorDataList::updateKey (int guiPos)
{
    if (m_guiList.at(guiPos)->key == m_key-1) return;
    m_priorityList->changeKey(m_guiList.at(guiPos)->pos, m_key++);
}
