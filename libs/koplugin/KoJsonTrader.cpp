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

#include <QDebug>

#include <QApplication>
#include <QList>
#include <QPluginLoader>
#include <QJsonObject>
#include <QJsonArray>
#include <QDirIterator>
#include <QDir>

KoJsonTrader::KoJsonTrader()
{
}

KoJsonTrader* KoJsonTrader::self()
{
    static KoJsonTrader *s_instance = 0;
    if (!s_instance)
        s_instance = new KoJsonTrader();
    return s_instance;
}

QList<QPluginLoader *> KoJsonTrader::query(const QString &servicetype, const QString &mimetype)
{

    if (m_pluginPath.isEmpty()) {

        QList<QDir> searchDirs;

        QDir appDir(qApp->applicationDirPath());
        appDir.cdUp();

        searchDirs << appDir;
#ifdef CMAKE_INSTALL_PREFIX
        searchDirs << QDir{CMAKE_INSTALL_PREFIX};
#endif

        foreach(const QDir& dir, searchDirs) {
            foreach(QString entry, dir.entryList()) {
                QFileInfo info(dir, entry);
                if (info.isDir() && info.fileName().contains("lib")) {
                    QDir libDir(info.absoluteFilePath());

                    // on many systems this will be the actual lib dir (and calligra subdir contains plugins)
                    if (libDir.entryList(QStringList() << "calligra").size() > 0) {
                        m_pluginPath = info.absoluteFilePath() + "/calligra";
                        break;
                    }

                    // on debian at least the actual libdir is a subdir named like "lib/x86_64-linux-gnu"
                    // so search there for the calligra subdir which will contain our plugins
                    foreach(QString subEntry, libDir.entryList()) {
                        QFileInfo subInfo(libDir, subEntry);
                        if (subInfo.isDir()) {
                            if (QDir(subInfo.absoluteFilePath()).entryList(QStringList() << "calligra").size() > 0) {
                                m_pluginPath = subInfo.absoluteFilePath() + "/calligra";
                                break; // will only break inner loop so we need the extra check below
                            }
                        }
                    }

                    if (!m_pluginPath.isEmpty()) {
                        break;
                    }
                }
            }

            if(!m_pluginPath.isEmpty()) {
                break;
            }
        }
        //qDebug() << "KoJsonTrader will load its plugins from" << m_pluginPath;

    }

    QList<QPluginLoader *>list;
    QDirIterator dirIter(m_pluginPath, QDirIterator::Subdirectories);
    while (dirIter.hasNext()) {
        dirIter.next();
        if (dirIter.fileInfo().isFile()) {
            QPluginLoader *loader = new QPluginLoader(dirIter.filePath());
            QJsonObject json = loader->metaData().value("MetaData").toObject();

            if (json.isEmpty()) {
                qDebug() << dirIter.filePath() << "has no json!";
            }
            if (!json.isEmpty()) {
                QJsonObject pluginData = json.value("KPlugin").toObject();
                if (!pluginData.value("ServiceTypes").toArray().contains(QJsonValue(servicetype))) {
                    continue;
                }

                if (!mimetype.isEmpty()) {
                    QStringList mimeTypes = json.value("X-KDE-ExtraNativeMimeTypes").toString().split(',');
                    mimeTypes += json.value("MimeType").toString().split(';');
                    mimeTypes += json.value("X-KDE-NativeMimeType").toString();
                    if (! mimeTypes.contains(mimetype)) {
                        continue;
                    }
                }
                list.append(loader);
            }
        }

    }

    return list;
}
