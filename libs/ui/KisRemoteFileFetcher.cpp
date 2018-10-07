/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "KisRemoteFileFetcher.h"

#include <QApplication>
#include <QMessageBox>

#include <klocalizedstring.h>

KisRemoteFileFetcher::KisRemoteFileFetcher(QObject *parent)
    : QObject(parent)
    , m_request(0)
    , m_reply(0)
{
}

KisRemoteFileFetcher::~KisRemoteFileFetcher()
{
    delete m_request;
    delete m_reply;
}

bool KisRemoteFileFetcher::fetchFile(const QUrl &remote, QIODevice *io)
{
    Q_ASSERT(!remote.isLocalFile());

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    m_request = new QNetworkRequest(remote);
    m_request->setRawHeader("User-Agent", QString("Krita-%1").arg(qApp->applicationVersion()).toUtf8());
    m_reply = manager->get(*m_request);
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(error(QNetworkReply::NetworkError)));
    connect(m_reply, SIGNAL(finished()), &m_loop, SLOT(quit()));

    // Wait until done
    m_loop.exec();

    if (m_reply->error() != QNetworkReply::NoError) {
        QMessageBox::critical(0, i18nc("@title:window", "Krita"), QString("Could not download %1: %2").arg(remote.toDisplayString()).arg(m_reply->errorString()), QMessageBox::Ok);
        return false;
    }

    if (!io->isOpen()) {
        io->open(QIODevice::WriteOnly);
    }
    io->write(m_reply->readAll());
    io->close();

    return true;

}

void KisRemoteFileFetcher::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    //qDebug() << "bytesReceived" << bytesReceived << "bytesTotal" << bytesTotal;
}

void KisRemoteFileFetcher::error(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error);

    qDebug() << "error" << m_reply->errorString();
    m_loop.quit();
}
