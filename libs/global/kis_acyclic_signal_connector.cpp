/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_acyclic_signal_connector.h"

#include "kis_debug.h"


KisAcyclicSignalConnector::KisAcyclicSignalConnector(QObject *parent)
    : QObject(parent),
      m_signalsBlocked(0)
{
}

KisAcyclicSignalConnector::~KisAcyclicSignalConnector()
{
}

void KisAcyclicSignalConnector::connectForwardDouble(QObject *sender, const char *signal,
                                                  QObject *receiver, const char *method)
{

    connect(sender, signal, this, SLOT(forwardSlotDouble(double)));
    connect(this, SIGNAL(forwardSignalDouble(double)), receiver, method);
}

void KisAcyclicSignalConnector::connectBackwardDouble(QObject *sender, const char *signal,
                                                   QObject *receiver, const char *method)
{

    connect(sender, signal, this, SLOT(backwardSlotDouble(double)));
    connect(this, SIGNAL(backwardSignalDouble(double)), receiver, method);
}

void KisAcyclicSignalConnector::connectForwardInt(QObject *sender, const char *signal,
                                                  QObject *receiver, const char *method)
{

    connect(sender, signal, this, SLOT(forwardSlotInt(int)));
    connect(this, SIGNAL(forwardSignalInt(int)), receiver, method);
}

void KisAcyclicSignalConnector::connectBackwardInt(QObject *sender, const char *signal,
                                                   QObject *receiver, const char *method)
{

    connect(sender, signal, this, SLOT(backwardSlotInt(int)));
    connect(this, SIGNAL(backwardSignalInt(int)), receiver, method);
}

void KisAcyclicSignalConnector::connectForwardBool(QObject *sender, const char *signal,
                                                  QObject *receiver, const char *method)
{

    connect(sender, signal, this, SLOT(forwardSlotBool(bool)));
    connect(this, SIGNAL(forwardSignalBool(bool)), receiver, method);
}

void KisAcyclicSignalConnector::connectBackwardBool(QObject *sender, const char *signal,
                                                   QObject *receiver, const char *method)
{

    connect(sender, signal, this, SLOT(backwardSlotBool(bool)));
    connect(this, SIGNAL(backwardSignalBool(bool)), receiver, method);
}

void KisAcyclicSignalConnector::connectForwardVoid(QObject *sender, const char *signal,
                                                  QObject *receiver, const char *method)
{

    connect(sender, signal, this, SLOT(forwardSlotVoid()));
    connect(this, SIGNAL(forwardSignalVoid()), receiver, method);
}

void KisAcyclicSignalConnector::connectBackwardVoid(QObject *sender, const char *signal,
                                                 QObject *receiver, const char *method)
{

    connect(sender, signal, this, SLOT(backwardSlotVoid()));
    connect(this, SIGNAL(backwardSignalVoid()), receiver, method);
}

void KisAcyclicSignalConnector::connectForwardVariant(QObject *sender, const char *signal,
                                                  QObject *receiver, const char *method)
{

    connect(sender, signal, this, SLOT(forwardSlotVariant(QVariant)));
    connect(this, SIGNAL(forwardSignalVariant(QVariant)), receiver, method);
}

void KisAcyclicSignalConnector::connectBackwardVariant(QObject *sender, const char *signal,
                                                       QObject *receiver, const char *method)
{
    connect(sender, signal, this, SLOT(backwardSlotVariant(QVariant)));
    connect(this, SIGNAL(backwardSignalVariant(QVariant)), receiver, method);
}

void KisAcyclicSignalConnector::connectForwardResourcePair(QObject *sender, const char *signal, QObject *receiver, const char *method)
{
    connect(sender, signal, this, SLOT(forwardSlotResourcePair(int,QVariant)));
    connect(this, SIGNAL(forwardSignalResourcePair(int,QVariant)), receiver, method);
}

void KisAcyclicSignalConnector::connectBackwardResourcePair(QObject *sender, const char *signal, QObject *receiver, const char *method)
{
    connect(sender, signal, this, SLOT(backwardSlotResourcePair(int,QVariant)));
    connect(this, SIGNAL(backwardSignalResourcePair(int,QVariant)), receiver, method);
}

void KisAcyclicSignalConnector::connectForwardKoColor(QObject *sender, const char *signal, QObject *receiver, const char *method)
{
    connect(sender, signal, this, SLOT(forwardSlotKoColor(KoColor)));
    connect(this, SIGNAL(forwardSignalKoColor(KoColor)), receiver, method);
}

void KisAcyclicSignalConnector::connectBackwardKoColor(QObject *sender, const char *signal, QObject *receiver, const char *method)
{
    connect(sender, signal, this, SLOT(backwardSlotKoColor(KoColor)));
    connect(this, SIGNAL(backwardSignalKoColor(KoColor)), receiver, method);
}

void KisAcyclicSignalConnector::lock()
{
    if (m_parentConnector) {
        m_parentConnector->lock();
    } else {
        coordinatedLock();

        Q_FOREACH(QPointer<KisAcyclicSignalConnector> conn, m_coordinatedConnectors) {
            if (!conn) continue;
            conn->coordinatedLock();
        }
    }
}

void KisAcyclicSignalConnector::unlock()
{
    if (m_parentConnector) {
        m_parentConnector->unlock();
    } else {
        Q_FOREACH(QPointer<KisAcyclicSignalConnector> conn, m_coordinatedConnectors) {
            if (!conn) continue;
            conn->coordinatedUnlock();
        }

        coordinatedUnlock();
    }
}

void KisAcyclicSignalConnector::coordinatedLock()
{
    m_signalsBlocked++;
}

void KisAcyclicSignalConnector::coordinatedUnlock()
{
    m_signalsBlocked--;
}

KisAcyclicSignalConnector *KisAcyclicSignalConnector::createCoordinatedConnector()
{
    KisAcyclicSignalConnector *conn = new KisAcyclicSignalConnector(this);
    conn->m_parentConnector = this;
    m_coordinatedConnectors.append(conn);
    return conn;
}

void KisAcyclicSignalConnector::forwardSlotDouble(double value)
{
    if (m_signalsBlocked) return;

    lock();
    emit forwardSignalDouble(value);
    unlock();
}

void KisAcyclicSignalConnector::backwardSlotDouble(double value)
{
    if (m_signalsBlocked) return;

    lock();
    emit backwardSignalDouble(value);
    unlock();
}

void KisAcyclicSignalConnector::forwardSlotInt(int value)
{
    if (m_signalsBlocked) return;

    lock();
    emit forwardSignalInt(value);
    unlock();
}

void KisAcyclicSignalConnector::backwardSlotInt(int value)
{
    if (m_signalsBlocked) return;

    lock();
    emit backwardSignalInt(value);
    unlock();
}

void KisAcyclicSignalConnector::forwardSlotBool(bool value)
{
    if (m_signalsBlocked) return;

    lock();
    emit forwardSignalBool(value);
    unlock();
}

void KisAcyclicSignalConnector::backwardSlotBool(bool value)
{
    if (m_signalsBlocked) return;

    lock();
    emit backwardSignalBool(value);
    unlock();
}

void KisAcyclicSignalConnector::forwardSlotVoid()
{
    if (m_signalsBlocked) return;

    lock();
    emit forwardSignalVoid();
    unlock();
}

void KisAcyclicSignalConnector::backwardSlotVoid()
{
    if (m_signalsBlocked) return;

    lock();
    emit backwardSignalVoid();
    unlock();
}

void KisAcyclicSignalConnector::forwardSlotVariant(const QVariant &value)
{
    if (m_signalsBlocked) return;

    lock();
    emit forwardSignalVariant(value);
    unlock();
}

void KisAcyclicSignalConnector::backwardSlotVariant(const QVariant &value)
{
    if (m_signalsBlocked) return;

    lock();
    emit backwardSignalVariant(value);
    unlock();
}

void KisAcyclicSignalConnector::forwardSlotResourcePair(int key, const QVariant &resource)
{
    if (m_signalsBlocked) return;

    lock();
    emit forwardSignalResourcePair(key, resource);
    unlock();
}

void KisAcyclicSignalConnector::backwardSlotResourcePair(int key, const QVariant &resource)
{
    if (m_signalsBlocked) return;

    lock();
    emit backwardSignalResourcePair(key, resource);
    unlock();
}

void KisAcyclicSignalConnector::forwardSlotKoColor(const KoColor &value)
{
    if (m_signalsBlocked) return;

    lock();
    emit forwardSignalKoColor(value);
    unlock();
}

void KisAcyclicSignalConnector::backwardSlotKoColor(const KoColor &value)
{
    if (m_signalsBlocked) return;

    lock();
    emit backwardSignalKoColor(value);
    unlock();
}
