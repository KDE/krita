/*
 *  Copyright (c) 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
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

#include "Throttle.h"

#include <QAction>
#include <QThread>
#include <QQmlContext>
#include <QQmlEngine>

#include <klocalizedstring.h>
#include <kactioncollection.h>

#include <kis_icon.h>
#include <KoCanvasBase.h>
#include <KisViewManager.h>
#include <kis_canvas2.h>
#include <KisMainWindow.h>

ThreadManager::ThreadManager(QObject *parent)
    : QObject(parent)
{}

ThreadManager::~ThreadManager()
{
}

void ThreadManager::setThreadCount(int threadCount)
{
    threadCount = qMin(maxThreadCount(), int(qreal(threadCount) * (maxThreadCount() / 100.0)));

    if (m_threadCount != threadCount) {
        m_threadCount = threadCount;
        emit threadCountChanged();
        // XXX: set the threadcount globally in Krita.
    }
}

int ThreadManager::threadCount() const
{
    return m_threadCount;
}

int ThreadManager::maxThreadCount() const
{
    return QThread::idealThreadCount();
}


Throttle::Throttle(QWidget *parent)
    : QQuickWidget(parent)
{
    m_threadManager = new ThreadManager();
    // In % of available cores...
    engine()->rootContext()->setContextProperty("ThreadManager", m_threadManager);
    m_threadManager->setThreadCount(100);
    setSource(QUrl("qrc:/slider.qml"));
}

Throttle::~Throttle()
{
    setSource(QUrl());
}
