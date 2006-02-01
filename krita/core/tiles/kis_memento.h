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

#include <ksharedptr.h>

class KisTile;
class KisTiledDataManager;

class KisMemento;
typedef KSharedPtr<KisMemento> KisMementoSP;

class KisMemento : public KShared
{
public:
    KisMemento(Q_UINT32 pixelSize);
    ~KisMemento();
/*
    // For consolidating transactions
    virtual KisTransaction &operator+=(const KisTransaction &) = 0;
    // For consolidating transactions
    virtual KisTransaction &operator+(const KisTransaction &,
                  const KisTransaction &) = 0;
*/
    void extent(Q_INT32 &x, Q_INT32 &y, Q_INT32 &w, Q_INT32 &h) const;

    // For debugging use
    bool valid() const { return m_valid; }
    void setInvalid() { m_valid = false; }

private:
    friend class KisTiledDataManager;
    KisTiledDataManager *originator;
    KisTile **m_hashTable;
    Q_UINT32 m_numTiles;
    KisTile **m_redoHashTable;
    struct DeletedTile {Q_INT32 col; Q_INT32 row; DeletedTile *next;};
    DeletedTile *m_delTilesTable;
    Q_UINT8 *m_defPixel;
    Q_UINT8 *m_redoDefPixel;
    void deleteAll(DeletedTile *deletedtile);
    void deleteAll(KisTile *tile);

    bool m_valid;
};

#endif // KIS_MEMENTO_H_
