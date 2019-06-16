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
#include <QPointer>
#include <QVector>

/**
 * A special wrapper class that represents a connection between two QObject objects.
 * It creates the connection on the construction and disconnects it on destruction.
 *
 * WARNING: never use QScopedPointer::reset() for updating the
 *          connection like:
 *
 * QScopedPointer<KisSignalAutoConnection> conn;
 * ...
 * void Something::setCanvas(KoCanvasBase * canvas) {
 *     conn.reset(new KisSignalAutoConnection(...));
 * }
 *
 * The object stored in a scoped pointer will be destructed *after*
 * the new object created which will cause you object to become
 * disconnected.
 *
 * Instead use two-stage updates:
 * conn.reset();
 * conn.reset(new KisSignalAutoConnection(...));
 */
class KisSignalAutoConnection
{
public:
    /**
     * Creates a connection object and starts the requested connection
     */
    template<class Sender, class Signal, class Receiver, class Method>
    inline KisSignalAutoConnection(Sender sender, Signal signal,
                                  Receiver receiver, Method method,
                                  Qt::ConnectionType type = Qt::AutoConnection)
        : m_connection(QObject::connect(sender, signal, receiver, method, type))
    {
    }

    inline ~KisSignalAutoConnection()
    {
        QObject::disconnect(m_connection);
    }

private:
    KisSignalAutoConnection(const KisSignalAutoConnection &rhs);

private:
    QMetaObject::Connection m_connection;
};

typedef QSharedPointer<KisSignalAutoConnection> KisSignalAutoConnectionSP;


/**
 * A class to store multiple connections and to be able to stop all of
 * them at once. It is handy when you need to reconnect some other
 * object to the current manager. Then you just call
 * connectionsStore.clear() and then call addConnection() again to
 * recreate them.
 */
class KisSignalAutoConnectionsStore
{
public:
    /**
     * Connects \p sender to \p receiver with a connection of type \p type.
     * The connection is saved into the store so can be reset later with clear()
     *
     * \see addUniqueConnection()
     */
    template<class Sender, class Signal, class Receiver, class Method>
    inline void addConnection(Sender sender, Signal signal,
                              Receiver receiver, Method method,
                              Qt::ConnectionType type = Qt::AutoConnection)
    {
        m_connections.append(KisSignalAutoConnectionSP(
                                 new KisSignalAutoConnection(sender, signal,
                                                             receiver, method, type)));
    }

    /**
     * Convenience override for addConnection() that creates a unique connection
     *
     * \see addConnection()
     */
    template<class Sender, class Signal, class Receiver, class Method>
    inline void addUniqueConnection(Sender sender, Signal signal,
                                    Receiver receiver, Method method)
    {
        m_connections.append(KisSignalAutoConnectionSP(
                                 new KisSignalAutoConnection(sender, signal,
                                                             receiver, method, Qt::UniqueConnection)));
    }

    /**
     * Disconnects all the stored connections and removes them from the store
     */
    inline void clear() {
        m_connections.clear();
    }

    inline bool isEmpty() {
        return m_connections.isEmpty();
    }

private:
    QVector<KisSignalAutoConnectionSP> m_connections;
};

#endif /* __KIS_SIGNAL_AUTO_CONNECTOR_H */
