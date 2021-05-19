/* This file is part of the KDE project
   SPDX-FileCopyrightText: 1998, 1999 Torben Weis <weis@kde.org>
   SPDX-FileCopyrightText: 2007 David Faure <faure@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
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
    QString requestedPath = QProcessEnvironment::systemEnvironment().value("KRITA_PLUGIN_PATH");
    if (!requestedPath.isEmpty()) {
        m_pluginPath = requestedPath;
    }
    else {

        QList<QDir> searchDirs;

        QDir appDir(qApp->applicationDirPath());
        appDir.cdUp();
#ifdef Q_OS_MACOS
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
#ifdef Q_OS_MACOS
                "*PlugIns*",
#endif
                "lib*",
            };
            Q_FOREACH (const QFileInfo &info, dir.entryInfoList(nameFilters, QDir::Dirs | QDir::NoDotAndDotDot)) {
#ifdef Q_OS_MACOS
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
#if defined(Q_PROCESSOR_ARM_64)
                    libDir.cd("arm64-v8a");
#elif defined(Q_PROCESSOR_ARM)
                    libDir.cd("armeabi-v7a");
#elif defined(Q_PROCESSOR_X86_64)
                    libDir.cd("x86_64");
#elif defined(Q_PROCESSOR_x86)
                    libDir.cd("x86");
#endif
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
