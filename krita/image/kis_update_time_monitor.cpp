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

#include "kis_update_time_monitor.h"

#include <kglobal.h>
#include <QHash>
#include <QSet>
#include <QMutex>
#include <QMutexLocker>
#include <QPointF>
#include <QRect>
#include <QRegion>
#include <QFile>
#include <QDir>

#if QT_VERSION >= 0x040700
#include <QElapsedTimer>
#endif

#include <QFileInfo>

#include "kis_debug.h"
#include "kis_global.h"
#include "kis_image_config.h"


#include "kis_paintop_preset.h"


struct StrokeTicket
{
    StrokeTicket()
        : m_jobTime(0)
        , m_updateTime(0) {}

    QRegion dirtyRegion;

    void start() {
#if QT_VERSION >= 0x040700
        m_timer.start();
#endif
    }

    void jobCompleted() {
#if QT_VERSION >= 0x040700
        m_jobTime = m_timer.restart();
#endif
    }

    void updateCompleted() {
#if QT_VERSION >= 0x040700
        m_updateTime = m_timer.restart();
#endif
    }

    qint64 jobTime() const {
        return m_jobTime;
    }

    qint64 updateTime() const {
        return m_updateTime;
    }

private:
#if QT_VERSION >= 0x040700
    QElapsedTimer m_timer;
#endif
    qint64 m_jobTime;
    qint64 m_updateTime;
};

struct KisUpdateTimeMonitor::Private
{
    Private()
        : jobsTime(0),
          responseTime(0),
          numTickets(0),
          numUpdates(0),
          mousePath(0.0),
          loggingEnabled(false)
    {
        loggingEnabled = KisImageConfig().enablePerfLog();
    }

    QHash<void*, StrokeTicket*> preliminaryTickets;
    QSet<StrokeTicket*> finishedTickets;

    qint64 jobsTime;
    qint64 responseTime;
    qint32 numTickets;
    qint32 numUpdates;
    QMutex mutex;

    qreal mousePath;
    QPointF lastMousePos;
#if QT_VERSION >= 0x040700
    QElapsedTimer strokeTime;
#endif
    KisPaintOpPresetSP preset;

    bool loggingEnabled;
};

KisUpdateTimeMonitor::KisUpdateTimeMonitor()
    : m_d(new Private)
{
#if QT_VERSION >= 0x040700
    if (m_d->loggingEnabled) {
        QDir dir;
        if (dir.exists("log")) {
            dir.remove("log");
        }
        dir.mkdir("log");
    }
#endif
}

KisUpdateTimeMonitor::~KisUpdateTimeMonitor()
{
    delete m_d;
}

KisUpdateTimeMonitor* KisUpdateTimeMonitor::instance()
{
    K_GLOBAL_STATIC(KisUpdateTimeMonitor, s_instance);
    return s_instance;
}

void KisUpdateTimeMonitor::startStrokeMeasure()
{
#if QT_VERSION >= 0x040700
    if (!m_d->loggingEnabled) return;

    QMutexLocker locker(&m_d->mutex);

    m_d->jobsTime = 0;
    m_d->responseTime = 0;
    m_d->numTickets = 0;
    m_d->numUpdates = 0;
    m_d->mousePath = 0;

    m_d->lastMousePos = QPointF();
    m_d->preset = 0;
    m_d->strokeTime.start();
#endif
}

void KisUpdateTimeMonitor::endStrokeMeasure()
{
#if QT_VERSION >= 0x040700
    if (!m_d->loggingEnabled) return;

    QMutexLocker locker(&m_d->mutex);

    if(m_d->numTickets) {
        printValues();
    }
#endif
}

void KisUpdateTimeMonitor::reportPaintOpPreset(KisPaintOpPresetSP preset)
{
#if QT_VERSION >= 0x040700
    if (!m_d->loggingEnabled) return;

    m_d->preset = preset;
#endif
}

void KisUpdateTimeMonitor::reportMouseMove(const QPointF &pos)
{
#if QT_VERSION >= 0x040700
    if (!m_d->loggingEnabled) return;

    QMutexLocker locker(&m_d->mutex);

    if (!m_d->lastMousePos.isNull()) {
        qreal distance = kisDistance(m_d->lastMousePos, pos);
        m_d->mousePath += distance;
    }

    m_d->lastMousePos = pos;
#endif
}

void KisUpdateTimeMonitor::printValues()
{
#if QT_VERSION >= 0x040700
    qint64 strokeTime = m_d->strokeTime.elapsed();
    qreal responseTime = qreal(m_d->responseTime) / m_d->numTickets;
    qreal nonUpdateTime = qreal(m_d->jobsTime) / m_d->numTickets;
    qreal jobsPerUpdate = qreal(m_d->numTickets) / m_d->numUpdates;
    qreal mouseSpeed = qreal(m_d->mousePath) / strokeTime;

    QString prefix;

    if (m_d->preset) {
        KisPaintOpPresetSP preset = m_d->preset->clone();
        prefix = QString("%1.").arg(preset->name());
        preset->setFilename(QString("log/%1.kpp").arg(preset->name()));
        preset->save();
    }

    QFile logFile(QString("log/%1stroke.rdata").arg(prefix));
    logFile.open(QIODevice::Append);
    QTextStream stream(&logFile);
    stream << mouseSpeed << "\t"
           << jobsPerUpdate << "\t"
           << nonUpdateTime << "\t"
           << responseTime << "\n";
    logFile.close();
#endif
}

void KisUpdateTimeMonitor::reportJobStarted(void *key)
{
#if QT_VERSION >= 0x040700
    if (!m_d->loggingEnabled) return;

    QMutexLocker locker(&m_d->mutex);

    StrokeTicket *ticket = new StrokeTicket();
    ticket->start();

    m_d->preliminaryTickets.insert(key, ticket);
#endif
}

void KisUpdateTimeMonitor::reportJobFinished(void *key, const QVector<QRect> &rects)
{
#if QT_VERSION >= 0x040700
    if (!m_d->loggingEnabled) return;

    QMutexLocker locker(&m_d->mutex);

    StrokeTicket *ticket = m_d->preliminaryTickets.take(key);
    ticket->jobCompleted();

    foreach(const QRect &rect, rects) {
        ticket->dirtyRegion += rect;
    }
    m_d->finishedTickets.insert(ticket);
#endif
}

void KisUpdateTimeMonitor::reportUpdateFinished(const QRect &rect)
{
#if QT_VERSION >= 0x040700
    if (!m_d->loggingEnabled) return;

    QMutexLocker locker(&m_d->mutex);

    foreach(StrokeTicket *ticket, m_d->finishedTickets) {
        ticket->dirtyRegion -= rect;
        if(ticket->dirtyRegion.isEmpty()) {
            ticket->updateCompleted();
            m_d->jobsTime += ticket->jobTime();
            m_d->responseTime += ticket->jobTime() + ticket->updateTime();
            m_d->numTickets++;

            m_d->finishedTickets.remove(ticket);
            delete ticket;
        }
    }
    m_d->numUpdates++;
#endif
}
