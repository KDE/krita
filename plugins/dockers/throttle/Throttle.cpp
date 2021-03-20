/*
 *  SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#include "Throttle.h"

#include <QAction>
#include <QThread>
#include <QQmlContext>
#include <QQmlEngine>
#include <qmath.h>

#include <klocalizedstring.h>
#include <kactioncollection.h>

#include <kis_image_config.h>
#include <kis_icon.h>
#include <KoCanvasBase.h>
#include <KisViewManager.h>
#include <kis_canvas2.h>
#include <KisMainWindow.h>
#include "kis_signal_compressor.h"

#include "KisUpdateSchedulerConfigNotifier.h"

#include <QVersionNumber>

namespace
{

bool shouldSetAcceptTouchEvents()
{
    // See https://bugreports.qt.io/browse/QTBUG-66718
    static QVersionNumber qtVersion = QVersionNumber::fromString(qVersion());
    static bool retval = qtVersion > QVersionNumber(5, 9, 3) && qtVersion.normalized() != QVersionNumber(5, 10);
    return retval;
}

} // namespace


ThreadManager::ThreadManager(QObject *parent)
    : QObject(parent),
      m_configUpdateCompressor(new KisSignalCompressor(500, KisSignalCompressor::POSTPONE, this))
{
    connect(m_configUpdateCompressor, SIGNAL(timeout()), SLOT(slotDoUpdateConfig()));
}

ThreadManager::~ThreadManager()
{
}

void ThreadManager::setThreadCount(int threadCount)
{
    threadCount = 1 + qreal(threadCount) * (maxThreadCount() - 1) / 100.0;

    if (m_threadCount != threadCount) {
        m_threadCount = threadCount;
        m_configUpdateCompressor->start();
        emit threadCountChanged();
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

void ThreadManager::slotDoUpdateConfig()
{
    KisImageConfig cfg;
    cfg.setMaxNumberOfThreads(m_threadCount);
    cfg.setFrameRenderingClones(qCeil(m_threadCount * 0.5));
    KisUpdateSchedulerConfigNotifier::instance()->notifyConfigChanged();
}


Throttle::Throttle(QWidget *parent)
    : QQuickWidget(parent)
{
    if (shouldSetAcceptTouchEvents()) {
        setAttribute(Qt::WA_AcceptTouchEvents);
    }
    m_threadManager = new ThreadManager();
    // In % of available cores...
    engine()->rootContext()->setContextProperty("ThreadManager", m_threadManager);
    m_threadManager->setThreadCount(100);
    setSource(QUrl("qrc:/slider.qml"));
    setResizeMode(SizeRootObjectToView);
}

Throttle::~Throttle()
{
    setSource(QUrl());
}
