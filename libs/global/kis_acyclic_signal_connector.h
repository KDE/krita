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

#ifndef __KIS_ACYCLIC_SIGNAL_CONNECTOR_H
#define __KIS_ACYCLIC_SIGNAL_CONNECTOR_H

#include <QObject>
#include "kritaglobal_export.h"
#include <mutex>

class KisAcyclicSignalConnector;
class KoColor;

#include <QVector>
#include <QPointer>

/**
 * A special class for connecting UI elements to manager classes.
 * It allows to avoid direct calling blockSignals() for the sender UI
 * element all the time. This is the most important when the measured
 * value can be changed not only by the user through the UI, but also
 * by the manager according to some internal rules.
 *
 * Example:
 *
 * Suppose we have the following connections:
 *
 * 1) QDoubleSpinBox::valueChanged(double) -> Manager::slotSetValue(double)
 * 2) Manager::valueChanged(double) -> QDoubleSpinBox::setValue(double)
 *
 * Now if the manager decides to change/correct the value, the spinbox
 * will go into an infinite loop.
 *
 * See an example in KisToolCropConfigWidget.
 *
 * NOTE (coordinated connectors):
 *
 * Please make sure that you don't convert more than one forward and one backward
 * connection to the connector! If you do so, they will become connected to the
 * same forwarding slot and, therefore, both output signals will be emitted on
 * every incoming signal.
 *
 * To connect multiple connections that block recursive calls, please use
 * "coordinated connectors". Each such connector will have two more connection
 * slots that you can reuse.
 *
 */

class KRITAGLOBAL_EXPORT KisAcyclicSignalConnector : public QObject
{
    Q_OBJECT
public:
    typedef std::unique_lock<KisAcyclicSignalConnector> Blocker;

public:

    KisAcyclicSignalConnector(QObject *parent = 0);
    ~KisAcyclicSignalConnector();

    void connectForwardDouble(QObject *sender, const char *signal,
                              QObject *receiver, const char *method);

    void connectBackwardDouble(QObject *sender, const char *signal,
                               QObject *receiver, const char *method);

    void connectForwardInt(QObject *sender, const char *signal,
                           QObject *receiver, const char *method);

    void connectBackwardInt(QObject *sender, const char *signal,
                            QObject *receiver, const char *method);

    void connectForwardBool(QObject *sender, const char *signal,
                            QObject *receiver, const char *method);

    void connectBackwardBool(QObject *sender, const char *signal,
                             QObject *receiver, const char *method);

    void connectForwardVoid(QObject *sender, const char *signal,
                            QObject *receiver, const char *method);

    void connectBackwardVoid(QObject *sender, const char *signal,
                             QObject *receiver, const char *method);

    void connectForwardVariant(QObject *sender, const char *signal,
                               QObject *receiver, const char *method);

    void connectBackwardVariant(QObject *sender, const char *signal,
                                QObject *receiver, const char *method);

    void connectForwardResourcePair(QObject *sender, const char *signal,
                                     QObject *receiver, const char *method);

    void connectBackwardResourcePair(QObject *sender, const char *signal,
                                     QObject *receiver, const char *method);

    void connectForwardKoColor(QObject *sender, const char *signal,
                               QObject *receiver, const char *method);

    void connectBackwardKoColor(QObject *sender, const char *signal,
                                QObject *receiver, const char *method);

    /**
     * Lock the connector and all its coordinated child connectors
     */
    void lock();

    /**
     * Unlock the connector and all its coordinated child connectors
     */
    void unlock();

    /**
     * @brief create a coordinated connector that can be used for extending
     *        the number of self-locking connection.
     *
     * The coordinated connector can be used to extend the number of self-locking
     * connections. Each coordinated connector adds two more connection slots (forward
     * and backward).  Lock of any connector in a coordinated group will lock the whole
     * group.
     *
     * The created connector is owned by *this, don't delete it!
     */
    KisAcyclicSignalConnector *createCoordinatedConnector();

private:

    /**
     * Lock this connector only.
     */
    void coordinatedLock();

    /**
     * Unlock this connector only.
     */
    void coordinatedUnlock();

private Q_SLOTS:
    void forwardSlotDouble(double value);
    void backwardSlotDouble(double value);

    void forwardSlotInt(int value);
    void backwardSlotInt(int value);

    void forwardSlotBool(bool value);
    void backwardSlotBool(bool value);

    void forwardSlotVoid();
    void backwardSlotVoid();

    void forwardSlotVariant(const QVariant &value);
    void backwardSlotVariant(const QVariant &value);

    void forwardSlotResourcePair(int key, const QVariant &resource);
    void backwardSlotResourcePair(int key, const QVariant &resource);

    void forwardSlotKoColor(const KoColor &value);
    void backwardSlotKoColor(const KoColor &value);

Q_SIGNALS:
    void forwardSignalDouble(double value);
    void backwardSignalDouble(double value);

    void forwardSignalInt(int value);
    void backwardSignalInt(int value);

    void forwardSignalBool(bool value);
    void backwardSignalBool(bool value);

    void forwardSignalVoid();
    void backwardSignalVoid();

    void forwardSignalVariant(const QVariant &value);
    void backwardSignalVariant(const QVariant &value);

    void forwardSignalResourcePair(int key, const QVariant &value);
    void backwardSignalResourcePair(int key, const QVariant &value);

    void forwardSignalKoColor(const KoColor &value);
    void backwardSignalKoColor(const KoColor &value);

private:
    int m_signalsBlocked;
    QVector<QPointer<KisAcyclicSignalConnector>> m_coordinatedConnectors;
    QPointer<KisAcyclicSignalConnector> m_parentConnector;
};

#endif /* __KIS_ACYCLIC_SIGNAL_CONNECTOR_H */
