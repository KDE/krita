/*
 *  SPDX-FileCopyrightText: 2023 Srirupa Datta <srirupa.sps@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "ui_wdgresourcepreview.h"
#include "wdg_resource_preview.h"

#include <QAction>
#include <QDebug>
#include <QItemSelection>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QLabel>
#include <QCompleter>
#include <QLineEdit>

#include <kis_action.h>
#include <kis_action_manager.h>
#include <kis_icon.h>
#include <KisResourceTypeModel.h>
#include <KisStorageModel.h>
#include <KisTagModel.h>
#include <KisResourceItemListView.h>
#include <KisResourceLocator.h>
#include <KisResourceModel.h>
#include <KisTagFilterResourceProxyModel.h>
#include <kis_assert.h>
#include <KisResourceItemDelegate.h>
#include <wdgtagselection.h>
#include <kis_paintop_factory.h>
#include <kis_paintop_registry.h>
#include <dlg_create_bundle.h>
#include <ResourceImporter.h>
#include <KoIcon.h>
#include "ResourceListViewModes.h"
#include <kis_config.h>
#include "KisResourceItemViewer.h"

#include <config-seexpr.h>


WdgResourcePreview::WdgResourcePreview(WidgetType type, QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::WdgResourcePreview),
    m_type(type)

{
    m_ui->setupUi(this);

    m_ui->resourceItemView->setFixedToolTipThumbnailSize(QSize(128, 128));

    if (m_type == WidgetType::ResourceManager) {
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
    } else {
        QStringList resourceTypes = QStringList() << ResourceType::Brushes << ResourceType::PaintOpPresets << ResourceType::Gradients << ResourceType::GamutMasks;
        #if defined HAVE_SEEXPR
        resourceTypes << ResourceType::SeExprScripts;
        #endif
        resourceTypes << ResourceType::Patterns << ResourceType::Palettes << ResourceType::Workspaces << ResourceType::CssStyles;

        for (int i = 0; i < resourceTypes.size(); i++) {
            m_ui->cmbResourceType->addItem(ResourceName::resourceTypeToName(resourceTypes[i]));
            m_ui->cmbResourceType->setItemData(m_ui->cmbResourceType->count()-1, resourceTypes[i], Qt::UserRole + KisResourceTypeModel::ResourceType);
         }
    }

    connect(m_ui->cmbResourceType, SIGNAL(activated(int)), SLOT(slotResourceTypeSelected(int)));
    connect(m_ui->cmbResourceType, SIGNAL(activated(int)), SIGNAL(resourceTypeSelected(int)));

    QString selectedResourceType = getCurrentResourceType();


    KisTagModel* tagModel = new KisTagModel(selectedResourceType);
    tagModel->sort(KisAllTagsModel::Name);
    m_tagModelsForResourceType.insert(selectedResourceType, tagModel);

    m_ui->cmbTag->setModel(tagModel);
    m_ui->cmbTag->setModelColumn(KisAllTagsModel::Name);

    connect(m_ui->cmbTag, SIGNAL(activated(int)), SLOT(slotTagSelected(int)));

    if (m_type == WidgetType::BundleCreator) {
        m_ui->cmbStorage->setVisible(false);
    } else {
        m_storageModel = new KisStorageModel(this);

        m_ui->cmbStorage->setModel(m_storageModel);
        m_ui->cmbStorage->setModelColumn(KisStorageModel::DisplayName);
        connect(m_ui->cmbStorage, SIGNAL(activated(int)), SLOT(slotStorageSelected(int)));
    }

    // the model will be owned by `proxyModel`
    KisResourceModel* resourceModel;

    if (m_type == WidgetType::ResourceManager) {
        resourceModel = new KisResourceModel(selectedResourceType);
        resourceModel->setStorageFilter(KisResourceModel::ShowAllStorages);
        resourceModel->setResourceFilter(m_ui->chkShowDeleted->isChecked() ? KisResourceModel::ShowAllResources : KisResourceModel::ShowActiveResources);

    } else {

        QString standarizedResourceType = (selectedResourceType == "presets" ? ResourceType::PaintOpPresets : selectedResourceType);
        resourceModel = new KisResourceModel(standarizedResourceType);
    }

    KisTagFilterResourceProxyModel* proxyModel = new KisTagFilterResourceProxyModel(selectedResourceType);
    proxyModel->setResourceModel(resourceModel);
    proxyModel->setTagFilter(0);
    if (m_type == WidgetType::ResourceManager) {
        proxyModel->setStorageFilter(true, getCurrentStorageId());
    }
    proxyModel->sort(KisAbstractResourceModel::Name);
    m_resourceProxyModelsForResourceType.insert(selectedResourceType, proxyModel);

    m_ui->resourceItemView->setModel(proxyModel);
    m_kisResourceItemDelegate = new KisResourceItemDelegate(this);
    m_ui->resourceItemView->setItemDelegate(m_kisResourceItemDelegate);
    m_ui->resourceItemView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    if (m_type == WidgetType::BundleCreator) {
        m_ui->chkShowDeleted->setVisible(false);
    } else {
        connect(m_ui->chkShowDeleted, SIGNAL(stateChanged(int)), SLOT(slotShowDeletedChanged(int)));
    }

    connect(m_ui->lneFilterText, SIGNAL(textChanged(const QString&)), SLOT(slotFilterTextChanged(const QString&)));

    connect(m_ui->resourceItemView, SIGNAL(currentResourceChanged(QModelIndex)), SIGNAL(signalResourcesSelectionChanged(QModelIndex)));

    KisResourceItemViewer *viewModeButton;

    if (m_type == WidgetType::BundleCreator) {
        viewModeButton = new KisResourceItemViewer(Viewer::TableAvailable, this);
    } else {
        viewModeButton = new KisResourceItemViewer(Viewer::ResourceManager, this);
    }

    KisConfig cfg(true);

    if (m_type == WidgetType::BundleCreator) {
        m_mode = (cfg.readEntry<quint32>("ResourceItemsBCSearch.viewMode", 1) == 1)? ListViewMode::IconGrid : ListViewMode::Detail;
    } else {
        m_mode = (cfg.readEntry<quint32>("ResourceItemsRM.viewMode", 1) == 1)? ListViewMode::IconGrid : ListViewMode::Detail;
    }

    connect(viewModeButton, SIGNAL(onViewThumbnail()), this, SLOT(slotViewThumbnail()));
    connect(viewModeButton, SIGNAL(onViewDetails()), this, SLOT(slotViewDetails()));

    if (m_type == WidgetType::BundleCreator) {
        QLabel *label = new QLabel("Search");
        m_ui->horizontalLayout_2->addWidget(label);
    } else {
        m_ui->horizontalLayout_2->setAlignment(Qt::AlignRight);
    }
    m_ui->horizontalLayout_2->addWidget(viewModeButton);

    if (m_mode == ListViewMode::IconGrid) {
        slotViewThumbnail();
    } else {
        slotViewDetails();
    }

}

WdgResourcePreview::~WdgResourcePreview()
{
    qDeleteAll(m_tagModelsForResourceType);
    qDeleteAll(m_resourceProxyModelsForResourceType);
    delete m_storageModel;
    delete m_resourceTypeModel;
}

void WdgResourcePreview::slotViewThumbnail()
{
    m_kisResourceItemDelegate->setShowText(false);
    m_ui->resourceItemView->setItemDelegate(m_kisResourceItemDelegate);
    m_ui->resourceItemView->setListViewMode(ListViewMode::IconGrid);
}

void WdgResourcePreview::slotViewDetails()
{
    m_kisResourceItemDelegate->setShowText(true);
    m_ui->resourceItemView->setItemDelegate(m_kisResourceItemDelegate);
    m_ui->resourceItemView->setListViewMode(ListViewMode::Detail);
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
        KisResourceModel* resourceModel;
        if (m_type == WidgetType::ResourceManager) {
            resourceModel = new KisResourceModel(selectedResourceType);
            KIS_SAFE_ASSERT_RECOVER_RETURN(resourceModel);
            resourceModel->setStorageFilter(KisResourceModel::ShowAllStorages);
            resourceModel->setResourceFilter(m_ui->chkShowDeleted->isChecked() ? KisResourceModel::ShowAllResources : KisResourceModel::ShowActiveResources);
        } else {
            QString standarizedResourceType = (selectedResourceType == "presets" ? ResourceType::PaintOpPresets : selectedResourceType);
            resourceModel = new KisResourceModel(standarizedResourceType);
            KIS_SAFE_ASSERT_RECOVER_RETURN(resourceModel);
        }

        KisTagFilterResourceProxyModel* proxyModel = new KisTagFilterResourceProxyModel(selectedResourceType);
        KIS_SAFE_ASSERT_RECOVER_RETURN(proxyModel);
        proxyModel->setResourceModel(resourceModel);
        proxyModel->sort(KisAbstractResourceModel::Name);
        m_resourceProxyModelsForResourceType.insert(selectedResourceType, proxyModel);
    }

    if (m_type == WidgetType::ResourceManager) {
        m_resourceProxyModelsForResourceType[selectedResourceType]->setStorageFilter(true, getCurrentStorageId());
    }
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

QModelIndexList WdgResourcePreview::getResourceItemsSelected()
{
    QModelIndexList list = m_ui->resourceItemView->selectionModel()->selection().indexes();
    return list;
}

QMap<QString, KisTagFilterResourceProxyModel*> WdgResourcePreview::getResourceProxyModelsForResourceType()
{
    return m_resourceProxyModelsForResourceType;
}

int WdgResourcePreview::getCurrentStorageId()
{
    return m_ui->cmbStorage->currentData(Qt::UserRole + KisStorageModel::Id).toInt();
}

QAbstractItemModel* WdgResourcePreview::getModel()
{
    return m_ui->resourceItemView->model();
}
