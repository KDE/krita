/*
 *  Copyright (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
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


#ifndef KIS_TILE_DATA_POOLER_H_
#define KIS_TILE_DATA_POOLER_H_

#include <QObject>
#include <QThread>
#include <QSemaphore>

class KisTileDataStore;
class KisTileData;


class KisTileDataPooler : public QThread
{
    Q_OBJECT
public:
    KisTileDataPooler(KisTileDataStore *store);
    virtual ~KisTileDataPooler();

    void kick();
    void terminatePooler();

protected:
    static const qint32 MAX_NUM_CLONES;
    static const qint32 MAX_TIMEOUT;
    static const qint32 MIN_TIMEOUT;
    static const qint32 TIMEOUT_FACTOR;

    void waitForWork();
    qint32 numClonesNeeded(KisTileData *td) const;
    void cloneTileData(KisTileData *td, qint32 numClones) const;
    void run();
 private:
    bool interestingTileData(KisTileData* td);

protected:
    QSemaphore m_semaphore;
    QAtomicInt m_shouldExitFlag;
    KisTileDataStore *m_store;
    qint32 m_timeout;
    bool m_lastCycleHadWork;
};



#endif /* KIS_TILE_DATA_POOLER_H_ */

