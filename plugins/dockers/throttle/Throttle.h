/*
 *  SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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

