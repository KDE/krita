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

void KisAcyclicSignalConnector::forwardSlotDouble(double value)
{
    if (m_signalsBlocked) return;

    m_signalsBlocked++;
    emit forwardSignalDouble(value);
    m_signalsBlocked--;
}

void KisAcyclicSignalConnector::backwardSlotDouble(double value)
{
    if (m_signalsBlocked) return;

    m_signalsBlocked++;
    emit backwardSignalDouble(value);
    m_signalsBlocked--;
}

void KisAcyclicSignalConnector::forwardSlotInt(int value)
{
    if (m_signalsBlocked) return;

    m_signalsBlocked++;
    emit forwardSignalInt(value);
    m_signalsBlocked--;
}

void KisAcyclicSignalConnector::backwardSlotInt(int value)
{
    if (m_signalsBlocked) return;

    m_signalsBlocked++;
    emit backwardSignalInt(value);
    m_signalsBlocked--;
}

void KisAcyclicSignalConnector::forwardSlotBool(bool value)
{
    if (m_signalsBlocked) return;

    m_signalsBlocked++;
    emit forwardSignalBool(value);
    m_signalsBlocked--;
}

void KisAcyclicSignalConnector::backwardSlotBool(bool value)
{
    if (m_signalsBlocked) return;

    m_signalsBlocked++;
    emit backwardSignalBool(value);
    m_signalsBlocked--;
}

void KisAcyclicSignalConnector::forwardSlotVoid()
{
    if (m_signalsBlocked) return;

    m_signalsBlocked++;
    emit forwardSignalVoid();
    m_signalsBlocked--;
}

void KisAcyclicSignalConnector::backwardSlotVoid()
{
    if (m_signalsBlocked) return;

    m_signalsBlocked++;
    emit backwardSignalVoid();
    m_signalsBlocked--;
}

