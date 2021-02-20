/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
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
        QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Krita"), QString("Could not download %1: %2").arg(remote.toDisplayString()).arg(m_reply->errorString()), QMessageBox::Ok);
        return false;
    }

    if (!io->isOpen()) {
        io->open(QIODevice::WriteOnly);
    }
    io->write(m_reply->readAll());
    io->close();

    return true;

}

void KisRemoteFileFetcher::downloadProgress(qint64 /*bytesReceived*/, qint64 /*bytesTotal*/)
{
    //qDebug() << "bytesReceived" << bytesReceived << "bytesTotal" << bytesTotal;
}

void KisRemoteFileFetcher::error(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error);

    qDebug() << "error" << m_reply->errorString();
    m_loop.quit();
}
