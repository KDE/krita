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
#ifndef KISREMOTEFILEFETCHER_H
#define KISREMOTEFILEFETCHER_H

#include <QObject>
#include <QUrl>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QList>
#include <QSslError>
#include <QEventLoop>
#include <QIODevice>

/**
 * @brief The KisRemoteFileFetcher class can fetch a remote file and blocks until the file is downloaded
 */
class KisRemoteFileFetcher : public QObject
{
    Q_OBJECT
public:
    explicit KisRemoteFileFetcher(QObject *parent = 0);
    ~KisRemoteFileFetcher() override;
    bool fetchFile(const QUrl &remote, QIODevice *io);

private Q_SLOTS:

    void downloadProgress(qint64 received,qint64 total);
    void error(QNetworkReply::NetworkError error);

private:
    QEventLoop m_loop;
    QNetworkRequest *m_request;
    QNetworkReply *m_reply;
};

#endif // KISREMOTEFILEFETCHER_H
