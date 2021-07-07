/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_MEMORY_STATISTICS_SERVER_H
#define __KIS_MEMORY_STATISTICS_SERVER_H

#include <QtGlobal>
#include <QObject>
#include <QScopedPointer>

#include "kritaimage_export.h"
#include "kis_types.h"


class KRITAIMAGE_EXPORT KisMemoryStatisticsServer : public QObject
{
    Q_OBJECT
public:
    struct Statistics
    {
        Statistics()
            : imageSize(0),
              layersSize(0),
              projectionsSize(0),
              lodSize(0),

              totalMemorySize(0),
              realMemorySize(0),
              historicalMemorySize(0),
              poolSize(0),

              swapSize(0),

              totalMemoryLimit(0),
              tilesHardLimit(0),
              tilesSoftLimit(0),
              tilesPoolLimit(0)
        {
        }

        qint64 imageSize;
        qint64 layersSize;
        qint64 projectionsSize;
        qint64 lodSize;

        qint64 totalMemorySize;
        qint64 realMemorySize;
        qint64 historicalMemorySize;
        qint64 poolSize;

        qint64 swapSize;

        qint64 totalMemoryLimit;
        qint64 tilesHardLimit;
        qint64 tilesSoftLimit;
        qint64 tilesPoolLimit;
    };



public:
    KisMemoryStatisticsServer();
    ~KisMemoryStatisticsServer() override;
    static KisMemoryStatisticsServer* instance();

    Statistics fetchMemoryStatistics(KisImageSP image) const;

public Q_SLOTS:
    void notifyImageChanged();
    void tryForceUpdateMemoryStatisticsWhileIdle();

Q_SIGNALS:
    void sigUpdateMemoryStatistics();


private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_MEMORY_STATISTICS_SERVER_H */
