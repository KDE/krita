/*
 *  Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
 *  Copyright (c) 2003 - 2007 Josef Spillner <spillner@kde.org>
 *  Copyright (c) 2009 Jeremy Whiting <jpwhiting@kde.org>
 *  Copyright (C) 2009 Frederik Gladhorn <gladhorn@kde.org>
 *  Copyright (c) 2017 Aniketh Girish anikethgireesh@gmail.com
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

#ifndef ENTRY_H
#define ENTRY_H

#include <QtCore/QStringList>
#include <QtCore/QSharedDataPointer>
#include <QLoggingCategory>

namespace KNSCore { class EntryInternal; }

class EntryPrivate;
/**
 *
 * This class provides information about the entries that
 * have been installed while the content dialog was shown.
 *
 */
class Entry
{
public:
    typedef QList<Entry> List;

    /**
    * Status of the entry. An entry will be downloadable from the provider's
    * site prior to the download. Once downloaded and installed, it will
    * be either installed or updateable, implying an out-of-date
    * installation. Finally, the entry can be deleted and hence show up as
    * downloadable again.
    * Entries not taking part in this cycle, for example those in upload,
    * have an invalid status.
    */
    enum Status {
        Invalid,
        Downloadable,
        Installed,
        Updateable,
        Deleted,
        Installing,
        Updating
    };

    ~Entry();
    Entry(const Entry &other);
    Entry &operator=(const Entry &other);

    /**
     * Retrieve the name of the data object.
     *
     * @return object name
     */
    QString name() const;

    /**
     * Retrieve the category of the data object.
     *
     * @return object category
     */
    QString category() const;

    /**
     * Retrieve the locally installed files.
     * @return file names
     */
    QStringList installedFiles() const;

    /**
     * Retrieve the locally uninstalled files.
     * @return file names
     */
    QStringList uninstalledFiles() const;

    /**
     * Retrieves the entry's status.
     *
     * @return Current status of the entry
     */
    Status status() const;

    /**
    * Retrieve the license name of the object.
    *
    * @return object license
    */
    QString license() const;

    /**
    * Retrieve a short description about the object.
    *
    * @return object description
    */
    QString summary() const;

    /**
    * Retrieve the version string of the object.
    *
    * @return object version
    *
    * @sa installedVersion()
    */
    QString version() const;

    /**
     * Id of this Entry. It is guaranteed to be unique for one provider.
     * Id and ProviderId together identifiy this entry.
     * @return the id
     * @since 4.5
     */
    QString id() const;

    /**
     * The Provider which is the source of the Entry.
     * @return the Id of the Provider
     * @since 4.5
     */
    QString providerId() const;

    /**
     * @returns if available an url identifying the asset
     */
    QUrl url() const;

    /**
     * @returns a list of urls to small previews to be displayed as thumbnails
     */
    QList<QUrl> previewThumbnails() const;

    /**
     * @returns a list of full previews of the asset
     */
    QList<QUrl> previewImages() const;

    /**
     * @returns the advertised disk size of the asset
     */
    quint64 size() const;

    /**
     * @returns the number of comments in the asset
     */
    uint numberOfComments() const;

    /**
     * @returns the rating of the asset, between 0 and 100
     */
    uint rating() const;

    /**
     * @returns the asset's change log
     */
    QString changelog() const;

    /**
     * @returns a short one-line summary of the asset
     */
    QString shortSummary() const;

    /**
     * @returns the available version
     *
     * If the entry is not updateable, it will be the same as version.
     *
     * @sa version()
     *
     */
    QString updateVersion() const;

private:
    Entry();

    QExplicitlySharedDataPointer<EntryPrivate> d;

    friend class KNSCore::EntryInternal;
    friend class EntryPrivate;
};

#endif // ENTRY_H
