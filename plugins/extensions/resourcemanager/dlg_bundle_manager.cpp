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

    KisStorageFilterProxyModel *proxyModel = new KisStorageFilterProxyModel(this);
    proxyModel->setSourceModel(KisStorageModel::instance());
    proxyModel->setFilter(KisStorageFilterProxyModel::ByStorageType,
                          QStringList()
                          << KisResourceStorage::storageTypeToUntranslatedString(KisResourceStorage::StorageType::Bundle)
                          << KisResourceStorage::storageTypeToUntranslatedString(KisResourceStorage::StorageType::Folder));
    m_ui->tableView->setModel(proxyModel);

}

void DlgBundleManager::addBundle()
{
    KoFileDialog* dlg = new KoFileDialog(this, KoFileDialog::OpenFile, i18n("Choose the bundle to import"));
    QString filename = dlg->filename();
    addBundleToActiveResources(filename);
}

void DlgBundleManager::createBundle()
{
    DlgCreateBundle* dlg = new DlgCreateBundle(0, this);
    dlg->exec();
}

void DlgBundleManager::deleteBundle()
{

}

void DlgBundleManager::addBundleToActiveResources(QString filename)
{
    warnKrita << "DlgBundleManager::addBundle(): Loading a bundle is not implemented yet.";
    Q_UNUSED(filename);
    // 1. Copy the bundle to the resource folder
    // 2. Add the bundle as a storage/update database
}
