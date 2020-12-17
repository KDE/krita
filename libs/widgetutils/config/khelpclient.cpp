/*  This file is part of the KDE libraries
 *  SPDX-FileCopyrightText: 2012 David Faure <faure@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "khelpclient.h"

#include "kdesktopfile.h"

#include <QCoreApplication>
#include <QUrl>
#include <QDirIterator>
#include <QUrlQuery>
#include <QStandardPaths>
#include <QDesktopServices>

void KHelpClient::invokeHelp(const QString &anchor, const QString &_appname)
{
    QString appname;
    if (_appname.isEmpty()) {
        appname = QCoreApplication::instance()->applicationName();
    } else {
        appname = _appname;
    }

    // Look for the .desktop file of the application

    // was:
    //KService::Ptr service(KService::serviceByDesktopName(appname));
    //if (service)
    //    docPath = service->docPath();
    // but we don't want to depend on KService here.

    QString docPath;
    const QStringList desktopDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    Q_FOREACH (const QString &dir, desktopDirs) {
        QDirIterator it(dir, QStringList() << appname + QLatin1String(".desktop"), QDir::NoFilter, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            const QString desktopPath(it.next());
            KDesktopFile desktopFile(desktopPath);
            docPath = desktopFile.readDocPath();

            // TODO: why explicit break in a loop?
            break;
        }
    }

    // docPath could be a path or a full URL, I think.

    QUrl url;
    if (!docPath.isEmpty()) {
        url = QUrl(QLatin1String("help:/")).resolved(QUrl(docPath));
    } else {
        url = QUrl(QString::fromLatin1("help:/%1/index.html").arg(appname));
    }

    if (!anchor.isEmpty()) {
        QUrlQuery query(url);
        query.addQueryItem(QString::fromLatin1("anchor"), anchor);
        url.setQuery(query);
    }

    // launch khelpcenter, or a browser for URIs not handled by khelpcenter
    QDesktopServices::openUrl(url);
}

