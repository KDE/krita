/*
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
#ifndef DLG_CONTENT_DOWNLOADER_H
#define DLG_CONTENT_DOWNLOADER_H

#include <QWidget>

#include "dlg_content_downloader_p.h"

//KNS Includes

#include <KNSCore/EntryInternal>

using namespace KNSCore;

class DlgContentDownloaderPrivate;

/**
 * Using KNewStuffCore
 * Content Downloader widget.
 *
 * The content downloader widget will present items to the user
 * for installation, updates and removal.
 * Preview images as well as other meta information can be seen.
 *
 * \section knsrc knsrc Files
 * The Dialog is configured by a .knsrc file containing the KHotNewStuff configuration.
 * Your application should install a file called: <em>$KDEDIR/share/config/appname.knsrc</em>
 *
 * An example file could look like this:
 * <pre>
    [KNewStuff3]
    ProvidersUrl=https://share.krita.org/ocs/providers.xml
    Categories=Krita Brush
    XdgTargetDir=krita/brushes
    Uncompress=never
 * </pre>
 *
 * Uncompress can be one of: always, never or archive:
 * <ol>
 * <li>always: assume all downloaded files are archives and need to be extracted</li>
 * <li>never: never try to extract the file</li>
 * <li>archive: if the file is an archive, uncompress it, otherwise just pass it on</li>
 * <li>subdir: logic as archive, but decompress into a subdirectory named after the payload filename</li>
 * </ol>
 *
 * You have different options to set the target install directory:
 *   <ol><li>StandardResource: not available in KF5, use XdgTargetDir instead.</li>
 *       <li>TargetDir: since KF5, this is equivalent to XdgTargetDir.
 *       <li>XdgTargetDir: a directory in the $XDG_DATA_HOME directory such as <em>.local/share/krita</em>.
 *           This is what QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + name will return.</li>
 *   </ol>
 *
 */

class DlgContentDownloader: public QWidget
{
Q_OBJECT
public:
    /**
     * Create a ContentDownloader Widget that lets the user install, update and uninstall
     * contents. It will try to find a appname.knsrc file with the configuration.
     * Appname is the name of your application as provided in the about data->
     *
     * @param parent the parent of the dialog
     */
    explicit DlgContentDownloader(QWidget *parent = nullptr);

    /**
     * Create a Content Downloader Widget that lets the user install, update and uninstall
     * contents. Manually specify the name of a .knsrc file where the
     * KHotNewStuff configuration can be found.
     *
     * @param knsrc the name of the configuration file
     * @param parent the parent of the dialog
     */
    explicit DlgContentDownloader(const QString &knsrc, QWidget *parent = nullptr);

    /**
     * destructor
     */
    ~DlgContentDownloader();

    /**
     * The list of entries with changed status (installed/uninstalled)
     * @return the list of entries
     */
    EntryInternal::List changedEntries();

    /**
     * The list of entries that have been newly installed
     * @return the list of entries
     */
    EntryInternal::List installedEntries();

    /**
     * Set the title for display purposes in the widget's title.
     * @param title the title of the application (or category or whatever)
     */
    void setTitle(const QString &title);

    /**
     * Get the current title
     * @return the current title
     */
    QString title() const;

    KNSCore::Engine *engine();

private:
    void init(const QString &knsrc); //knsrc is the configfile

    DlgContentDownloaderPrivate *const d;
    Q_DISABLE_COPY(DlgContentDownloader)

    Q_PRIVATE_SLOT(d, void slotListViewListMode())
    Q_PRIVATE_SLOT(d, void slotListViewIconMode())

    Q_PRIVATE_SLOT(d, void slotProvidersLoaded())
    Q_PRIVATE_SLOT(d, void slotEntriesLoaded(const KNSCore::EntryInternal::List &entries))
    Q_PRIVATE_SLOT(d, void slotEntryChanged(const KNSCore::EntryInternal &entry))
    Q_PRIVATE_SLOT(d, void slotShowDetails(const KNSCore::EntryInternal &entry))
    Q_PRIVATE_SLOT(d, void slotShowOverview())

    Q_PRIVATE_SLOT(d, void slotPayloadFailed(const KNSCore::EntryInternal &entry))
    Q_PRIVATE_SLOT(d, void slotPayloadLoaded(QUrl url))

    Q_PRIVATE_SLOT(d, void slotResetMessage())
    Q_PRIVATE_SLOT(d, void slotNetworkTimeout())
    Q_PRIVATE_SLOT(d, void sortingChanged())
    Q_PRIVATE_SLOT(d, void slotSearchTextChanged())
    Q_PRIVATE_SLOT(d, void slotUpdateSearch())
    Q_PRIVATE_SLOT(d, void slotCategoryChanged(int))

    friend class ContentDownloaderDialog;
};

#endif // DLG_CONTENT_DOWNLOADER_H
