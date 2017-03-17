/*
 * Copyright (c) 2017 Boudewijn Rempt <boud@valdyas.org>
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

#include "gmic-qt.h"

#include <QDebug>
#include <QFileInfo>
#include <QLocalSocket>
#include <QProcess>
#include <QUuid>

#include <klocalizedstring.h>
#include <kpluginfactory.h>

#include <kis_config.h>

K_PLUGIN_FACTORY_WITH_JSON(GmicQtFactory, "kritagmic-qt.json", registerPlugin<GmicQt>();)

GmicQt::GmicQt(QObject *parent, const QVariantList &)
        : KisViewPlugin(parent)
{
    KisAction *action = createAction("GmicQt");
    connect(action,  SIGNAL(triggered()), this, SLOT(slotGmicQt()));

    KisAction *action = createAction("GmicQtAgain");
    connect(action,  SIGNAL(triggered()), this, SLOT(slotGmicQtAgain()));
}

GmicQt::~GmicQt()
{
    delete m_localServer;
}

void GmicQt::slotGmicQtAgain()
{
    slotGmicQt(true);
}

void GmicQt::slotGmicQt(bool again)
{
    m_localServer = new QLocalServer();
    m_localServer->listen("krita-gmic");
    connect(m_localServer, SIGNAL(newConnection()), SLOT(connected()));

    // find the krita-gmic-qt plugin
    KisConfig cfg;
    QString pluginPath = cfg.gmicQtPluginPath();

    // start the plugin
    if (QFileInfo(pluginPath).exists()) {
        int retval = QProcess::execute(pluginPath + " "
                                       + QUuid::createUuid().toString()
                                       + (again ? " reapply" : QString::null));
        qDebug() << retval;
    }
}

void GmicQt::connected()
{
    QLocalSocke *socket = server->nextPendingConnection();
    if (!socket) { return; }

    while (socket->bytesAvailable() < static_cast<int>(sizeof(quint32))) {
        if (!socket->isValid()) { // stale request
            m_localServer->close();
            delete m_localServer;
            m_localServer = 0;
            return;
        }
        socket->waitForReadyRead(1000);
    }
    QDataStream ds(socket);
    QByteArray message;
    quint32 remaining;
    ds >> remaining;
    message.resize(remaining);
    int got = 0;
    char* uMsgBuf = message.data();
    do {
        got = ds.readRawData(uMsgBuf, remaining);
        remaining -= got;
        uMsgBuf += got;
    }
    while (remaining && got >= 0 && socket->waitForReadyRead(2000));

    if (got < 0) {
        qWarning() << "Message reception failed" << socket->errorString();
        delete socket;
        m_localServer->close();
        delete m_localServer;
        m_localServer = 0;
        return;
    }

    // Check the message: we can get three different ones
    QMap<QByteArray, QByteArray> messageMap;
    Q_FOREACH(QByteArray line, message.split("\n")) {
        QList<QByteArray> kv = line.split('=');
        if (kv.size() == 2) {
            messageMap[kv[0]] = kv[1];
        }
        else {
            qWarning() << "line" << line << "is invalid.";
        }
    }

    if (message.startsWith("")) {

    }
    else if (message.startsWith("gmic_qt_get_cropped_images")) {
        // Parse the message, create the shared memory segments, and create a new message to send back and waid for ack
        QByteArray ba;
        QDataStream ds(&socket);
        ds.writeBytes(ba.constData(), ba.length());
        bool r = socket.waitForBytesWritten();

        r &= socket.waitForReadyRead(timeout); // wait for ack
        r &= (socket.read(qstrlen(ack)) == ack);

    }
    else if (message.startsWith("gmic_qt_output_images")) {
        // Parse the message. read the shared memory segments, fix up the current image and send an ack
        socket.write(ack, qstrlen(ack));
        socket.waitForBytesWritten(1000);

    }
    socket.disconnectFromServer();
    m_localServer->close();
    delete m_localServer;
    m_localServer = 0;

}


#include "gmic-qt.moc"
