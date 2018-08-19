/*  This file is part of the KDE libraries
 *  Copyright 2012 David Faure <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License or ( at
 *  your option ) version 3 or, at the discretion of KDE e.V. ( which shall
 *  act as a proxy as in section 14 of the GPLv3 ), any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
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

