/*
 *  Copyright (c) 2014 Victor Lafon metabolic.ewilan@hotmail.fr
 *  Copyright (c) 2017 Aniketh Girish <anikethgireesh@gmail.com>
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
#ifndef DLG_BUNDLE_MANAGER_H
#define DLG_BUNDLE_MANAGER_H

#include <QWidget>

#include <KoDialog.h>
#include "kis_action_manager.h"
#include "resourcemanager.h"

class KisResourceBundle;
class QListWidget;
class QListWidgetItem;
class KoAbstractResourceServerAdapter;

namespace Ui
{
class WdgDlgBundleManager;
}

/**
 *
 * The bundle manager will present the bundle to the user in a list view.
 * A Tree view, Which shows the resources inside the bundle, when selected are shown in this tree view.
 * Preview images as well as other meta information like Bundle Name, Author, License, Description, can be seen.
 * Bundle manager provides the user the functionality to create a new bundle. Where the Create bundle functionality is
 * maintained by dlg_create_bundle class.
 * Options to delete the bundle created, Edit the bundle created are also available in the Bundle Manager.
 * User are capable of searching through the bundles they have created/Imported.
 * Other funtions to import different resources like Brushes, Bundles, Workspaces, Palettes, Patterns, Presets, Gradients.
 * Dialog which allows the user to remove Blacklisted file is available as Delete Backup files.
 * To get any item from the system folder there is an option to open-up the Resource Folder made available in the bundle manager.
 * The integration of Content Downloader is made available here.
 *
 */

class DlgBundleManager : public KoDialog
{
    Q_OBJECT
public:
    explicit DlgBundleManager(ResourceManager *resourceManager, KisActionManager* actionMgr, QWidget *parent = 0);

Q_SIGNALS:
    void resourceTextChanged(const QString &resourceText);

private Q_SLOTS:

    void accept() override;
    void itemSelected(QListWidgetItem *current, QListWidgetItem *previous);
    void itemSelected(QListWidgetItem *current);

    /// Opens up the dialog for creating a bundle.
    void slotCreateBundle();

    /// Opens the create bundle dialog box in Edit Mode. Which allows the user to edit the
    void editBundle();

    /// Dialog for cleaning up the blacklisted files.
    void slotDeleteBackupFiles();

    void slotImportResources();

    /// Opens a dialog pointing towards the Resource diectory.
    void slotOpenResourceFolder();

    /// This loads up the content downloader dialog when the Share Resource button is clicked.
    void slotShareResources();

    /// This sets the knsrcFile, the file which allows to link share.krita.org and this helps in populating the categories.
    void setKnsrcFile(const QString& knsrcFileArg);

    /// deleteBundle provides the provision to blackList the bundles and user could manually clean up the reasources.
    void deleteBundle();

    /// Allows to search the bundles created in the resource manager.
    void searchTextChanged(const QString &lineEditText);

private:

    QWidget *m_page;
    Ui::WdgDlgBundleManager *m_ui;

    /// Populates the listWidget in the Bundle manager with the bundles.
    void fillListWidget(QList<KisResourceBundle*> bundles, QListWidget *w);

    void refreshListData();

    QMap<QString, KisResourceBundle*> m_blacklistedBundles;
    QMap<QString, KisResourceBundle*> m_activeBundles;
    QMap<QString, KisResourceBundle*> m_Bundles;

    KisResourceBundle *m_currentBundle;
    KisActionManager *m_actionManager;
    ResourceManager *m_resourceManager;

    class Private;
    Private *const d;
};

#endif // DLG_BUNDLE_MANAGER_H
