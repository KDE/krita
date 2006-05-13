/*
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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

#ifndef KIS_MEMENTO_H_
#define KIS_MEMENTO_H_

#include <qglobal.h>
#include <QRect>

#include <ksharedptr.h>

class KisTile;
class KisTiledDataManager;

class KisMemento;
typedef KSharedPtr<KisMemento> KisMementoSP;

class KisMemento : public KShared
{
public:
    KisMemento(quint32 pixelSize);
    ~KisMemento();
/*
    // For consolidating transactions
    virtual KisTransaction &operator+=(const KisTransaction &) = 0;
    // For consolidating transactions
    virtual KisTransaction &operator+(const KisTransaction &,
                  const KisTransaction &) = 0;
*/
    void extent(qint32 &x, qint32 &y, qint32 &w, qint32 &h) const;
    QRect extent() const;

    bool containsTile(qint32 col, qint32 row, quint32 tileHash) const;

    // For debugging use
    bool valid() const { return m_valid; }
    void setInvalid() { m_valid = false; }

private:

    class DeletedTile {
    public:
        DeletedTile(qint32 col, qint32 row, const DeletedTile *next)
            : m_col(col),
              m_row(row),
              m_next(next)
        {
        }

        qint32 col() const { return m_col; }
        qint32 row() const { return m_row; }
        const DeletedTile *next() const { return m_next; }

    private:
        qint32 m_col;
        qint32 m_row;
        const DeletedTile *m_next;
    };

    class DeletedTileList {
    public:
        DeletedTileList()
            : m_firstDeletedTile(0)
        {
        }

        ~DeletedTileList();

        void addTile(qint32 col, qint32 row)
        {
            DeletedTile *d = new DeletedTile(col, row, m_firstDeletedTile);
            Q_CHECK_PTR(d);

            m_firstDeletedTile = d;
        }

        DeletedTile *firstTile() const
        {
            return m_firstDeletedTile;
        }

        void clear();

    private:
        DeletedTile *m_firstDeletedTile;
    };

    void addTileToDeleteOnRedo(qint32 col, qint32 row)
    {
        m_redoDelTilesList.addTile(col, row);
    }

    DeletedTile *tileListToDeleteOnRedo()
    {
        return m_redoDelTilesList.firstTile();
    }

    void clearTilesToDeleteOnRedo()
    {
        m_redoDelTilesList.clear();
    }

    void addTileToDeleteOnUndo(qint32 col, qint32 row)
    {
        m_undoDelTilesList.addTile(col, row);
    }

    DeletedTile *tileListToDeleteOnUndo()
    {
        return m_undoDelTilesList.firstTile();
    }

    void clearTilesToDeleteOnUndo()
    {
        m_undoDelTilesList.clear();
    }

    friend class KisTiledDataManager;
    KisTiledDataManager *originator;
    KisTile **m_hashTable;
    quint32 m_numTiles;
    KisTile **m_redoHashTable;
    DeletedTileList m_redoDelTilesList;
    DeletedTileList m_undoDelTilesList;
    quint8 *m_defPixel;
    quint8 *m_redoDefPixel;
    void deleteAll(KisTile *tile);

    bool m_valid;
};

#endif // KIS_MEMENTO_H_
