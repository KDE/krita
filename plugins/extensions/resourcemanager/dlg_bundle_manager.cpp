/*
 *  Copyright (c) 2014 Victor Lafon metabolic.ewilan@hotmail.fr
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
#include "dlg_bundle_manager.h"
#include "ui_wdgdlgbundlemanager.h"

#include "resourcemanager.h"
#include "dlg_create_bundle.h"

#include <QListWidget>
#include <QTreeWidget>
#include <QListWidgetItem>
#include <QPainter>
#include <QPixmap>
#include <QMessageBox>
#include <QInputDialog>
#include <QItemSelectionModel>


#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <KoIcon.h>
#include <KoFileDialog.h>

#include <kis_icon.h>
#include "kis_action.h"
#include <KisResourceStorage.h>
#include <KisResourceServerProvider.h>
#include <KisStorageModel.h>
#include <KisStorageFilterProxyModel.h>
#include <kis_config.h>
#include <KisResourceLocator.h>


DlgBundleManager::DlgBundleManager(QWidget *parent)
    : KoDialog(parent)
    , m_page(new QWidget())
    , m_ui(new Ui::WdgDlgBundleManager)
{
    setCaption(i18n("Manage Resource Libraries"));
    m_ui->setupUi(m_page);
    setMainWidget(m_page);
    resize(m_page->sizeHint());

    m_ui->bnAdd->setIcon(KisIconUtils::loadIcon("list-add"));
    m_ui->bnAdd->setText(i18n("Import"));
    connect(m_ui->bnAdd, SIGNAL(clicked(bool)), SLOT(addBundle()));

    m_ui->bnNew->setIcon(KisIconUtils::loadIcon("document-new"));
    m_ui->bnNew->setText(i18n("Create"));
    connect(m_ui->bnNew, SIGNAL(clicked(bool)), SLOT(createBundle()));

    m_ui->bnDelete->setIcon(KisIconUtils::loadIcon("edit-delete"));
    m_ui->bnDelete->setText(i18n("Delete"));
    connect(m_ui->bnDelete, SIGNAL(clicked(bool)), SLOT(deleteBundle()));

    setButtons(Close);

    m_proxyModel = new KisStorageFilterProxyModel(this);
    m_proxyModel->setSourceModel(KisStorageModel::instance());
    m_proxyModel->setFilter(KisStorageFilterProxyModel::ByStorageType,
                          QStringList()
                          << KisResourceStorage::storageTypeToUntranslatedString(KisResourceStorage::StorageType::Bundle)
                          << KisResourceStorage::storageTypeToUntranslatedString(KisResourceStorage::StorageType::Folder));
    m_ui->tableView->setModel(m_proxyModel);

    m_ui->tableView->setColumnHidden(KisStorageModel::PreInstalled, true);
    m_ui->tableView->setColumnHidden(KisStorageModel::Id, true);
    m_ui->tableView->setColumnHidden(KisStorageModel::TimeStamp, true);

    QItemSelectionModel* selectionModel = m_ui->tableView->selectionModel();
    connect(selectionModel, &QItemSelectionModel::currentChanged, this, &DlgBundleManager::currentCellSelectedChanged);

    connect(KisStorageModel::instance(), &KisStorageModel::modelAboutToBeReset, this, &DlgBundleManager::slotModelAboutToBeReset);
    connect(KisStorageModel::instance(), &KisStorageModel::modelReset, this, &DlgBundleManager::slotModelReset);



}

void DlgBundleManager::addBundle()
{
    KoFileDialog* dlg = new KoFileDialog(this, KoFileDialog::OpenFile, i18n("Choose the bundle to import"));
    dlg->setCaption(i18n("Select the bundle"));
    QString filename = dlg->filename();
    if (!filename.isEmpty()) {
        addBundleToActiveResources(filename);
    }
}

void DlgBundleManager::createBundle()
{
    DlgCreateBundle* dlg = new DlgCreateBundle(0, this);
    dlg->exec();
}

void DlgBundleManager::deleteBundle()
{
    QModelIndex idx = m_ui->tableView->currentIndex();
    KIS_ASSERT(m_proxyModel);
    if (!idx.isValid()) {
        ENTER_FUNCTION() << "Index is invalid\n";
        return;
    }
    bool active = m_proxyModel->data(idx, Qt::UserRole + KisStorageModel::Active).toBool();
    idx = m_proxyModel->index(idx.row(), 0);
    m_proxyModel->setData(idx, QVariant(!active), Qt::CheckStateRole);
}

void DlgBundleManager::slotModelAboutToBeReset()
{
    ENTER_FUNCTION();
    lastIndex = QPersistentModelIndex(m_proxyModel->mapToSource(m_ui->tableView->currentIndex()));
    ENTER_FUNCTION() << ppVar(lastIndex) << ppVar(lastIndex.isValid());
}

void DlgBundleManager::slotModelReset()
{
    ENTER_FUNCTION();
    ENTER_FUNCTION() << ppVar(lastIndex) << ppVar(lastIndex.isValid());
    if (lastIndex.isValid()) {
        ENTER_FUNCTION() << "last index valid!";
        m_ui->tableView->setCurrentIndex(m_proxyModel->mapToSource(lastIndex));
    }
    lastIndex = QModelIndex();
}

void DlgBundleManager::currentCellSelectedChanged(QModelIndex current, QModelIndex previous)
{
    ENTER_FUNCTION() << "Current cell changed!";
    QModelIndex idx = m_ui->tableView->currentIndex();
    KIS_ASSERT(m_proxyModel);
    if (!idx.isValid()) {
        ENTER_FUNCTION() << "Index is invalid\n";
        return;
    }
    bool active = m_proxyModel->data(idx, Qt::UserRole + KisStorageModel::Active).toBool();

    if (active) {
        m_ui->bnDelete->setText(i18n("Deactivate"));
    } else {
        m_ui->bnDelete->setText(i18n("Activate"));
    }
}

QString createNewBundlePath(QString resourceFolder, QString filename)
{
    return resourceFolder + QDir::separator() + "bundles" + QDir::separator() + filename;
}

void DlgBundleManager::addBundleToActiveResources(QString filename)
{
    warnKrita << "DlgBundleManager::addBundle(): Loading a bundle is not implemented yet.";
    Q_UNUSED(filename);
    // 1. Copy the bundle to the resource folder
    // 2. Add the bundle as a storage/update database
    QFileInfo oldFileInfo(filename);

    KisConfig cfg(true);
    QString newDir = cfg.readEntry<QString>(KisResourceLocator::resourceLocationKey,
                                            QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    QString newName = oldFileInfo.fileName();
    QString newLocation = createNewBundlePath(newDir, newName);

    QFileInfo newFileInfo(newLocation);
    if (newFileInfo.exists()) {
        bool done = false;
        int i = 0;
        do {
            // ask for new filename
            bool ok;
            newName = QInputDialog::getText(this, i18n("New name for the bundle"), i18n("The old filename %1 is taken.\nNew name:", newName),
                                                    QLineEdit::Normal, newName, &ok);
            newLocation = createNewBundlePath(newDir, newName);
            newFileInfo.setFile(newLocation);
            done = !newFileInfo.exists();
            i++;
        } while (!done);
    }

    QFile::copy(filename, newLocation);
    KisResourceStorageSP storage = QSharedPointer<KisResourceStorage>::create(newLocation);
    KIS_ASSERT(!storage.isNull());
    KisResourceLocator::instance()->addStorage(newLocation, storage);
}
