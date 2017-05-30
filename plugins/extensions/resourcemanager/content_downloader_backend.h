/*
 *  Copyright (c) 2016 Aniketh Girish anikethgireesh@gmail.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef CONTENT_DOWNLOADER_BACKEND_H
#define CONTENT_DOWNLOADER_BACKEND_H

#include <QWidget>

#include <KoResourceServer.h> //backend of resources are coming from this

//KNSCore Includes
#include <KNSCore/EntryInternal>

class KoResource;
class KNSResourceDownloader; //KNS resource class needed to be added.

namespace KNSCore
{
class Engine;
}

class KNSDownloaderBackend: KoResourceServer //KoResource here as the backend
{
    Q_OBJECT
public:
    explicit KNSDownloaderBackend(QObject* parent, const QString& iconName, const QString &knsrc);
    ~KNSDownloaderBackend() override;

    void removeResource(KoResourceServer* app) override;
    void installResource(KoResourceServer* app) override;

    QString iconName() const { return m_iconName; }

    KNSCore::Engine* engine() const { return m_engine; }

    //Search needed to be implemented

    //loading the categories

Q_SIGNALS:
    void receivedResources(const QVector<KoResource> &resources); //act as the selected resource or the downloaded resource

public Q_SLOTS:
    void receivedEntries(const KNSCore::EntryInternal::List& entries);

private:
    KNSResource* resourceForEntry(const KNSCore::EntryInternal& entry);

    KNSCore::Engine* m_engine;
    QString m_name;
    QString m_iconName;
    QHash<QString, KoResource> m_resourceByName;

    QStringList m_categories; //Sorting between categories.


};
#endif // CONTENT_DOWNLOADER_BACKEND_H
