/*
 *  Copyright (c) 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef THROTTLE_H
#define THROTTLE_H

#include <QtQuickWidgets/QQuickWidget>


class KisCanvas2;
class KisSignalCompressor;

class ThreadManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(int threadCount READ threadCount WRITE setThreadCount NOTIFY threadCountChanged)
    Q_PROPERTY(int maxThreadCount READ maxThreadCount)
public:
    ThreadManager(QObject *parent = 0);
    ~ThreadManager() override;

    void setThreadCount(int threadCount);
    int threadCount() const;
    int maxThreadCount() const;

private Q_SLOTS:
    void slotDoUpdateConfig();

Q_SIGNALS:
    void threadCountChanged();
private:
    int m_threadCount = 0;
    KisSignalCompressor *m_configUpdateCompressor;
};

class Throttle : public QQuickWidget  {
    Q_OBJECT
public:
    Throttle(QWidget *parent);
    ~Throttle() override;

private:
    ThreadManager *m_threadManager {0};
};


#endif

