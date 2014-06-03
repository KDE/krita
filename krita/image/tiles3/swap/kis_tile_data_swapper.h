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

#include "krita_export.h"


class KisTileDataStore;
class KisTileData;

class KRITAIMAGE_EXPORT KisTileDataSwapper : public QThread
{
    Q_OBJECT

public:

    KisTileDataSwapper(KisTileDataStore *store);
    virtual ~KisTileDataSwapper();

    void kick();
    void terminateSwapper();
    void checkFreeMemory();

    void testingRereadConfig();

private:
    void waitForWork();
    void run();

    void doJob();
    template<class strategy> qint64 pass(qint64 needToFreeMetric);

private:
    static const qint32 TIMEOUT;
    static const qint32 DELAY;

private:
    struct Private;
    Private * const m_d;
};



#endif /* KIS_TILE_DATA_SWAPPER_H_ */

