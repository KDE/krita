/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

    connect(sender, signal, this, SLOT(forwardSlotDouble(double)), Qt::UniqueConnection);
    connect(this, SIGNAL(forwardSignalDouble(double)), receiver, method, Qt::UniqueConnection);
}

void KisAcyclicSignalConnector::connectBackwardDouble(QObject *sender, const char *signal,
                                                   QObject *receiver, const char *method)
{

    connect(sender, signal, this, SLOT(backwardSlotDouble(double)), Qt::UniqueConnection);
    connect(this, SIGNAL(backwardSignalDouble(double)), receiver, method, Qt::UniqueConnection);
}

void KisAcyclicSignalConnector::connectForwardInt(QObject *sender, const char *signal,
                                                  QObject *receiver, const char *method)
{

    connect(sender, signal, this, SLOT(forwardSlotInt(int)), Qt::UniqueConnection);
    connect(this, SIGNAL(forwardSignalInt(int)), receiver, method, Qt::UniqueConnection);
}

void KisAcyclicSignalConnector::connectBackwardInt(QObject *sender, const char *signal,
                                                   QObject *receiver, const char *method)
{

    connect(sender, signal, this, SLOT(backwardSlotInt(int)), Qt::UniqueConnection);
    connect(this, SIGNAL(backwardSignalInt(int)), receiver, method, Qt::UniqueConnection);
}

void KisAcyclicSignalConnector::connectForwardBool(QObject *sender, const char *signal,
                                                  QObject *receiver, const char *method)
{

    connect(sender, signal, this, SLOT(forwardSlotBool(bool)), Qt::UniqueConnection);
    connect(this, SIGNAL(forwardSignalBool(bool)), receiver, method, Qt::UniqueConnection);
}

void KisAcyclicSignalConnector::connectBackwardBool(QObject *sender, const char *signal,
                                                   QObject *receiver, const char *method)
{

    connect(sender, signal, this, SLOT(backwardSlotBool(bool)), Qt::UniqueConnection);
    connect(this, SIGNAL(backwardSignalBool(bool)), receiver, method, Qt::UniqueConnection);
}

void KisAcyclicSignalConnector::connectForwardVoid(QObject *sender, const char *signal,
                                                  QObject *receiver, const char *method)
{

    connect(sender, signal, this, SLOT(forwardSlotVoid()), Qt::UniqueConnection);
    connect(this, SIGNAL(forwardSignalVoid()), receiver, method, Qt::UniqueConnection);
}

void KisAcyclicSignalConnector::connectBackwardVoid(QObject *sender, const char *signal,
                                                 QObject *receiver, const char *method)
{

    connect(sender, signal, this, SLOT(backwardSlotVoid()), Qt::UniqueConnection);
    connect(this, SIGNAL(backwardSignalVoid()), receiver, method, Qt::UniqueConnection);
}

void KisAcyclicSignalConnector::connectForwardVariant(QObject *sender, const char *signal,
                                                  QObject *receiver, const char *method)
{

    connect(sender, signal, this, SLOT(forwardSlotVariant(QVariant)), Qt::UniqueConnection);
    connect(this, SIGNAL(forwardSignalVariant(QVariant)), receiver, method, Qt::UniqueConnection);
}

void KisAcyclicSignalConnector::connectBackwardVariant(QObject *sender, const char *signal,
                                                       QObject *receiver, const char *method)
{
    connect(sender, signal, this, SLOT(backwardSlotVariant(QVariant)), Qt::UniqueConnection);
    connect(this, SIGNAL(backwardSignalVariant(QVariant)), receiver, method, Qt::UniqueConnection);
}

void KisAcyclicSignalConnector::connectForwardResourcePair(QObject *sender, const char *signal, QObject *receiver, const char *method)
{
    connect(sender, signal, this, SLOT(forwardSlotResourcePair(int,QVariant)), Qt::UniqueConnection);
    connect(this, SIGNAL(forwardSignalResourcePair(int,QVariant)), receiver, method, Qt::UniqueConnection);
}

void KisAcyclicSignalConnector::connectBackwardResourcePair(QObject *sender, const char *signal, QObject *receiver, const char *method)
{
    connect(sender, signal, this, SLOT(backwardSlotResourcePair(int,QVariant)), Qt::UniqueConnection);
    connect(this, SIGNAL(backwardSignalResourcePair(int,QVariant)), receiver, method, Qt::UniqueConnection);
}

void KisAcyclicSignalConnector::connectForwardKoColor(QObject *sender, const char *signal, QObject *receiver, const char *method)
{
    connect(sender, signal, this, SLOT(forwardSlotKoColor(KoColor)), Qt::UniqueConnection);
    connect(this, SIGNAL(forwardSignalKoColor(KoColor)), receiver, method, Qt::UniqueConnection);
}

void KisAcyclicSignalConnector::connectBackwardKoColor(QObject *sender, const char *signal, QObject *receiver, const char *method)
{
    connect(sender, signal, this, SLOT(backwardSlotKoColor(KoColor)), Qt::UniqueConnection);
    connect(this, SIGNAL(backwardSignalKoColor(KoColor)), receiver, method, Qt::UniqueConnection);
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

bool KisAcyclicSignalConnector::isLocked() const
{
    return m_signalsBlocked;
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
