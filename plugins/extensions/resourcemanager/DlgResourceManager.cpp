/*
 * SPDX-FileCopyrightText: 2021 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "DlgResourceManager.h"

#include "ui_WdgDlgResourceManager.h"

#include <KisResourceTypeModel.h>
#include <KisStorageModel.h>
#include <KisTagModel.h>
#include <KisResourceModel.h>
#include <KisTagFilterResourceProxyModel.h>
#include <kis_assert.h>
#include <KisResourceItemDelegate.h>
#include <QItemSelection>
#include <wdgtagselection.h>


DlgResourceManager::DlgResourceManager(QWidget *parent)
    : KoDialog(parent)
    , m_ui(new Ui::WdgDlgResourceManager)
    , m_tagsController(0)
{
    setCaption(i18n("Manage Resources"));
    m_page = new QWidget(this);
    m_ui->setupUi(m_page);
    resize(m_page->size());
    setMainWidget(m_page);
    setButtons(Close);
    setDefaultButton(Close);

    m_resourceTypeModel = new KisResourceTypeModel(this);

    m_ui->cmbResourceType->setModel(m_resourceTypeModel);
    m_ui->cmbResourceType->setModelColumn(KisResourceTypeModel::Name);
    connect(m_ui->cmbResourceType, SIGNAL(activated(int)), SLOT(slotResourceTypeSelected(int)));

    m_storageModel = new KisStorageModel(this);

    m_ui->cmbStorage->setModel(m_storageModel);
    m_ui->cmbStorage->setModelColumn(KisStorageModel::DisplayName);
    connect(m_ui->cmbStorage, SIGNAL(activated(int)), SLOT(slotStorageSelected(int)));

    QString selectedResourceType = getCurrentResourceType();

    KisTagModel* tagModel = new KisTagModel(selectedResourceType);
    m_tagModelsForResourceType.insert(selectedResourceType, tagModel);

    m_ui->cmbTag->setModel(tagModel);
    m_ui->cmbTag->setModelColumn(KisAllTagsModel::Name);
    connect(m_ui->cmbTag, SIGNAL(activated(int)), SLOT(slotTagSelected(int)));

    KisResourceModel* resourceModel = new KisResourceModel(selectedResourceType);
    resourceModel->setStorageFilter(KisResourceModel::ShowAllStorages);
    resourceModel->setResourceFilter(m_ui->chkShowDeleted->isChecked() ? KisResourceModel::ShowAllResources : KisResourceModel::ShowActiveResources);
    m_resourceModelsForResourceType.insert(selectedResourceType, resourceModel);

    KisTagFilterResourceProxyModel* proxyModel = new KisTagFilterResourceProxyModel(selectedResourceType);
    proxyModel->setResourceModel(resourceModel);
    proxyModel->setTagFilter(0);
    proxyModel->setStorageFilter(true, getCurrentStorageId());
    m_resourceProxyModelsForResourceType.insert(selectedResourceType, proxyModel);

    m_ui->resourceItemView->setModel(proxyModel);
    m_ui->resourceItemView->setItemDelegate(new KisResourceItemDelegate(this));
    m_ui->resourceItemView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(m_ui->resourceItemView, SIGNAL(currentResourceChanged(QModelIndex)), SLOT(slotResourcesSelectionChanged(QModelIndex)));

    connect(m_ui->btnImportBundle, SIGNAL(clicked(bool)), SLOT(slotImportBundle()));
    connect(m_ui->btnOpenResourceFolder, SIGNAL(clicked(bool)), SLOT(slotOpenResourceFolder()));
    connect(m_ui->btnImportResources, SIGNAL(clicked(bool)), SLOT(slotImportResources()));
    connect(m_ui->btnDeleteBackupFiles, SIGNAL(clicked(bool)), SLOT(slotDeleteBackupFiles()));

    connect(m_ui->lneFilterText, SIGNAL(textChanged(const QString&)), SLOT(slotFilterTextChanged(const QString&)));

    m_tagsController.reset(new KisWdgTagSelectionControllerOneResource(m_ui->wdgResourcesTags, true));

}

DlgResourceManager::~DlgResourceManager()
{

}

void DlgResourceManager::slotResourceTypeSelected(int)
{
    QString selectedResourceType = getCurrentResourceType();
    if (!m_tagModelsForResourceType.contains(selectedResourceType)) {
        m_tagModelsForResourceType.insert(selectedResourceType, new KisTagModel(selectedResourceType));
    }

    m_ui->cmbTag->setModel(m_tagModelsForResourceType[selectedResourceType]);

    if (!m_resourceProxyModelsForResourceType.contains(selectedResourceType)) {
        KisResourceModel* resourceModel;
        if (!m_resourceModelsForResourceType.contains(selectedResourceType)) {
            resourceModel = new KisResourceModel(selectedResourceType);
            KIS_SAFE_ASSERT_RECOVER_RETURN(resourceModel);
            resourceModel->setStorageFilter(KisResourceModel::ShowAllStorages);
            resourceModel->setResourceFilter(m_ui->chkShowDeleted->isChecked() ? KisResourceModel::ShowAllResources : KisResourceModel::ShowActiveResources);

            m_resourceModelsForResourceType.insert(selectedResourceType, resourceModel);
        } else {
            resourceModel = m_resourceModelsForResourceType[selectedResourceType];
        }

        KisTagFilterResourceProxyModel* proxyModel = new KisTagFilterResourceProxyModel(selectedResourceType);
        KIS_SAFE_ASSERT_RECOVER_RETURN(proxyModel);
        proxyModel->setResourceModel(resourceModel);
        proxyModel->setTagFilter(getCurrentTag());
        proxyModel->setStorageFilter(true, getCurrentStorageId());
        m_resourceProxyModelsForResourceType.insert(selectedResourceType, proxyModel);
    }
    m_ui->resourceItemView->setModel(m_resourceProxyModelsForResourceType[selectedResourceType]);
}

void DlgResourceManager::slotStorageSelected(int)
{
    if (!m_resourceProxyModelsForResourceType.contains(getCurrentResourceType())) {
        return;
    }
    m_resourceProxyModelsForResourceType[getCurrentResourceType()]->setStorageFilter(true, getCurrentStorageId());
}

void DlgResourceManager::slotTagSelected(int)
{
    if (!m_resourceProxyModelsForResourceType.contains(getCurrentResourceType())) {
        return;
    }
    m_resourceProxyModelsForResourceType[getCurrentResourceType()]->setTagFilter(getCurrentTag());
}

void DlgResourceManager::slotResourcesSelectionChanged(QModelIndex index)
{
    QModelIndexList list = m_ui->resourceItemView->selectionModel()->selection().indexes();
    KisTagFilterResourceProxyModel* model = m_resourceProxyModelsForResourceType[getCurrentResourceType()];
    if (list.size() == 1) {
        QModelIndex idx = list[0];
        m_ui->lblFilename->setText(model->data(idx, Qt::UserRole + KisAllResourcesModel::Filename).toString());
        m_ui->lneName->setText(model->data(idx, Qt::UserRole + KisAllResourcesModel::Name).toString());
        m_ui->lblLocation->setText(model->data(idx, Qt::UserRole + KisAllResourcesModel::Location).toString());
        m_ui->lblId->setText(model->data(idx, Qt::UserRole + KisAllResourcesModel::Id).toString());

        QImage thumb = model->data(idx, Qt::UserRole + KisAllResourcesModel::Thumbnail).value<QImage>();
        thumb.setDevicePixelRatio(this->devicePixelRatioF());
        qCritical() << thumb.size() << "and should be " << m_ui->lblThumbnail->size();
        QPixmap pix = QPixmap::fromImage(thumb);
        pix = pix.scaled(m_ui->lblThumbnail->size()*devicePixelRatioF(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_ui->lblThumbnail->setPixmap(pix);

        m_ui->lblFilename->setDisabled(false);
        m_ui->lblLocation->setDisabled(false);
        m_ui->lblThumbnail->setDisabled(false);
        m_ui->lneName->setDisabled(false);
        m_ui->lblId->setDisabled(false);

        int resourceId = model->data(idx, Qt::UserRole + KisAllResourcesModel::Id).toInt();
        m_tagsController->setResourceId(getCurrentResourceType(), resourceId);

    } else {
        QString multipleSelectedText = i18nc("In Resource manager, this is text shown instead of filename, name or location, "
                                             "when multiple resources are shown so there is no one specific filename", "(Multiple selected)");
        m_ui->lblFilename->setText(multipleSelectedText);
        m_ui->lblLocation->setText(multipleSelectedText);
        m_ui->lneName->setText(multipleSelectedText);
        m_ui->lblThumbnail->setText(multipleSelectedText);
        QPixmap pix;
        m_ui->lblThumbnail->setPixmap(pix);

        m_ui->lblFilename->setDisabled(true);
        m_ui->lblLocation->setDisabled(true);
        m_ui->lblThumbnail->setDisabled(true);
        m_ui->lneName->setDisabled(true);
        m_ui->lblId->setDisabled(false);

        m_tagsController->setResourceId(getCurrentResourceType(), -1);
    }
    Q_FOREACH(QModelIndex index, list) {

    }
}

void DlgResourceManager::slotFilterTextChanged(const QString &filterText)
{
    if (m_resourceProxyModelsForResourceType.contains(getCurrentResourceType())) {
        m_resourceProxyModelsForResourceType[getCurrentResourceType()]->setSearchText(filterText);
    }
}

void DlgResourceManager::slotImportResources()
{

}

void DlgResourceManager::slotOpenResourceFolder()
{

}

void DlgResourceManager::slotImportBundle()
{

}

void DlgResourceManager::slotDeleteBackupFiles()
{

}

QString DlgResourceManager::getCurrentResourceType()
{
    return m_ui->cmbResourceType->currentData(Qt::UserRole + KisResourceTypeModel::ResourceType).toString();
}

int DlgResourceManager::getCurrentStorageId()
{
    return m_ui->cmbStorage->currentData(Qt::UserRole + KisStorageModel::Id).toInt();
}

QSharedPointer<KisTag> DlgResourceManager::getCurrentTag()
{
    return m_ui->cmbTag->currentData(Qt::UserRole + KisAllTagsModel::KisTagRole).value<KisTagSP>();
}
