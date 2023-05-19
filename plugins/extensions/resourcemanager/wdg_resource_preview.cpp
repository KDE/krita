#include "wdg_resource_preview.h"
#include "ui_wdgresourcepreview.h"

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
#include <QDebug>

WdgResourcePreview::WdgResourcePreview(int type, QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::WdgResourcePreview),
    m_type(type)

{
    m_ui->setupUi(this);

    m_ui->resourceItemView->setFixedToolTipThumbnailSize(QSize(128, 128));

    // resource type combo box code
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

    // tag type combo box code
    QString selectedResourceType = getCurrentResourceType();

    KisTagModel* tagModel = new KisTagModel(selectedResourceType);
    tagModel->sort(KisAllTagsModel::Name);
    m_tagModelsForResourceType.insert(selectedResourceType, tagModel);

    m_ui->cmbTag->setModel(tagModel);
    m_ui->cmbTag->setModelColumn(KisAllTagsModel::Name);
    connect(m_ui->cmbTag, SIGNAL(activated(int)), SLOT(slotTagSelected(int)));

    // storage type combo box
    if (m_type == 0) {
        m_ui->cmbStorage->setVisible(false);
    } else {
        m_storageModel = new KisStorageModel(this);

        m_ui->cmbStorage->setModel(m_storageModel);
        m_ui->cmbStorage->setModelColumn(KisStorageModel::DisplayName);
        connect(m_ui->cmbStorage, SIGNAL(activated(int)), SLOT(slotStorageSelected(int)));
    }


    // resource item view code
    // the model will be owned by `proxyModel`
    KisResourceModel* resourceModel = new KisResourceModel(selectedResourceType);
    resourceModel->setStorageFilter(KisResourceModel::ShowAllStorages);
    resourceModel->setResourceFilter(m_ui->chkShowDeleted->isChecked() ? KisResourceModel::ShowAllResources : KisResourceModel::ShowActiveResources);

    KisTagFilterResourceProxyModel* proxyModel = new KisTagFilterResourceProxyModel(selectedResourceType);
    proxyModel->setResourceModel(resourceModel);
    proxyModel->setTagFilter(0);
    proxyModel->setStorageFilter(true, getCurrentStorageId());
    proxyModel->sort(KisAbstractResourceModel::Name);
    m_resourceProxyModelsForResourceType.insert(selectedResourceType, proxyModel);

    m_ui->resourceItemView->setModel(proxyModel);
    m_ui->resourceItemView->setItemDelegate(new KisResourceItemDelegate(this));
    m_ui->resourceItemView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    if (m_type == 0) {
        m_ui->chkShowDeleted->setVisible(false);
        connect(m_ui->chkShowDeleted, SIGNAL(stateChanged(int)), SLOT(slotShowDeletedChanged(int)));
    }

    connect(m_ui->lneFilterText, SIGNAL(textChanged(const QString&)), SLOT(slotFilterTextChanged(const QString&)));

    connect(m_ui->resourceItemView, SIGNAL(currentResourceChanged(QModelIndex)), SIGNAL(signalResourcesSelectionChanged(QModelIndex)));


}

WdgResourcePreview::~WdgResourcePreview()
{
    qDeleteAll(m_tagModelsForResourceType);
    qDeleteAll(m_resourceProxyModelsForResourceType);
    delete m_storageModel;
    delete m_resourceTypeModel;
}

void WdgResourcePreview::slotResourceTypeSelected(int)
{
        QString selectedResourceType = getCurrentResourceType();
    if (!m_tagModelsForResourceType.contains(selectedResourceType)) {
        m_tagModelsForResourceType.insert(selectedResourceType, new KisTagModel(selectedResourceType));
        m_tagModelsForResourceType[selectedResourceType]->sort(KisAllTagsModel::Name);
    }

    m_ui->cmbTag->setModel(m_tagModelsForResourceType[selectedResourceType]);

    if (!m_resourceProxyModelsForResourceType.contains(selectedResourceType)) {
        // the model will be owned by `proxyModel`
        KisResourceModel* resourceModel = new KisResourceModel(selectedResourceType);
        KIS_SAFE_ASSERT_RECOVER_RETURN(resourceModel);
        resourceModel->setStorageFilter(KisResourceModel::ShowAllStorages);
        resourceModel->setResourceFilter(m_ui->chkShowDeleted->isChecked() ? KisResourceModel::ShowAllResources : KisResourceModel::ShowActiveResources);

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
void WdgResourcePreview::slotTagSelected(int)
{
    if (!m_resourceProxyModelsForResourceType.contains(getCurrentResourceType())) {
        return;
    }
    m_resourceProxyModelsForResourceType[getCurrentResourceType()]->setTagFilter(getCurrentTag());
}
void WdgResourcePreview::slotStorageSelected(int)
{
    if (!m_resourceProxyModelsForResourceType.contains(getCurrentResourceType())) {
        return;
    }
    m_resourceProxyModelsForResourceType[getCurrentResourceType()]->setStorageFilter(true, getCurrentStorageId());
}

void WdgResourcePreview::slotFilterTextChanged(const QString& filterText)
{
    if (m_resourceProxyModelsForResourceType.contains(getCurrentResourceType())) {
        m_resourceProxyModelsForResourceType[getCurrentResourceType()]->setSearchText(filterText);
    }
}
void WdgResourcePreview::slotShowDeletedChanged(int newState)
{
    Q_UNUSED(newState);

    if (m_resourceProxyModelsForResourceType.contains(getCurrentResourceType())) {
        m_resourceProxyModelsForResourceType[getCurrentResourceType()]->setResourceFilter(
                    m_ui->chkShowDeleted->isChecked() ? KisTagFilterResourceProxyModel::ShowAllResources : KisTagFilterResourceProxyModel::ShowActiveResources);
    }
}

QString WdgResourcePreview::getCurrentResourceType()
{
    return m_ui->cmbResourceType->currentData(Qt::UserRole + KisResourceTypeModel::ResourceType).toString();
}

QSharedPointer<KisTag> WdgResourcePreview::getCurrentTag()
{
    return m_ui->cmbTag->currentData(Qt::UserRole + KisAllTagsModel::KisTagRole).value<KisTagSP>();
}

QModelIndexList WdgResourcePreview::geResourceItemsSelected()
{
    QModelIndexList list = m_ui->resourceItemView->selectionModel()->selection().indexes();
    return list;
}

int WdgResourcePreview::getCurrentStorageId()
{
    if (m_type == 0) {
        return 1;
    } else if (m_type == 1) {
        return m_ui->cmbStorage->currentData(Qt::UserRole + KisStorageModel::Id).toInt();
    } else {
        return 0;
    }
}
