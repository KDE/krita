/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright     2007       David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoJsonTrader.h"

#include "KritaPluginDebug.h"

#include <QCoreApplication>
#include <QPluginLoader>
#include <QJsonObject>
#include <QJsonArray>
#include <QDirIterator>
#include <QDir>
#include <QProcessEnvironment>
#include <QGlobalStatic>
#include <QMutexLocker>

KoJsonTrader::KoJsonTrader()
{
    // Allow a command line variable KRITA_PLUGIN_PATH to override the automatic search
    auto requestedPath = QProcessEnvironment::systemEnvironment().value("KRITA_PLUGIN_PATH");
    if (!requestedPath.isEmpty()) {
        m_pluginPath = requestedPath;
    }
    else {

        QList<QDir> searchDirs;

        QDir appDir(qApp->applicationDirPath());
        appDir.cdUp();
#ifdef Q_OS_OSX
        // Help Krita run without deployment
        QDir d(appDir);
        d.cd("../../../");
        searchDirs << d;
#endif

#ifdef Q_OS_ANDROID
        appDir.cdUp();
#endif
        searchDirs << appDir;
        // help plugin trader find installed plugins when run from uninstalled tests
#ifdef CMAKE_INSTALL_PREFIX
        searchDirs << QDir(CMAKE_INSTALL_PREFIX);
#endif
        Q_FOREACH (const QDir& dir, searchDirs) {
            const QStringList nameFilters = {
#ifdef Q_OS_OSX
                "*PlugIns*",
#endif
                "lib*",
            };
            Q_FOREACH (const QFileInfo &info, dir.entryInfoList(nameFilters, QDir::Dirs | QDir::NoDotAndDotDot)) {
#ifdef Q_OS_OSX
                if (info.fileName().contains("PlugIns")) {
                    m_pluginPath = info.absoluteFilePath();
                    break;
                }
                else if (info.fileName().contains("lib")) {
#else
                if (info.fileName().contains("lib")) {
#endif
                    QDir libDir(info.absoluteFilePath());

#ifdef Q_OS_ANDROID
                    libDir.cd("arm");
                    m_pluginPath = libDir.absolutePath();
                    break;
#else
                    // on many systems this will be the actual lib dir (and krita subdir contains plugins)
                    if (libDir.cd("kritaplugins")) {
                        m_pluginPath = libDir.absolutePath();
                        break;
                    }

                    // on debian at least the actual libdir is a subdir named like "lib/x86_64-linux-gnu"
                    // so search there for the Krita subdir which will contain our plugins
                    // FIXME: what are the chances of there being more than one Krita install with different arch and compiler ABI?
                    Q_FOREACH (const QFileInfo &subInfo, libDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
                        QDir subDir(subInfo.absoluteFilePath());
                        if (subDir.cd("kritaplugins")) {
                            m_pluginPath = subDir.absolutePath();
                            break; // will only break inner loop so we need the extra check below
                        }
                    }

                    if (!m_pluginPath.isEmpty()) {
                        break;
                    }
#endif
                }
            }

            if (!m_pluginPath.isEmpty()) {
                break;
            }
        }
        debugPlugin << "KoJsonTrader will load its plugins from" << m_pluginPath;
    }
}

Q_GLOBAL_STATIC(KoJsonTrader, s_instance)

KoJsonTrader* KoJsonTrader::instance()
{
    return s_instance;
}

QList<QPluginLoader *> KoJsonTrader::query(const QString &servicetype, const QString &mimetype) const
{
    QMutexLocker l(&m_mutex);

    QList<QPluginLoader *>list;
    QDirIterator dirIter(m_pluginPath, QDirIterator::Subdirectories);
    while (dirIter.hasNext()) {
        dirIter.next();
#ifdef Q_OS_ANDROID
        // files starting with lib_krita are plugins, it is needed because of the loading rules in NDK
        if (dirIter.fileInfo().isFile() && dirIter.fileName().startsWith("lib_krita")) {
#else
        if (dirIter.fileInfo().isFile() && dirIter.fileName().startsWith("krita") && !dirIter.fileName().endsWith(".debug")) {
#endif
            debugPlugin << dirIter.fileName();
            QPluginLoader *loader = new QPluginLoader(dirIter.filePath());
            QJsonObject json = loader->metaData().value("MetaData").toObject();

            debugPlugin << mimetype << json << json.value("X-KDE-ServiceTypes");

            if (json.isEmpty()) {
                delete loader;
                qWarning() << dirIter.filePath() << "has no json!";
            }
            else {
                QJsonArray  serviceTypes = json.value("X-KDE-ServiceTypes").toArray();
                if (serviceTypes.isEmpty()) {
                    qWarning() << dirIter.fileName() << "has no X-KDE-ServiceTypes";
                }
                if (!serviceTypes.contains(QJsonValue(servicetype))) {
                    delete loader;
                    continue;
                }

                if (!mimetype.isEmpty()) {
                    QStringList mimeTypes = json.value("X-KDE-ExtraNativeMimeTypes").toString().split(',');
                    mimeTypes += json.value("MimeType").toString().split(';');
                    mimeTypes += json.value("X-KDE-NativeMimeType").toString();
                    if (! mimeTypes.contains(mimetype)) {
                        qWarning() << dirIter.filePath() << "doesn't contain mimetype" << mimetype << "in" << mimeTypes;
                        delete loader;
                        continue;
                    }
                }
                list.append(loader);
            }
        }

    }
    return list;
}
