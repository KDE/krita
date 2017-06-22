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
#include <QBuffer>
#include <QByteArray>
#include <QDataStream>
#include <QProcess>
#include <QLocalServer>
#include <QVBoxLayout>
#include <QUuid>
#include <QList>
#include <QSharedPointer>
#include <QSharedMemory>

#include <klocalizedstring.h>
#include <kpluginfactory.h>

#include <KoDialog.h>
#include <KoColorSpaceConstants.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpaceTraits.h>

#include <KisViewManager.h>
#include <kis_action.h>
#include <kis_config.h>
#include <kis_preference_set_registry.h>
#include <kis_image.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_layer.h>
#include <kis_selection.h>
#include <kis_paint_layer.h>

#include <kis_input_output_mapper.h>
#include <kis_qmic_simple_convertor.h>
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
    m_qmicAction->setActivationFlags(KisAction::ACTIVE_DEVICE);

    connect(m_qmicAction ,  SIGNAL(triggered()), this, SLOT(slotQMic()));

    m_againAction = createAction("QMicAgain");
    m_againAction->setActivationFlags(KisAction::ACTIVE_DEVICE);
    m_againAction->setEnabled(false);
    connect(m_againAction,  SIGNAL(triggered()), this, SLOT(slotQMicAgain()));
}

QMic::~QMic()
{
    Q_FOREACH(QSharedMemory *memorySegment, m_sharedMemorySegments) {
        qDebug() << "detaching" << memorySegment->key();
        memorySegment->detach();
    }
    qDeleteAll(m_sharedMemorySegments);
    m_sharedMemorySegments.clear();

    if (m_pluginProcess) {
        m_pluginProcess->close();
    }

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
    m_pluginProcess->setProcessChannelMode(QProcess::ForwardedChannels);
    connect(m_pluginProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(pluginFinished(int,QProcess::ExitStatus)));
    connect(m_pluginProcess, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(pluginStateChanged(QProcess::ProcessState)));
    m_pluginProcess->start(pluginPath, QStringList() << m_key << (again ? QString(" reapply") : QString::null));

    bool r = m_pluginProcess->waitForStarted();
    while (m_pluginProcess->waitForFinished(10)) {
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
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

    QByteArray msg;
    quint32 remaining;
    ds >> remaining;
    msg.resize(remaining);
    int got = 0;
    char* uMsgBuf = msg.data();
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

    QString message = QString::fromUtf8(msg);
    qDebug() << "Received" << message;

    // Check the message: we can get three different ones
    QMap<QString, QString> messageMap;
    Q_FOREACH(QString line, message.split('\n', QString::SkipEmptyParts)) {
        QList<QString> kv = line.split('=', QString::SkipEmptyParts);
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
        QRectF cropRect = m_view->image()->bounds();
        if (!messageMap.contains("croprect") || !messageMap["croprect"].split(',', QString::SkipEmptyParts).size() == 4) {
            qWarning() << "gmic-qt didn't send a croprect or not a valid croprect";
        }
        else {
            QStringList cr = messageMap["croprect"].split(',', QString::SkipEmptyParts);
            cropRect.setX(cr[0].toFloat());
            cropRect.setY(cr[1].toFloat());
            cropRect.setWidth(cr[2].toFloat());
            cropRect.setHeight(cr[3].toFloat());
        }
        if (!prepareCroppedImages(&ba, cropRect, mode)) {
            qWarning() << "Failed to prepare images for gmic-qt";
        }
    }
    else if (messageMap["command"] == "gmic_qt_output_images") {
        // Parse the message. read the shared memory segments, fix up the current image and send an ack


    }
    else if (messageMap["command"] == "gmic_qt_detach") {
        Q_FOREACH(QSharedMemory *memorySegment, m_sharedMemorySegments) {
            qDebug() << "detaching" << memorySegment->key() << memorySegment->isAttached();
            if (memorySegment->isAttached()) {
                if (!memorySegment->detach()) {
                    qDebug() << "\t" << memorySegment->error() << memorySegment->errorString();
                }
            }
        }
        qDeleteAll(m_sharedMemorySegments);
        m_sharedMemorySegments.clear();
    }
    else {
        qWarning() << "Received unknown command" << messageMap["command"];
    }

    qDebug() << "Sending" << QString::fromUtf8(ba);

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

bool QMic::prepareCroppedImages(QByteArray *message, QRectF &rc, int inputMode)
{
    m_view->image()->lock();

    qDebug() << "prepareCroppedImages()" << QString::fromUtf8(*message) << rc << inputMode;

    KisInputOutputMapper mapper(m_view->image(), m_view->activeNode());
    KisNodeListSP nodes = mapper.inputNodes((InputLayerMode)inputMode);
    if (nodes->isEmpty()) {
        m_view->image()->unlock();
        return false;
    }

    for (int i = 0; i < nodes->size(); ++i) {
        KisNodeSP node = nodes->at(i);
        qDebug() << "Converting node" << node->name() << node->exactBounds();
        if (node->paintDevice()) {

            QRect cropRect = node->exactBounds();

            const int ix = static_cast<int>(std::floor(rc.x() * cropRect.width()));
            const int iy = static_cast<int>(std::floor(rc.y() * cropRect.height()));
            const int iw = std::min(cropRect.width() - ix, static_cast<int>(1 + std::ceil(rc.width() * cropRect.width())));
            const int ih = std::min(cropRect.height() - iy, static_cast<int>(1 + std::ceil(rc.height() * cropRect.height())));

            qDebug() << "crop rect from" << cropRect << QRect(ix, iy, iw, ih);


            QImage img = node->projection()->convertToQImage(0);
            QBuffer buf;
            buf.open(QBuffer::ReadWrite);
            QDataStream out(&buf);
            out << img;

            QSharedMemory *m = new QSharedMemory(QString("key_%1").arg(QUuid::createUuid().toString()));
            m_sharedMemorySegments.append(m);
            if (!m->create(buf.size())) { //iw * ih * 4 * sizeof(float))) {
                qDebug() << "Could not create shared memory segment" << m->error() << m->errorString();
                return false;
            }
            m->lock();

            char *to = (char*)m->data();
            const char *from = buf.data().data();
            Q_ASSERT(m->size() == buf.size());
            memcpy(to, from, m->size());

//            gmic_image<float> img;
//            img.assign(iw, ih, 1, 4);
//            img._data = reinterpret_cast<float*>(m->data());

//            KisQmicSimpleConvertor::convertToGmicImageFast(node->paintDevice(), &img, QRect(ix, iy, iw, ih));

//            KisPaintDeviceSP dev2 = new KisPaintDevice(node->colorSpace());
//            KisQmicSimpleConvertor::convertFromGmicFast(img, dev2, 1.0);
//            qDebug() << "Dev:" << dev2->exactBounds();
//            QImage qimg = dev2->convertToQImage(KoColorSpaceRegistry::instance()->rgb8()->profile());
//            qDebug() << "going to save test image";
//            if (!qimg.save("test2.png", "PNG")) {
//                qDebug() << "Could not save image!" << qimg.size();
//            }

            message->append(m->key().toUtf8());

            m->unlock();

            qDebug() << "size" << m->size();

            message->append(",");
            message->append(node->name().toUtf8());
            message->append(",");
            message->append(QByteArray::number(iw));
            message->append(",");
            message->append(QByteArray::number(ih));

            message->append("\n");
        }
    }

    qDebug() << QString::fromUtf8(*message);

    m_view->image()->unlock();

    return true;
}


#include "QMic.moc"
