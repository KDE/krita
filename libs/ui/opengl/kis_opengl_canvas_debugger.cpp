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

#include "kis_opengl_canvas_debugger.h"

#include <QGlobalStatic>

#include <QElapsedTimer>
#include <QDebug>

#include "kis_config.h"
#include <kis_config_notifier.h>

struct KisOpenglCanvasDebugger::Private
{

    Private()
        : fpsCounter(0),
          fpsSum(0),
          syncFlaggedCounter(0),
          syncFlaggedSum(0),
          isEnabled(true) {}

    QElapsedTimer time;

    int fpsCounter;
    int fpsSum;

    int syncFlaggedCounter;
    int syncFlaggedSum;

    bool isEnabled;
};

Q_GLOBAL_STATIC(KisOpenglCanvasDebugger, s_instance)

KisOpenglCanvasDebugger::KisOpenglCanvasDebugger()
    : m_d(new Private)
{
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotConfigChanged()));
    slotConfigChanged();
}

KisOpenglCanvasDebugger::~KisOpenglCanvasDebugger()
{
}

KisOpenglCanvasDebugger*
KisOpenglCanvasDebugger::instance()
{
    return s_instance;
}

bool KisOpenglCanvasDebugger::showFpsOnCanvas() const
{
    return m_d->isEnabled;
}

qreal KisOpenglCanvasDebugger::accumulatedFps()
{
    qreal value = 0;

    if (m_d->fpsSum > 0) {
        value = qreal(m_d->fpsCounter) / m_d->fpsSum * 1000.0;
    }

    return value;
}

void KisOpenglCanvasDebugger::slotConfigChanged()
{
    KisConfig cfg(true);
    m_d->isEnabled = cfg.enableOpenGLFramerateLogging();

    if (m_d->isEnabled) {
        m_d->time.start();
    }
}

void KisOpenglCanvasDebugger::nofityPaintRequested()
{
    if (!m_d->isEnabled) return;

    m_d->fpsSum += m_d->time.restart();
    m_d->fpsCounter++;

    if (m_d->fpsCounter > 100 && m_d->fpsSum > 0) {
        qDebug() << "Requested FPS:" << qreal(m_d->fpsCounter) / m_d->fpsSum * 1000.0;
        m_d->fpsSum = 0;
        m_d->fpsCounter = 0;
    }
}

void KisOpenglCanvasDebugger::nofitySyncStatus(bool isBusy)
{
    if (!m_d->isEnabled) return;

    m_d->syncFlaggedSum += isBusy;
    m_d->syncFlaggedCounter++;

    if (m_d->syncFlaggedCounter > 500 && m_d->syncFlaggedSum > 0) {
        qDebug() << "glSync effectiveness:" << qreal(m_d->syncFlaggedSum) / m_d->syncFlaggedCounter;
        m_d->syncFlaggedSum = 0;
        m_d->syncFlaggedCounter = 0;
    }
}
