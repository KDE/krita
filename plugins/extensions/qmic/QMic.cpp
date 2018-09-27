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
#include <QMultiMap>
#include <QSharedMemory>
#include <QMessageBox>

#include <klocalizedstring.h>
#include <kpluginfactory.h>

#include <KoDialog.h>
#include <KoColorSpaceConstants.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpaceTraits.h>

#include <KisPart.h>
#include <KisViewManager.h>
#include <kis_action.h>
#include <kis_config.h>
#include <kis_preference_set_registry.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_layer.h>
#include <kis_selection.h>
#include <kis_paint_layer.h>
#include <kis_algebra_2d.h>

#include "kis_input_output_mapper.h"
#include "kis_qmic_simple_convertor.h"
#include "kis_import_qmic_processing_visitor.h"
#include <PluginSettings.h>

#include "kis_qmic_applicator.h"

static const char ack[] = "ack";

K_PLUGIN_FACTORY_WITH_JSON(QMicFactory, "kritaqmic.json", registerPlugin<QMic>();)

QMic::QMic(QObject *parent, const QVariantList &)
    : KisActionPlugin(parent)
    , m_gmicApplicator(0)
{
#ifndef Q_OS_MAC
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

    m_gmicApplicator = new KisQmicApplicator();
    connect(m_gmicApplicator, SIGNAL(gmicFinished(bool,int,QString)), this, SLOT(slotGmicFinished(bool,int,QString)));
#endif
}

QMic::~QMic()
{
    Q_FOREACH(QSharedMemory *memorySegment, m_sharedMemorySegments) {
//        dbgPlugins << "detaching" << memorySegment->key();
        memorySegment->detach();
    }
    qDeleteAll(m_sharedMemorySegments);
    m_sharedMemorySegments.clear();

    if (m_pluginProcess) {
        m_pluginProcess->close();
    }

    delete m_gmicApplicator;
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

    // find the krita-gmic-qt plugin
    QString pluginPath = PluginSettings::gmicQtPath();
    if (pluginPath.isEmpty() || !QFileInfo(pluginPath).exists() || !QFileInfo(pluginPath).isFile()) {
        QMessageBox::warning(0, i18nc("@title:window", "Krita"), i18n("Krita cannot find the gmic-qt plugin. You can set the location of the gmic-qt plugin in Settings/Configure Krita."));
        return;
    }

    m_key = QUuid::createUuid().toString();
    m_localServer = new QLocalServer();
    m_localServer->listen(m_key);
    connect(m_localServer, SIGNAL(newConnection()), SLOT(connected()));
    m_pluginProcess = new QProcess(this);
    connect(viewManager(), SIGNAL(destroyed(QObject*o)), m_pluginProcess, SLOT(terminate()));
    m_pluginProcess->setProcessChannelMode(QProcess::ForwardedChannels);
    connect(m_pluginProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(pluginFinished(int,QProcess::ExitStatus)));
    connect(m_pluginProcess, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(pluginStateChanged(QProcess::ProcessState)));
    m_pluginProcess->start(pluginPath, QStringList() << m_key << (again ? QString(" reapply") : QString()));

    bool r = m_pluginProcess->waitForStarted();
    while (m_pluginProcess->waitForFinished(10)) {
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }
    dbgPlugins << "Plugin started" << r << m_pluginProcess->state();
}

void QMic::connected()
{
    dbgPlugins << "connected";
    if (!viewManager()) return;

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
    // FIXME: Should use read transaction for Qt >= 5.7:
    //        https://doc.qt.io/qt-5/qdatastream.html#using-read-transactions
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
    dbgPlugins << "Received" << message;

    // Check the message: we can get three different ones
    QMultiMap<QString, QString> messageMap;
    Q_FOREACH(QString line, message.split('\n', QString::SkipEmptyParts)) {
        QList<QString> kv = line.split('=', QString::SkipEmptyParts);
        if (kv.size() == 2) {
            messageMap.insert(kv[0], kv[1]);
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
        mode = messageMap.values("mode").first().toInt();
    }

    QByteArray ba;
    QString messageBoxWarningText;

    if (messageMap.values("command").first() == "gmic_qt_get_image_size") {
        KisSelectionSP selection = viewManager()->image()->globalSelection();

        if (selection) {
            QRect selectionRect = selection->selectedExactRect();
            ba = QByteArray::number(selectionRect.width()) + "," + QByteArray::number(selectionRect.height());
        }
        else {
            ba = QByteArray::number(viewManager()->image()->width()) + "," + QByteArray::number(viewManager()->image()->height());
        }
    }
    else if (messageMap.values("command").first() == "gmic_qt_get_cropped_images") {
        // Parse the message, create the shared memory segments, and create a new message to send back and waid for ack
        QRectF cropRect(0.0, 0.0, 1.0, 1.0);
        if (!messageMap.contains("croprect") || messageMap.values("croprect").first().split(',', QString::SkipEmptyParts).size() != 4) {
            qWarning() << "gmic-qt didn't send a croprect or not a valid croprect";
        }
        else {
            QStringList cr = messageMap.values("croprect").first().split(',', QString::SkipEmptyParts);
            cropRect.setX(cr[0].toFloat());
            cropRect.setY(cr[1].toFloat());
            cropRect.setWidth(cr[2].toFloat());
            cropRect.setHeight(cr[3].toFloat());
        }
        if (!prepareCroppedImages(&ba, cropRect, mode)) {
            qWarning() << "Failed to prepare images for gmic-qt";
        }
    }
    else if (messageMap.values("command").first() == "gmic_qt_output_images") {
        // Parse the message. read the shared memory segments, fix up the current image and send an ack
        dbgPlugins << "gmic_qt_output_images";
        QStringList layers = messageMap.values("layer");
        m_outputMode = (OutputMode)mode;
        if (m_outputMode != IN_PLACE) {
            messageBoxWarningText = i18n("Sorry, this output mode is not implemented yet.");
            m_outputMode = IN_PLACE;
        }
        slotStartApplicator(layers);
    }
    else if (messageMap.values("command").first() == "gmic_qt_detach") {
        Q_FOREACH(QSharedMemory *memorySegment, m_sharedMemorySegments) {
            dbgPlugins << "detaching" << memorySegment->key() << memorySegment->isAttached();
            if (memorySegment->isAttached()) {
                if (!memorySegment->detach()) {
                    dbgPlugins << "\t" << memorySegment->error() << memorySegment->errorString();
                }
            }
        }
        qDeleteAll(m_sharedMemorySegments);
        m_sharedMemorySegments.clear();
    }
    else {
        qWarning() << "Received unknown command" << messageMap.values("command");
    }

    dbgPlugins << "Sending" << QString::fromUtf8(ba);

    // HACK: Make sure QDataStream does not refuse to write!
    // Proper fix: Change the above read to use read transaction
    ds.resetStatus();
    ds.writeBytes(ba.constData(), ba.length());
    // Flush the socket because we might not return to the event loop!
    if (!socket->waitForBytesWritten(2000)) {
        qWarning() << "Failed to write response:" << socket->error();
    }

    // Wait for the ack
    bool r = true;
    r &= socket->waitForReadyRead(2000); // wait for ack
    r &= (socket->read(qstrlen(ack)) == ack);
    if (!socket->waitForDisconnected(2000)) {
        qWarning() << "Remote not disconnected:" << socket->error();
        // Wait again
        socket->disconnectFromServer();
        if (socket->waitForDisconnected(2000)) {
            qWarning() << "Disconnect timed out:" << socket->error();
        }
    }

    if (!messageBoxWarningText.isEmpty()) {
        // Defer the message box to the event loop
        QTimer::singleShot(0, [messageBoxWarningText]() {
            QMessageBox::warning(KisPart::instance()->currentMainwindow(), i18nc("@title:window", "Krita"), messageBoxWarningText);
        });
    }
}

void QMic::pluginStateChanged(QProcess::ProcessState state)
{
    dbgPlugins << "stateChanged" << state;
}

void QMic::pluginFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    dbgPlugins << "pluginFinished" << exitCode << exitStatus;
    delete m_pluginProcess;
    m_pluginProcess = 0;
    delete m_localServer;
    m_localServer = 0;
    m_qmicAction->setEnabled(true);
    m_againAction->setEnabled(true);
}

void QMic::slotGmicFinished(bool successfully, int milliseconds, const QString &msg)
{
    dbgPlugins << "slotGmicFinished();" << successfully << milliseconds << msg;
    if (successfully) {
        m_gmicApplicator->finish();
    }
    else {
        m_gmicApplicator->cancel();
        QMessageBox::warning(0, i18nc("@title:window", "Krita"), i18n("G'Mic failed, reason:") + msg);
    }
}

void QMic::slotStartApplicator(QStringList gmicImages)
{
    dbgPlugins << "slotStartApplicator();" << gmicImages;
    if (!viewManager()) return;
    // Create a vector of gmic images

    QVector<gmic_image<float> *> images;

    Q_FOREACH(const QString &image, gmicImages) {
        QStringList parts = image.split(',', QString::SkipEmptyParts);
        Q_ASSERT(parts.size() == 4);
        QString key = parts[0];
        QString layerName = QByteArray::fromHex(parts[1].toLatin1());
        int spectrum = parts[2].toInt();
        int width = parts[3].toInt();
        int height = parts[4].toInt();

        dbgPlugins << key << layerName << width << height;

        QSharedMemory m(key);
        if (!m.attach(QSharedMemory::ReadOnly)) {
            qWarning() << "Could not attach to shared memory area." << m.error() << m.errorString();
        }
        if (m.isAttached()) {
            if (!m.lock()) {
                dbgPlugins << "Could not lock memory segment"  << m.error() << m.errorString();
            }
            dbgPlugins << "Memory segment" << key << m.size() << m.constData() << m.data();
            gmic_image<float> *gimg = new gmic_image<float>();
            gimg->assign(width, height, 1, spectrum);
            gimg->name = layerName;

            gimg->_data = new float[width * height * spectrum * sizeof(float)];
            dbgPlugins << "width" << width << "height" << height << "size" << width * height * spectrum * sizeof(float) << "shared memory size" << m.size();
            memcpy(gimg->_data, m.constData(), width * height * spectrum * sizeof(float));

            dbgPlugins << "created gmic image" << gimg->name << gimg->_width << gimg->_height;

            if (!m.unlock()) {
                dbgPlugins << "Could not unlock memory segment"  << m.error() << m.errorString();
            }
            if (!m.detach()) {
                dbgPlugins << "Could not detach from memory segment"  << m.error() << m.errorString();
            }
            images.append(gimg);
        }
    }

    dbgPlugins << "Got" << images.size() << "gmic images";

    // Start the applicator
    KUndo2MagicString actionName = kundo2_i18n("Gmic filter");
    KisNodeSP rootNode = viewManager()->image()->root();
    KisInputOutputMapper mapper(viewManager()->image(), viewManager()->activeNode());
    KisNodeListSP layers = mapper.inputNodes(m_inputMode);

    m_gmicApplicator->setProperties(viewManager()->image(), rootNode, images, actionName, layers);
    m_gmicApplicator->apply();
    m_gmicApplicator->finish();
}

bool QMic::prepareCroppedImages(QByteArray *message, QRectF &rc, int inputMode)
{
    if (!viewManager()) return false;

    viewManager()->image()->lock();

    m_inputMode = (InputLayerMode)inputMode;

    dbgPlugins << "prepareCroppedImages()" << QString::fromUtf8(*message) << rc << inputMode;

    KisInputOutputMapper mapper(viewManager()->image(), viewManager()->activeNode());
    KisNodeListSP nodes = mapper.inputNodes(m_inputMode);
    if (nodes->isEmpty()) {
        viewManager()->image()->unlock();
        return false;
    }

    for (int i = 0; i < nodes->size(); ++i) {
        KisNodeSP node = nodes->at(i);
        if (node && node->paintDevice()) {
            QRect cropRect;

            KisSelectionSP selection = viewManager()->image()->globalSelection();

            if (selection) {
                cropRect = selection->selectedExactRect();
            }
            else {
                cropRect = viewManager()->image()->bounds();
            }

            dbgPlugins << "Converting node" << node->name() << cropRect;

            const QRectF mappedRect = KisAlgebra2D::mapToRect(cropRect).mapRect(rc);
            const QRect resultRect = mappedRect.toAlignedRect();

            QSharedMemory *m = new QSharedMemory(QString("key_%1").arg(QUuid::createUuid().toString()));
            m_sharedMemorySegments.append(m);
            if (!m->create(resultRect.width() * resultRect.height() * 4 * sizeof(float))) {  //buf.size())) {
                qWarning() << "Could not create shared memory segment" << m->error() << m->errorString();
                return false;
            }
            m->lock();

            gmic_image<float> img;
            img.assign(resultRect.width(), resultRect.height(), 1, 4);
            img._data = reinterpret_cast<float*>(m->data());

            KisQmicSimpleConvertor::convertToGmicImageFast(node->paintDevice(), &img, resultRect);

            message->append(m->key().toUtf8());

            m->unlock();

            message->append(",");
            message->append(node->name().toUtf8().toHex());
            message->append(",");
            message->append(QByteArray::number(resultRect.width()));
            message->append(",");
            message->append(QByteArray::number(resultRect.height()));

            message->append("\n");
        }
    }

    dbgPlugins << QString::fromUtf8(*message);

    viewManager()->image()->unlock();

    return true;
}


#include "QMic.moc"
