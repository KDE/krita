/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_TILE_DATA_SWAPPER_H_
#define KIS_TILE_DATA_SWAPPER_H_

#include <QObject>
#include <QThread>

#include "kritaimage_export.h"


class KisTileDataStore;
class KisTileData;

class KRITAIMAGE_EXPORT KisTileDataSwapper : public QThread
{
    Q_OBJECT

public:

    KisTileDataSwapper(KisTileDataStore *store);
    ~KisTileDataSwapper() override;

    void kick();
    void terminateSwapper();
    void checkFreeMemory();

    void testingRereadConfig();

private:
    void waitForWork();
    void run() override;

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

