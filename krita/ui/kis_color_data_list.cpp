#include "kis_color_data_list.h"

#ifndef _MSC_EXTENSIONS
const int KisColorDataList::MAX_RECENT_COLOR;
#endif

void KisColorDataList::appendNew(const QColor& data)
{
    if (size() >= KisColorDataList::MAX_RECENT_COLOR) removeLeastUsed();

    PriorityNode<QColor> * node;
    node = new PriorityNode <QColor>();
    node->data = data;
    node->key = m_key++;
    m_priorityList.append(node);

    int pos = guiInsertPos(data);
    pos >= m_guiList.size() ? m_guiList.append(node)
                            : m_guiList.insert(pos, node);
    node = 0;
}

void KisColorDataList::append(const QColor& data)
{
    int pos = findPos(data);
    if (pos > -1) updateKey(pos);
    else appendNew(data);
}

void KisColorDataList::removeLeastUsed()
{
    Q_ASSERT_X(size() >= 0, "KisColorDataList::removeLeastUsed", "index out of bound");
    if (size() <= 0) return;

    int pos = findPos(m_priorityList.valueAt(0));
    m_guiList.removeAt(pos);
    m_priorityList.remove(0);
}

const QColor& KisColorDataList::guiColor(int pos)
{
    Q_ASSERT_X(pos < size(), "KisColorDataList::guiColor", "index out of bound");
    Q_ASSERT_X(pos >= 0, "KisColorDataList::guiColor", "negative index");

    return m_guiList.at(pos)->data;
}

void KisColorDataList::printGuiList()
{
    qDebug() << "Printing guiList: ";
    for (int pos = 0; pos < size() ; pos++)
    {
        qDebug() << "pos: " << pos << " | data " << m_guiList.at(pos)->data;
    }
}

int KisColorDataList::guiInsertPos(const QColor& color)
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

int KisColorDataList::hsvComparison(const QColor& c1, const QColor& c2)
{
    if (c1.hue() < c2.hue()) return -1;
    if (c1.hue() > c2.hue()) return 1;

    // hue is the same, ok let's compare saturation
    if (c1.saturation() < c2.saturation()) return -1;
    if (c1.saturation() > c2.saturation()) return 1;

    // oh, also saturation is same?
    if (c1.value() < c2.value()) return -1;
    if (c1.value() > c2.value()) return 1;

    // user selected two similar colors
    return 0;
}

int KisColorDataList::findPos (const QColor& color)
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
