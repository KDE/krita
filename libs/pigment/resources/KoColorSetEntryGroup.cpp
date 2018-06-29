#include "KoColorSetEntryGroup.h"

KoColorSetEntry KoColorSetEntryGroup::NULLENTRY = KoColorSetEntry();

KoColorSetEntryGroup::KoColorSetEntryGroup()
    : m_nColors(0)
    , m_nLastRowEntries(0)
{
    m_nRows[0] = 0;
}

void KoColorSetEntryGroup::setEntry(KoColorSetEntry e)
{
    Q_ASSERT(e.x() <= m_colors.size() && e.x() >= 0);
    if (e.y() >= m_nRows.lastKey() - 1) {
        m_nRows[e.y() + 1] = 0;
        if (!m_colors[e.x()].contains(e.y())) {
            m_nLastRowEntries++;
        }
    }
    m_colors[e.x()][e.y()] = e;
    m_nColors++;
}

void KoColorSetEntryGroup::removeEntry(int x, int y)
{
    Q_ASSERT(x <= m_colors.size() && x >= 0);
    m_nColors -= m_colors[x].remove(y); // remove returns 1 if target found, else 0
    if (y == m_nRows.lastKey() - 1) {
        m_nLastRowEntries--;
    }
    if (m_nLastRowEntries == 0) {
         m_nRows.remove(m_nRows.lastKey());
    }
}

void KoColorSetEntryGroup::setNColumns(int nColumns)
{
    m_colors.resize(nColumns);
}

KoColorSetEntry KoColorSetEntryGroup::getEntry (int x, int y, bool &success) const
{
    Q_ASSERT(x >= 0 && x <= m_colors.size());
    if (m_colors[x].contains(y)) {
        success = true;
        return m_colors[x][y];
    } else {
        success = false;
        return NULLENTRY;
    }
}
