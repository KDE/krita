/*
 *  SPDX-FileCopyrightText: 2023 Srirupa Datta <srirupa.sps@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "page_resource_chooser.h"
#include "ui_pageresourcechooser.h"
#include "wdg_resource_preview.h"
#include "KisResourceItemViewer.h"
#include <kis_config.h>
#include "dlg_create_bundle.h"

#include <QPainter>
#include <QDebug>
#include <QLabel>
#include <QScrollBar>
#include <QScrollArea>
#include <QListWidgetItem>

#include <KisResourceModel.h>
#include <KisResourceTypeModel.h>
#include <KisTagFilterResourceProxyModel.h>
#include "KisResourceItemListWidget.h"
#include "ResourceListViewModes.h"
#include "KisGlobalResourcesInterface.h"

#include <config-seexpr.h>

#define ICON_SIZE 56

PageResourceChooser::PageResourceChooser(KoResourceBundleSP bundle, QWidget *parent) :
    QWizardPage(parent),
    m_ui(new Ui::PageResourceChooser)
    , m_bundle(bundle)
{
    m_ui->setupUi(this);

    m_wdgResourcePreview = new WdgResourcePreview(WidgetType::BundleCreator, this);
    m_ui->formLayout->addWidget(m_wdgResourcePreview);

    connect(m_wdgResourcePreview, SIGNAL(signalResourcesSelectionChanged(QModelIndex)), this, SLOT(slotResourcesSelectionChanged(QModelIndex)));
    connect(m_wdgResourcePreview, SIGNAL(resourceTypeSelected(int)), this, SLOT(slotResourceTypeSelected(int)));

    m_resourceItemWidget = new KisResourceItemListWidget(this);
    m_ui->verticalLayout_2->insertWidget(1, m_resourceItemWidget);

    m_kisResourceItemDelegate = new KisResourceItemDelegate(this);
    m_kisResourceItemDelegate->setIsWidget(true);
    m_resourceItemWidget->setItemDelegate(m_kisResourceItemDelegate);

    connect(m_ui->btnRemoveSelected, SIGNAL(clicked(bool)), this, SLOT(slotRemoveSelected(bool)));

    KisResourceItemViewer *viewModeButton;
    viewModeButton = new KisResourceItemViewer(Viewer::TableSelected, this);

    KisConfig cfg(true);
    m_mode = (cfg.readEntry<quint32>("ResourceItemsBCSelected.viewMode", 1) == 1)? ListViewMode::IconGrid : ListViewMode::Detail;


    connect(viewModeButton, SIGNAL(onViewThumbnail()), this, SLOT(slotViewThumbnail()));
    connect(viewModeButton, SIGNAL(onViewDetails()), this, SLOT(slotViewDetails()));

    QLabel *label = new QLabel("Selected");
    m_ui->horizontalLayout_2->addWidget(label);
    m_ui->horizontalLayout_2->addWidget(viewModeButton);

    if (m_mode == ListViewMode::IconGrid) {
        slotViewThumbnail();
    } else {
        slotViewDetails();
    }

    m_resourceItemWidget->clear();

    if (m_bundle) {

        m_bundleStorage = new KisBundleStorage(m_bundle->filename());

        QStringList resourceTypes = QStringList() << ResourceType::Brushes << ResourceType::PaintOpPresets << ResourceType::Gradients << ResourceType::GamutMasks;
        #if defined HAVE_SEEXPR
        resourceTypes << ResourceType::SeExprScripts;
        #endif
        resourceTypes << ResourceType::Patterns << ResourceType::Palettes << ResourceType::Workspaces << ResourceType::CssStyles;


        for (int i = 0; i < resourceTypes.size(); i++) {
            QSharedPointer<KisResourceStorage::ResourceIterator> iter = m_bundleStorage->resources(resourceTypes[i]);
            KisResourcesInterfaceSP resourcesInterface = KisGlobalResourcesInterface::instance();
            auto resourceSourceAdapter = resourcesInterface->source<KoResource>(resourceTypes[i]);
            while (iter->hasNext()) {
                iter->next();
                KoResourceSP res = resourceSourceAdapter.bestMatch(iter->resource()->md5Sum(false), iter->resource()->filename(), iter->resource()->name());
                if (!res.isNull()) {
                    m_selectedResourcesIds.append(res->resourceId());
                }
            }
        }


        QString standarizedResourceType = ResourceType::Brushes;

        KisResourceModel model(standarizedResourceType);
        for (int i = 0; i < model.rowCount(); i++) {
            QModelIndex idx = model.index(i, 0);
            int id = model.data(idx, Qt::UserRole + KisAbstractResourceModel::Id).toInt();

            if (m_selectedResourcesIds.contains(id)) {
                selectResource(&model, idx);
            }
        }

        m_resourceItemWidget->sortItems();
    }

}

void PageResourceChooser::updateResources(QString resourceType, int count)
{
    DlgCreateBundle *wizard = qobject_cast<DlgCreateBundle*>(this->wizard());
    if (wizard) {
        wizard->m_count[resourceType] = count;
        qDebug() << resourceType << ": " << count;
    }
    Q_EMIT countUpdated();

}

void PageResourceChooser::slotViewThumbnail()
{
    m_kisResourceItemDelegate->setShowText(false);
    m_resourceItemWidget->setItemDelegate(m_kisResourceItemDelegate);
    m_resourceItemWidget->setListViewMode(ListViewMode::IconGrid);
}

void PageResourceChooser::slotViewDetails()
{
    m_kisResourceItemDelegate->setShowText(true);
    m_resourceItemWidget->setItemDelegate(m_kisResourceItemDelegate);
    m_resourceItemWidget->setListViewMode(ListViewMode::Detail);
}

void PageResourceChooser::slotResourcesSelectionChanged(QModelIndex selected)
{
    Q_UNUSED(selected);

    QModelIndexList list = m_wdgResourcePreview->getResourceItemsSelected();
    KisTagFilterResourceProxyModel* proxyModel = m_wdgResourcePreview->getResourceProxyModelsForResourceType()[m_wdgResourcePreview->getCurrentResourceType()];


    Q_FOREACH (QModelIndex idx, list) {
        int id = proxyModel->data(idx, Qt::UserRole + KisAllResourcesModel::Id).toInt();
        if (!m_selectedResourcesIds.contains(id)) {
            selectResource(proxyModel, idx);
            m_selectedResourcesIds.append(id);
            updateCount(true);
        }
    }

    m_resourceItemWidget->sortItems();
}

void PageResourceChooser::slotResourceTypeSelected(int idx)
{
    Q_UNUSED(idx);

    QString resourceType = m_wdgResourcePreview->getCurrentResourceType();
    m_resourceItemWidget->clear();
    QString standarizedResourceType = (resourceType == "presets" ? ResourceType::PaintOpPresets : resourceType);

    KisResourceModel model(standarizedResourceType);
    for (int i = 0; i < model.rowCount(); i++) {
        QModelIndex idx = model.index(i, 0);
        int id = model.data(idx, Qt::UserRole + KisAbstractResourceModel::Id).toInt();

        if (m_selectedResourcesIds.contains(id)) {
            selectResource(&model, idx);
        }
    }

    m_resourceItemWidget->sortItems();
}

void PageResourceChooser::selectResource(QSortFilterProxyModel *model, const QModelIndex idx)
{
    QString name = model->data(idx, Qt::UserRole + KisAllResourcesModel::Name).toString();

    // Don't try to bundle special resources
    QString resourceType = m_wdgResourcePreview->getCurrentResourceType();
    if (resourceType == ResourceType::Gradients) {
        if (name == "Foreground to Transparent" || name == "Foreground to Background") {
            return;
         }
     }

    int id = model->data(idx, Qt::UserRole + KisAllResourcesModel::Id).toInt();
    QRect paintRect = QRect(0, 0, ICON_SIZE, ICON_SIZE);
    QPixmap pixmap(paintRect.size() * devicePixelRatioF());
    pixmap.setDevicePixelRatio(devicePixelRatioF());
    QPainter painter(&pixmap);
    KisResourceThumbnailPainter().paint(&painter, idx, paintRect, this->palette(), false, true);

    QListWidgetItem *item = new QListWidgetItem(pixmap, name);
    item->setData(Qt::UserRole, id);
    item->setData(Qt::UserRole + KisAbstractResourceModel::Name, name);

    m_resourceItemWidget->addItem(item);
}

void PageResourceChooser::slotRemoveSelected(bool)
{
    int row = m_resourceItemWidget->currentRow();

    Q_FOREACH (QListWidgetItem *item, m_resourceItemWidget->selectedItems()) {
        m_resourceItemWidget->takeItem(m_resourceItemWidget->row(item));
        m_selectedResourcesIds.removeAll(item->data(Qt::UserRole).toInt());
        updateCount(false);
    }

    m_resourceItemWidget->setCurrentRow(row);
}

QList<int> PageResourceChooser::getSelectedResourcesIds()
{
    return m_selectedResourcesIds;
}

void PageResourceChooser::updateCount(bool flag)
{
    DlgCreateBundle *wizard = qobject_cast<DlgCreateBundle*>(this->wizard());
    if (wizard) {
        flag == true? wizard->m_count[m_wdgResourcePreview->getCurrentResourceType()] += 1 : wizard->m_count[m_wdgResourcePreview->getCurrentResourceType()] -= 1;
    }

    Q_EMIT countUpdated();
}

PageResourceChooser::~PageResourceChooser()
{
    delete m_ui;
}
