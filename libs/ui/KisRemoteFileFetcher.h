/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
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
