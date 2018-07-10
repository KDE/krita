#ifndef KOCOLORSETENTRYGROUP_H
#define KOCOLORSETENTRYGROUP_H

#include "kritapigment_export.h"
#include <QVector>
#include <QHash>
#include <QMap> // Used to keep track of the last row. Qt doesn't provide a priority queue...
#include "KoColorSetEntry.h"

class KoColorSet;

class KRITAPIGMENT_EXPORT KoColorSetEntryGroup
{
    static KoColorSetEntry NULLENTRY;
public:
    typedef QHash<int, KoColorSetEntry> Column;

public:
    KoColorSetEntryGroup();
    int colorCount() const { return m_nColors; }
    int rowCount() const { return m_nRows.lastKey(); } // last is the largest
    void setEntry(KoColorSetEntry e);
    void setColumnCount(int nColumns);
    const QVector<Column> &colors() const { return m_colors; }
    void removeEntry(int x, int y);
    void clear() { m_colors.clear(); }
    KoColorSetEntry getEntry (int x, int y, bool &success) const;

private:
    QVector<Column> m_colors;
    int m_nColors;
    QMap<int, int> m_nRows;
    int m_nLastRowEntries;

friend class KoColorSet;
};

#endif // KOCOLORSETENTRYGROUP_H
