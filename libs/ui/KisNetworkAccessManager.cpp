/*
 * Copyright (c) 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "KisNetworkAccessManager.h"

#include <QApplication>
#include <QLocale>
#include <QUrl>
#include <QNetworkReply>
#include <QDebug>

KisNetworkAccessManager::KisNetworkAccessManager(QObject *parent)
    : QNetworkAccessManager(parent)
{
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
