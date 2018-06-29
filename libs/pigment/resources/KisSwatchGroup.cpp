#include "KisSwatchGroup.h"

KisSwatch KisSwatchGroup::NULLENTRY = KisSwatch();
quint32 KisSwatchGroup::DEFAULT_N_COLUMN = 16;

KisSwatchGroup::KisSwatchGroup()
    : m_nColors(0)
    , m_colorMatrix(DEFAULT_N_COLUMN)
    , m_nLastRowEntries(0)
{ }

void KisSwatchGroup::setEntry(const KisSwatch &e, int x, int y)
{
    Q_ASSERT(x < m_colorMatrix.size() && x >= 0 && y >= 0);
    if (!checkEntry(x, y)) {
        m_nColors++;
    }
    m_colorMatrix[x][y] = e;
}

bool KisSwatchGroup::checkEntry(int x, int y) const
{
    if (x >= m_colorMatrix.size() || x < 0) {
        return false;
    }
    if (!m_colorMatrix[x].contains(y)) {
        return false;
    }
    return true;
}

bool KisSwatchGroup::removeEntry(int x, int y)
{
    if (m_nColors == 0) {
        return false;
    }

    if (x >= m_colorMatrix.size() || x < 0) {
        return false;
    }

    // QMap::remove returns 1 if key found else 0
    if (m_colorMatrix[x].remove(y)) {
        m_nColors -= 1;
        return true;
    } else {
        return false;
    }
}

void KisSwatchGroup::setNColumns(int nColumns)
{
    m_colorMatrix.resize(nColumns);
}

KisSwatch KisSwatchGroup::getEntry(int x, int y) const
{
    Q_ASSERT(checkEntry(x, y));
    return m_colorMatrix[x][y];
}

int KisSwatchGroup::nRows() const
{
    int res = 0;
    Q_FOREACH(const Column &c, m_colorMatrix) {
        if (c.empty()) {
            continue;
        }
        res = res >= c.lastKey() ? res : c.lastKey();
    }
    return res + 1;
}
