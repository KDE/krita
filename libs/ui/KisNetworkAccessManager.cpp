/*
 * SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "KisNetworkAccessManager.h"

#include <QApplication>
#include <QLocale>
#include <QUrl>
#include <QNetworkReply>

KisNetworkAccessManager::KisNetworkAccessManager(QObject *parent)
    : QNetworkAccessManager(parent)
{
    setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
}

void KisNetworkAccessManager::getUrl(const QUrl &url)
{
    QNetworkRequest req;
    req.setUrl(url);
    get(req);
}

QNetworkReply* KisNetworkAccessManager::createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
    QString agentStr = QString::fromLatin1("%1/%2 (QNetworkAccessManager %3; %4; %5 bit)")
            .arg(qApp->applicationName())
            .arg(qApp->applicationVersion())
            .arg(QSysInfo::prettyProductName())
            .arg(QLocale::system().name())
            .arg(QSysInfo::WordSize);
    QNetworkRequest req(request);
    req.setRawHeader("User-Agent", agentStr.toLatin1());
    return QNetworkAccessManager::createRequest(op, req, outgoingData);
}
