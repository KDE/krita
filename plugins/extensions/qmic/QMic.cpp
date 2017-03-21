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

#include "QMic.h"

#include <QDebug>
#include <QFileInfo>
#include <QLocalSocket>
#include <QProcess>
#include <QMessageBox>
#include <QUuid>

#include <klocalizedstring.h>
#include <kpluginfactory.h>

#include <kis_action.h>
#include <kis_config.h>
#include <kis_preference_set_registry.h>

#include <PluginSettings.h>

static const char ack[] = "ack";

K_PLUGIN_FACTORY_WITH_JSON(QMicFactory, "kritaqmic.json", registerPlugin<QMic>();)

QMic::QMic(QObject *parent, const QVariantList &)
    : KisViewPlugin(parent)
{
    KisPreferenceSetRegistry *preferenceSetRegistry = KisPreferenceSetRegistry::instance();
    PluginSettingsFactory* settingsFactory = new PluginSettingsFactory();
    preferenceSetRegistry->add("QMicPluginSettingsFactory", settingsFactory);

    KisAction *action = createAction("QMic");
    connect(action,  SIGNAL(triggered()), this, SLOT(slotQMic()));

    action = createAction("QMicAgain");
    connect(action,  SIGNAL(triggered()), this, SLOT(slotQMicAgain()));
}

QMic::~QMic()
{
    delete m_localServer;
}

void QMic::slotQMicAgain()
{
    slotQMic(true);
}

void QMic::slotQMic(bool again)
{
    m_localServer = new QLocalServer();
    m_localServer->listen("krita-gmic");
    connect(m_localServer, SIGNAL(newConnection()), SLOT(connected()));

    // find the krita-gmic-qt plugin
    KisConfig cfg;
    QString pluginPath = cfg.readEntry<QString>("gmic_qt_plugin_path", QString::null);

    if (pluginPath.isEmpty() || !QFileInfo(pluginPath).exists()) {
        QMessageBox::warning(0, i18nc("@title:window", "Krita"), i18n("Please download the gmic-qt plugin for Krita from <a href=\"http://gmic.eu/\">G'MIC.eu</a>."));
        return;
    }

    // start the plugin
    int retval = QProcess::execute(pluginPath + " "
                                   + QUuid::createUuid().toString()
                                   + (again ? QString(" reapply") : QString::null));
    qDebug() << retval;

}

void QMic::connected()
{
    QLocalSocket *socket = m_localServer->nextPendingConnection();
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
    Q_FOREACH(QByteArray line, message.split('\n')) {
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
        QDataStream ds(socket);
        ds.writeBytes(ba.constData(), ba.length());
        bool r = socket->waitForBytesWritten();

        r &= socket->waitForReadyRead(); // wait for ack
        r &= (socket->read(qstrlen(ack)) == ack);

    }
    else if (message.startsWith("gmic_qt_output_images")) {
        // Parse the message. read the shared memory segments, fix up the current image and send an ack
        socket->write(ack, qstrlen(ack));
        socket->waitForBytesWritten(1000);

    }
    socket->disconnectFromServer();
    m_localServer->close();
    delete m_localServer;
    m_localServer = 0;

}


#include "QMic.moc"
