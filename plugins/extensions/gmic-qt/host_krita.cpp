/*
 * Copyright (C) 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <QApplication>
#include <QProcess>
#include <QByteArray>
#include <QDebug>
#include <QFileDialog>
#include <QSharedMemory>
#include <QFileInfo>
#include <QDesktopWidget>
#include <QLocalServer>
#include <QLocalSocket>
#include <QDataStream>

#include <algorithm>
#include "host.h"
#include "gmic_qt.h"
#include "gmic.h"

static const char ack[] = "ack";

namespace GmicQt {
    const QString HostApplicationName;
    const char *HostApplicationShortname = GMIC_QT_XSTRINGIFY(GMIC_HOST);
}


void gmic_qt_get_image_size(int * x, int * y)
{
    *x = gmic_qt_standalone::input_image.width();
    *y = gmic_qt_standalone::input_image.height();
}

void gmic_qt_get_layers_extends(int * width, int * height, GmicQt::InputMode )
{
    gmic_qt_get_image_size(width,height);
}

void gmic_qt_get_cropped_images(gmic_list<float> & images,
                                gmic_list<char> & imageNames,
                                double x, double y, double width, double height,
                                GmicQt::InputMode mode)
{
    // Send a message to Krita to ask for the images and image with the given crop and mode
    QLocalSocket socket;
    socket.connectToServer("krita-gmic");
    bool connected = socket.waitForConnected(1000);
    if (!connected) return;

    // Create a message for Krita
    QString message = QString("gmic_qt_get_cropped_images:%1,%2,%3,%4").arg(x).arg(y).arg(width).arg(height).arg(mode);
    QByteArray ba = message.toLatin1();

    // Send the message to Krita
    QDataStream ds(&socket);
    ds.writeBytes(ba.constData(), ba.length());
    bool r = socket.waitForBytesWritten();

    // get the return message; a list of
    QByteArray answer;

    while (socket->bytesAvailable() < static_cast<int>(sizeof(quint32))) {
        if (!socket->isValid()) { // stale request
            return;
        }
        socket->waitForReadyRead(1000);
    }

    QDataStream ds(&socket);
    quint32 remaining;
    ds >> remaining;
    answer.resize(remaining);
    int got = 0;
    char *answerBuf = answer.data();
    do {
        got = ds.readRawData(answerBuf, remaining);
        remaining -= got;
        answerBuf += got;
    } while (remaining && got >= 0 && socket->waitForReadyRead(2000));

    if (got < 0) {
        qWarning() << "Message reception failed" << socket.errorString();
        return;
    }
    socket.write(ack, qstrlen(ack));
    socket.waitForBytesWritten(1000);
    socket.disconnectFromServer();

    // Parse the answer -- there should be no new lines in layernames
    QStringList memoryKeys;
    Q_FOREACH(const QString &layer, QString::fromUtf8(answer).split("\n")) {
        qDebug() << layer;
        keys << layer.split(',')[0];
        //imageNames[i] = layer.toLatin1().data();
    }
    // Fill images and imageNames
    for (int i = 0; i < memoryKeys.length(); ++i) {
        const QString &key = memoryKeys[i];
        QSharedMemory m(key);
        if (m.isAttached()) {
            const void *constData = m.constData();
            int size = m.size();
            // convert the data to the list of float
            // images[i] = gmic_image()
        }
        else {
            qWarning() << "Could not attach to shared memory area.";
        }
    }
}

void gmic_qt_output_images( gmic_list<float> & images,
                            const gmic_list<char> & imageNames,
                            GmicQt::OutputMode mode,
                            const char * verboseLayersLabel )
{
    // Convert the images (if necessary) to play arrays of float
    // Create qsharedmemory segments for each image

    // Create a message for Krita based on mode, the keys of the qsharedmemory segments and the imageNames
    QString message = QString("gmic_qt_output_images:blablabla");
    QByteArray ba = message.toLatin1();

    // Create a socket
    QLocalSocket socket;
    socket.connectToServer("krita-gmic");
    bool connected = socket.waitForConnected(1000);
    if (!connected) return;

    // Send the message to Krita
    QDataStream ds(&socket);
    ds.writeBytes(ba.constData(), ba.length());
    bool r = socket.waitForBytesWritten();

    // Wait for the ack
    r &= socket.waitForReadyRead(timeout); // wait for ack
    r &= (socket.read(qstrlen(ack)) == ack);
    socket.waitForDisconnected(-1);
}


int main(int argc, char * argv[])
{
    GmicQt::HostApplicationName = "Krita";
    return launchPlugin();
}
