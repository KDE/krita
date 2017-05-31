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
#ifndef DLG_CONTENT_DOWNLOADER_H
#define DLG_CONTENT_DOWNLOADER_H

#include <QWidget>

#include <KoResourceServer.h> //The backend for resource selection

//KNS Includes

#include <KNSCore/EntryInternal>
#include <KNSCore/Author>
#include <KNSCore/Engine>

class Private;
class KNSResourceDownloader: public KoResourceServer
{
Q_OBJECT
public:
    explicit KNSResourceDownloader(const KNSCore::EntryInternal & c, const KNSCore::Author & a, QStringList categories, const QString &knsrc, QObject* parent);
    ~KNSResourceDownloader();

    // KoResourceServer::state state() override; this is to verify the state of the content. need to implement, ie, enumaration stating that if the package is downloading, processing etc.

    QString name();
    QString packagename() const; // Have a doubt if name() and this isn't similar? KNSCore issue

//    QVariant icon() const override;

    QString author();
    QStringList categories(); //Need to implement. No idea yet.
    int downloadcount(); //no of downloads done
    QString source(); //orgin / providerID
    QString license();
    QString details(); //Long description

//    QString iconName() const { return m_iconName; }

    void downloadResources(KoResourceServer* app);

//    void removeResources(KoResourceServer* res);

    void setKnsrcFile(const QString& knsrcFileArg);

//    QUrl thumbnailUrl() override;  // If needed.
//    QUrl screenshotUrl() override; // If needed.
//    void fetchScreenshots() override; //if needed.

    KoResourceServer* koresourceserver() const;

    KNSCore::EntryInternal entry() const;
    KNSCore::Engine* engine() const { return m_engine; }

private:
    const QStringList m_categories;
//    QString m_iconName;

    KNSCore::EntryInternal m_entry;
    KNSCore::Author m_author;
    KNSCore::Engine* m_engine;

    class Private;

    Private * const d;

};


#endif // DLG_CONTENT_DOWNLOADER_H
