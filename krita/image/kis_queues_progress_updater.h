/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_QUEUES_PROGRESS_UPDATER_H
#define __KIS_QUEUES_PROGRESS_UPDATER_H

#include <QObject>
#include "krita_export.h"

class KoProgressProxy;


class KRITAIMAGE_EXPORT KisQueuesProgressUpdater : public QObject
{
    Q_OBJECT

public:
    KisQueuesProgressUpdater(KoProgressProxy *progressProxy);
    ~KisQueuesProgressUpdater();

    void notifyJobDone(int sizeMetric);
    void updateProgress(int queueSizeMetric, const QString &jobName);

private slots:
    void updateProxy();

private:
    struct Private;
    Private * const m_d;
};

#endif /* __KIS_QUEUES_PROGRESS_UPDATER_H */
