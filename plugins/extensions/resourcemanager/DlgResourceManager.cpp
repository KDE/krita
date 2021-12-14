/*
 * SPDX-FileCopyrightText: 2021 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "DlgResourceManager.h"

#include "ui_WdgDlgResourceManager.h"

#include <QItemSelection>
#include <QPainter>

#include <kis_action.h>
#include <kis_action_manager.h>
#include <KisResourceTypeModel.h>
#include <KisStorageModel.h>
#include <KisTagModel.h>
#include <KisResourceModel.h>
#include <KisTagFilterResourceProxyModel.h>
#include <kis_assert.h>
#include <KisResourceItemDelegate.h>
#include <wdgtagselection.h>
#include <kis_paintop_factory.h>
#include <kis_paintop_registry.h>
#include <dlg_create_bundle.h>
#include <ResourceImporter.h>
#include <KisResourceLocator.h>

DlgResourceManager::DlgResourceManager(KisActionManager *actionMgr, QWidget *parent)
    : KoDialog(parent)
    , m_ui(new Ui::WdgDlgResourceManager)
    , m_actionManager(actionMgr)
    , m_tagsController(0)
{
    setCaption(i18n("Manage Resources"));
    m_page = new QWidget(this);
    m_ui->setupUi(m_page);
    resize(m_page->size());
    setMainWidget(m_page);
    setButtons(Close);
    setDefaultButton(Close);

    m_ui->resourceItemView->setFixedToolTipThumbnailSize(QSize(128, 128));

    m_resourceTypeModel = new KisResourceTypeModel(this);

    m_ui->cmbResourceType->setModel(m_resourceTypeModel);
    m_ui->cmbResourceType->setModelColumn(KisResourceTypeModel::Name);
    for (int i = 0; i < m_resourceTypeModel->rowCount(); i++) {
        QModelIndex idx = m_resourceTypeModel->index(i, 0);
        QString resourceType = m_resourceTypeModel->data(idx, Qt::UserRole + KisResourceTypeModel::ResourceType).toString();
        if (resourceType == "paintoppresets") {
            m_ui->cmbResourceType->setCurrentIndex(i);
            break;
        }
    }
    connect(m_ui->cmbResourceType, SIGNAL(activated(int)), SLOT(slotResourceTypeSelected(int)));

    m_storageModel = new KisStorageModel(this);

    m_ui->cmbStorage->setModel(m_storageModel);
    m_ui->cmbStorage->setModelColumn(KisStorageModel::DisplayName);
    connect(m_ui->cmbStorage, SIGNAL(activated(int)), SLOT(slotStorageSelected(int)));

    QString selectedResourceType = getCurrentResourceType();

    KisTagModel* tagModel = new KisTagModel(selectedResourceType);
    tagModel->sort(KisAllTagsModel::Name);
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
    proxyModel->sort(KisAbstractResourceModel::Name);
    m_resourceProxyModelsForResourceType.insert(selectedResourceType, proxyModel);

    m_ui->resourceItemView->setModel(proxyModel);
    m_ui->resourceItemView->setItemDelegate(new KisResourceItemDelegate(this));
    m_ui->resourceItemView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(m_ui->resourceItemView, SIGNAL(currentResourceChanged(QModelIndex)), SLOT(slotResourcesSelectionChanged(QModelIndex)));

    connect(m_ui->btnDeleteResource, SIGNAL(clicked(bool)), SLOT(slotDeleteResources()));
    connect(m_ui->btnCreateBundle, SIGNAL(clicked(bool)), SLOT(slotCreateBundle()));
    connect(m_ui->btnOpenResourceFolder, SIGNAL(clicked(bool)), SLOT(slotOpenResourceFolder()));
    connect(m_ui->btnImportResources, SIGNAL(clicked(bool)), SLOT(slotImportResources()));
    connect(m_ui->btnExtractTagsToResourceFolder, SIGNAL(clicked(bool)), SLOT(slotSaveTags()));

    connect(m_ui->lneFilterText, SIGNAL(textChanged(const QString&)), SLOT(slotFilterTextChanged(const QString&)));
    connect(m_ui->chkShowDeleted, SIGNAL(stateChanged(int)), SLOT(slotShowDeletedChanged(int)));

    m_tagsController.reset(new KisWdgTagSelectionControllerOneResource(m_ui->wdgResourcesTags, true));

#ifdef Q_OS_ANDROID
    // TODO(sh_zam): Opening a directory can cause a crash. A ContentProvider is needed for this.
    m_ui->btnOpenResourceFolder->setEnabled(false);
#endif
}

DlgResourceManager::~DlgResourceManager()
{
    // TODO: we are leaking models and proxies!!!
}

void DlgResourceManager::slotResourceTypeSelected(int)
{
    QString selectedResourceType = getCurrentResourceType();
    if (!m_tagModelsForResourceType.contains(selectedResourceType)) {
        m_tagModelsForResourceType.insert(selectedResourceType, new KisTagModel(selectedResourceType));
        m_tagModelsForResourceType[selectedResourceType]->sort(KisAllTagsModel::Name);
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
        proxyModel->sort(KisAbstractResourceModel::Name);
        m_resourceProxyModelsForResourceType.insert(selectedResourceType, proxyModel);
    }
    m_resourceProxyModelsForResourceType[selectedResourceType]->setStorageFilter(true, getCurrentStorageId());
    m_resourceProxyModelsForResourceType[selectedResourceType]->setTagFilter(getCurrentTag());

    m_ui->resourceItemView->setModel(m_resourceProxyModelsForResourceType[selectedResourceType]);

    if (selectedResourceType == ResourceType::Gradients) {
        m_ui->resourceItemView->setFixedToolTipThumbnailSize(QSize(256, 64));
        m_ui->resourceItemView->setToolTipShouldRenderCheckers(true);
    }
    else if (selectedResourceType == ResourceType::PaintOpPresets) {
        m_ui->resourceItemView->setFixedToolTipThumbnailSize(QSize(128, 128));
    } else if (selectedResourceType == ResourceType::Patterns || selectedResourceType == ResourceType::Palettes) {
        m_ui->resourceItemView->setFixedToolTipThumbnailSize(QSize(256, 256));
        m_ui->resourceItemView->setToolTipShouldRenderCheckers(false);
    }

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
    Q_UNUSED(index);
    QModelIndexList list = m_ui->resourceItemView->selectionModel()->selection().indexes();
    KisTagFilterResourceProxyModel* model = m_resourceProxyModelsForResourceType[getCurrentResourceType()];
    if (list.size() == 1) {
        QModelIndex idx = list[0];
        m_ui->lblFilename->setText(model->data(idx, Qt::UserRole + KisAllResourcesModel::Filename).toString());
        m_ui->lneName->setText(model->data(idx, Qt::UserRole + KisAllResourcesModel::Name).toString());
        m_ui->lblLocation->setText(model->data(idx, Qt::UserRole + KisAllResourcesModel::Location).toString());
        m_ui->lblId->setText(model->data(idx, Qt::UserRole + KisAllResourcesModel::Id).toString());

        QSize thumbSize = m_ui->lblThumbnail->size();

        QImage thumbLabel = m_thumbnailPainter.getReadyThumbnail(idx, thumbSize*devicePixelRatioF(), palette());
        thumbLabel.setDevicePixelRatio(devicePixelRatioF());

        QPixmap pix = QPixmap::fromImage(thumbLabel);
        m_ui->lblThumbnail->setScaledContents(true);
        m_ui->lblThumbnail->setPixmap(pix);

        QMap<QString, QVariant> metadata = model->data(idx, Qt::UserRole + KisAllResourcesModel::MetaData).toMap();

        m_ui->lblFilename->setDisabled(false);
        m_ui->lblLocation->setDisabled(false);
        m_ui->lblThumbnail->setDisabled(false);
        m_ui->lneName->setDisabled(false);
        m_ui->lblId->setDisabled(false);
        m_ui->lblMetadata->setText(constructMetadata(metadata, getCurrentResourceType()));


    } else if (list.size() > 1) {

        QString commonLocation = model->data(list.first(), Qt::UserRole + KisAllResourcesModel::Location).toString();
        bool commonLocationFound = true;
        Q_FOREACH(QModelIndex idx, list) {
            QString location = model->data(idx, Qt::UserRole + KisAllResourcesModel::Location).toString();
            if (location != commonLocation) {
                commonLocationFound = false;
            }
        }

        QString multipleSelectedText = i18nc("In Resource manager, this is text shown instead of filename, name or location, "
                                             "when multiple resources are shown so there is no one specific filename", "(Multiple selected)");

        m_ui->lblFilename->setText(multipleSelectedText);
        m_ui->lblLocation->setText(commonLocationFound ? commonLocation : multipleSelectedText);
        m_ui->lneName->setText(multipleSelectedText);
        m_ui->lblThumbnail->setText(multipleSelectedText);
        QPixmap pix;
        m_ui->lblThumbnail->setPixmap(pix);

        m_ui->lblFilename->setDisabled(true);
        m_ui->lblLocation->setDisabled(!commonLocationFound);
        m_ui->lblThumbnail->setDisabled(true);
        m_ui->lneName->setDisabled(true);
        m_ui->lblId->setDisabled(false);
    } else {
        QString noneSelectedText = i18nc("In Resource manager, this is text shown instead of filename, name or location, "
                                             "when no resource is shown so there is no specific filename", "(None selected)");

        m_ui->lblFilename->setText(noneSelectedText);
        m_ui->lblLocation->setText(noneSelectedText);
        m_ui->lneName->setText(noneSelectedText);
        m_ui->lblThumbnail->setText(noneSelectedText);
        QPixmap pix;
        m_ui->lblThumbnail->setPixmap(pix);

        m_ui->lblFilename->setDisabled(true);
        m_ui->lblLocation->setDisabled(true);
        m_ui->lblThumbnail->setDisabled(true);
        m_ui->lneName->setDisabled(true);
        m_ui->lblId->setDisabled(false);
    }

    QList<int> resourceIds;
    Q_FOREACH(QModelIndex idx, list) {
        int resourceId = model->data(idx, Qt::UserRole + KisAllResourcesModel::Id).toInt();
        resourceIds << resourceId;
    }
    updateDeleteButtonState(list);
    m_tagsController->setResourceIds(getCurrentResourceType(), resourceIds);
}

void DlgResourceManager::slotFilterTextChanged(const QString &filterText)
{
    if (m_resourceProxyModelsForResourceType.contains(getCurrentResourceType())) {
        m_resourceProxyModelsForResourceType[getCurrentResourceType()]->setSearchText(filterText);
    }
}

void DlgResourceManager::slotShowDeletedChanged(int newState)
{
    Q_UNUSED(newState);

    if (m_resourceProxyModelsForResourceType.contains(getCurrentResourceType())) {
        m_resourceProxyModelsForResourceType[getCurrentResourceType()]->setResourceFilter(
                    m_ui->chkShowDeleted->isChecked() ? KisTagFilterResourceProxyModel::ShowAllResources : KisTagFilterResourceProxyModel::ShowActiveResources);
    }
}

void DlgResourceManager::slotDeleteResources()
{
    QModelIndexList list = m_ui->resourceItemView->selectionModel()->selection().indexes();
    if (!m_resourceProxyModelsForResourceType.contains(getCurrentResourceType()) || list.empty()) {
        return;
    }
    KisTagFilterResourceProxyModel *model = m_resourceProxyModelsForResourceType[getCurrentResourceType()];
    KisAllResourcesModel *allModel = KisResourceModelProvider::resourceModel(getCurrentResourceType());

    if (static_cast<QAbstractItemModel*>(model) != m_ui->resourceItemView->model()) {
        qCritical() << "wrong item model!";
        return;
    }

    // deleting a resource with "Show deleted resources" disabled will update the proxy model
    // and next index in selection now points at wrong item.
    QList<int> resourceIds;
    Q_FOREACH (QModelIndex index, list) {
        int resourceId = model->data(index, Qt::UserRole + KisResourceModel::Id).toInt();
        resourceIds.append(resourceId);
    }

    Q_FOREACH (int resourceId, resourceIds) {
        if (m_undeleteMode) {
            // FIXME: There is currently no nicer way to set an inactive resource active again...
            QModelIndex index = allModel->indexForResourceId(resourceId);
            allModel->setData(index, true, Qt::CheckStateRole);
        } else {
            QModelIndex index = allModel->indexForResourceId(resourceId);
            allModel->setResourceInactive(index);
        }
    }

    updateDeleteButtonState(list);
}

void DlgResourceManager::slotImportResources()
{
    ResourceImporter importer(this);
    importer.importResources();



}

void DlgResourceManager::slotOpenResourceFolder()
{
    if (m_actionManager) {
        KisAction *action = m_actionManager->actionByName("open_resources_directory");
        action->trigger();
    }
}

void DlgResourceManager::slotCreateBundle()
{
    DlgCreateBundle* dlg = new DlgCreateBundle(0, this);
    dlg->exec();
}


void DlgResourceManager::slotSaveTags()
{
    KisResourceLocator::instance()->saveTags();
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

void DlgResourceManager::updateDeleteButtonState(const QModelIndexList &list)
{
    bool allActive = true;
    bool allInactive = true;

    for(QModelIndex index: list) {
        bool active = index.data(Qt::UserRole + KisAllResourcesModel::ResourceActive).toBool();
        allActive = allActive && active;
        allInactive = allInactive && !active;
    }

    // if nothing selected or selected are mixed active/inactive state
    if (allActive == allInactive) {
        m_ui->btnDeleteResource->setEnabled(false);
    }
    // either all are active or all are inactive
    else {
        m_undeleteMode = allInactive;
        m_ui->btnDeleteResource->setEnabled(true);
        if (m_undeleteMode) {
            m_ui->btnDeleteResource->setText(i18n("Undelete Resources"));
        } else {
            m_ui->btnDeleteResource->setText(i18n("Delete Resources"));
        }
    }
}

QString DlgResourceManager::constructMetadata(QMap<QString, QVariant> metadata, QString resourceType)
{
    QString response;
    if (resourceType == ResourceType::PaintOpPresets) {
        QString paintopKey = "paintopid";
        QString paintopId = metadata.contains(paintopKey) ? metadata[paintopKey].toString() : "";
        if (!paintopId.isEmpty()) {

            KisPaintOpFactory* factory = KisPaintOpRegistry::instance()->get(paintopId);
            if (factory) {
                QString name = factory->name();
                response.append(name);
            } else {
                response.append(i18nc("Brush engine type, in resource manager", "Engine: "));
                response.append(paintopId);
            }
        }


    } else if (resourceType == ResourceType::GamutMasks) {
        QString descriptonKey = "description";
        QString description = metadata.contains(descriptonKey) ? metadata[descriptonKey].toString() : "";
        response.append(description);
    } else {
        Q_FOREACH(QString key, metadata.keys()) {
            response.append(key).append(": ").append(metadata[key].toString()).append("\n");
        }
    }
    return response;

}
