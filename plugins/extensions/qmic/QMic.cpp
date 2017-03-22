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

#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QLocalSocket>
#include <QProcess>
#include <QLocalServer>
#include <QVBoxLayout>
#include <QUuid>

#include <klocalizedstring.h>
#include <kpluginfactory.h>

#include <KoDialog.h>

#include <KisViewManager.h>
#include <kis_action.h>
#include <kis_config.h>
#include <kis_preference_set_registry.h>
#include <kis_image.h>

#include <PluginSettings.h>

static const char ack[] = "ack";

K_PLUGIN_FACTORY_WITH_JSON(QMicFactory, "kritaqmic.json", registerPlugin<QMic>();)

QMic::QMic(QObject *parent, const QVariantList &)
    : KisViewPlugin(parent)
{
    KisPreferenceSetRegistry *preferenceSetRegistry = KisPreferenceSetRegistry::instance();
    PluginSettingsFactory* settingsFactory = new PluginSettingsFactory();
    preferenceSetRegistry->add("QMicPluginSettingsFactory", settingsFactory);

    m_qmicAction = createAction("QMic");
    connect(m_qmicAction ,  SIGNAL(triggered()), this, SLOT(slotQMic()));

    m_againAction = createAction("QMicAgain");
    connect(m_againAction,  SIGNAL(triggered()), this, SLOT(slotQMicAgain()));
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
    m_qmicAction->setEnabled(false);
    m_againAction->setEnabled(false);

    if (m_pluginProcess) {
        qDebug() << "Plugin is already started" << m_pluginProcess->state();
        return;
    }

    // find the krita-gmic-qt plugin
    KisConfig cfg;
    QString pluginPath = cfg.readEntry<QString>("gmic_qt_plugin_path", QString::null);

    if (pluginPath.isEmpty() || !QFileInfo(pluginPath).exists()) {
        KoDialog dlg;
        dlg.setWindowTitle(i18nc("@title:Window", "Krita"));
        QWidget *w = new QWidget(&dlg);
        dlg.setMainWidget(w);
        QVBoxLayout *l = new QVBoxLayout(w);
        l->addWidget(new PluginSettings(w));
        dlg.setButtons(KoDialog::Ok);
        dlg.exec();
        pluginPath = cfg.readEntry<QString>("gmic_qt_plugin_path", QString::null);
        if (pluginPath.isEmpty() || !QFileInfo(pluginPath).exists()) {
            return;
        }
    }

    m_key = QUuid::createUuid().toString();
    m_localServer = new QLocalServer();
    m_localServer->listen(m_key);
    connect(m_localServer, SIGNAL(newConnection()), SLOT(connected()));
    m_pluginProcess = new QProcess(this);
    connect(m_pluginProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(pluginFinished(int,QProcess::ExitStatus)));
    connect(m_pluginProcess, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(pluginStateChanged(QProcess::ProcessState)));
    m_pluginProcess->start(pluginPath, QStringList() << m_key << (again ? QString(" reapply") : QString::null));

    bool r = m_pluginProcess->waitForStarted();
    while (m_pluginProcess->waitForFinished(10)) {
        qApp->processEvents();
    }
    qDebug() << "Plugin started" << r << m_pluginProcess->state();
}

void QMic::connected()
{
    qDebug() << "connected";

    QLocalSocket *socket = m_localServer->nextPendingConnection();
    if (!socket) { return; }

    while (socket->bytesAvailable() < static_cast<int>(sizeof(quint32))) {
        if (!socket->isValid()) { // stale request
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

    qDebug() << "Received" << QString::fromLatin1(message);

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

    if (!messageMap.contains("command")) {
        qWarning() << "Message did not contain a command";
        return;
    }

    int mode = 0;
    if (messageMap.contains("mode")) {
        mode = messageMap["mode"].toInt();
    }

    QByteArray ba;

    if (messageMap["command"] == "gmic_qt_get_image_size") {
        ba = QByteArray::number(m_view->image()->width()) + "," + QByteArray::number(m_view->image()->height());
    }
    else if (messageMap["command"] == "gmic_qt_get_cropped_images") {
        // Parse the message, create the shared memory segments, and create a new message to send back and waid for ack
        QRect cropRect = m_view->image()->bounds();
        if (!messageMap.contains("croprect") || !messageMap["croprect"].split(',').size() == 4) {
            qWarning() << "gmic-qt didn't send a croprect or not a valid croprect";
        }
        else {
            QList<QByteArray> cr = messageMap["croprect"].split(',');
            cropRect.setX(cr[0].toInt());
            cropRect.setY(cr[1].toInt());
            cropRect.setWidth(cr[2].toInt());
            cropRect.setHeight(cr[3].toInt());
    }
    else if (messageMap["command"] == "gmic_qt_output_images") {
        // Parse the message. read the shared memory segments, fix up the current image and send an ack


    }
    else {
        qWarning() << "Received unknown command" << messageMap["command"];
    }

    ds.writeBytes(ba.constData(), ba.length());

    // Wait for the ack
    bool r;
    r &= socket->waitForReadyRead(); // wait for ack
    r &= (socket->read(qstrlen(ack)) == ack);
    socket->waitForDisconnected(-1);

}

void QMic::pluginStateChanged(QProcess::ProcessState state)
{
    qDebug() << "stateChanged" << state;
}

void QMic::pluginFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "pluginFinished" << exitCode << exitStatus;
    delete m_pluginProcess;
    m_pluginProcess = 0;
    delete m_localServer;
    m_localServer = 0;
    m_qmicAction->setEnabled(true);
    m_againAction->setEnabled(true);
}


#include "QMic.moc"
