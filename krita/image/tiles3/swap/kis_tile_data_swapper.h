/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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
#ifndef KIS_TILE_DATA_SWAPPER_H_
#define KIS_TILE_DATA_SWAPPER_H_

#include <QObject>
#include <QThread>

class KisTileDataStore;
class KisTileData;


class KisTileDataSwapper : public QThread
{
    Q_OBJECT

public:

    KisTileDataSwapper(KisTileDataStore *store);
    virtual ~KisTileDataSwapper();

    void kick();
    void terminateSwapper();

    void checkFreeMemory();

private:
    void waitForWork();
    void run();

    void doJob();
    qint32 pass0(qint32 tilesToFree);
    qint32 pass1(qint32 tilesToFree);

private:
    static const qint32 TIMEOUT;
    static const qint32 DELAY;

private:
    class Private;
    Private * const m_d;
};



#endif /* KIS_TILE_DATA_SWAPPER_H_ */

