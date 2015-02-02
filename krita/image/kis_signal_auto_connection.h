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

#ifndef __KIS_SIGNAL_AUTO_CONNECTOR_H
#define __KIS_SIGNAL_AUTO_CONNECTOR_H

#include <QObject>


class KisSignalAutoConnection
{
public:
    inline KisSignalAutoConnection(const QObject *sender, const char *signal,
                                  const QObject *receiver, const char *method,
                                  Qt::ConnectionType type = Qt::AutoConnection)
        : m_sender(const_cast<QObject*>(sender)),
          m_signal(signal),
          m_receiver(const_cast<QObject*>(receiver)),
          m_method(method)
    {
        QObject::connect(m_sender, m_signal, m_receiver, m_method, type);
    }
    inline ~KisSignalAutoConnection()
    {
        if (!m_sender.isNull() && !m_receiver.isNull()) {
            QObject::disconnect(m_sender, m_signal, m_receiver, m_method);
        }
    }

private:
    KisSignalAutoConnection(const KisSignalAutoConnection &rhs);

private:
    QPointer<QObject> m_sender;
    const char *m_signal;
    QPointer<QObject> m_receiver;
    const char *m_method;
};

typedef QSharedPointer<KisSignalAutoConnection> KisSignalAutoConnectionSP;

class KisSignalAutoConnectionsStore
{
public:
    inline void addConnection(const QObject *sender, const char *signal,
                              const QObject *receiver, const char *method,
                              Qt::ConnectionType type = Qt::AutoConnection)
    {
        m_connections.append(KisSignalAutoConnectionSP(
                                 new KisSignalAutoConnection(sender, signal,
                                                             receiver, method, type)));
    }

    inline void addUniqueConnection(const QObject *sender, const char *signal,
                                    const QObject *receiver, const char *method)
    {
        m_connections.append(KisSignalAutoConnectionSP(
                                 new KisSignalAutoConnection(sender, signal,
                                                             receiver, method, Qt::UniqueConnection)));
    }

    inline void clear() {
        m_connections.clear();
    }

private:
    QVector<KisSignalAutoConnectionSP> m_connections;
};

#endif /* __KIS_SIGNAL_AUTO_CONNECTOR_H */
