/*
 * SPDX-FileCopyrightText: 2021 Mathias Wein <lynx.mw+kde@gmail.com>
 *  SPDX-FileCopyrightText: 2023 Srirupa Datta <srirupa.sps@gmail.com>
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

    m_wdgResourcePreview = new WdgResourcePreview(WidgetType::ResourceManager, this);
    m_ui->formLayout->addWidget(m_wdgResourcePreview);

    connect(m_wdgResourcePreview, SIGNAL(signalResourcesSelectionChanged(QModelIndex)), this, SLOT(slotResourcesSelectionChanged(QModelIndex)));

    connect(m_ui->btnDeleteResource, SIGNAL(clicked(bool)), SLOT(slotDeleteResources()));
    connect(m_ui->btnCreateBundle, SIGNAL(clicked(bool)), SLOT(slotCreateBundle()));
    connect(m_ui->btnOpenResourceFolder, SIGNAL(clicked(bool)), SLOT(slotOpenResourceFolder()));
    connect(m_ui->btnImportResources, SIGNAL(clicked(bool)), SLOT(slotImportResources()));
    connect(m_ui->btnExtractTagsToResourceFolder, SIGNAL(clicked(bool)), SLOT(slotSaveTags()));


    m_tagsController.reset(new KisWdgTagSelectionControllerOneResource(m_ui->wdgResourcesTags, true));

#ifdef Q_OS_ANDROID
    // TODO(sh_zam): Opening a directory can cause a crash. A ContentProvider is needed for this.
    m_ui->btnOpenResourceFolder->setEnabled(false);
    m_ui->btnOpenResourceFolder->setVisible(false);
#endif

    // make sure the panel is properly cleared. -Amy
    slotResourcesSelectionChanged({});
}

DlgResourceManager::~DlgResourceManager()
{
}

void DlgResourceManager::slotResourcesSelectionChanged(QModelIndex index)
{
    Q_UNUSED(index);
    QModelIndexList list = m_wdgResourcePreview->geResourceItemsSelected();
    KisTagFilterResourceProxyModel* model = m_wdgResourcePreview->getResourceProxyModelsForResourceType()[m_wdgResourcePreview->getCurrentResourceType()];
    if (list.size() == 1) {
        const QModelIndex idx = list[0];
        m_ui->lblFilename->setText(model->data(idx, Qt::UserRole + KisAllResourcesModel::Filename).toString());
        m_ui->lneName->setText(model->data(idx, Qt::UserRole + KisAllResourcesModel::Name).toString());
        m_ui->lblLocation->setText(model->data(idx, Qt::UserRole + KisAllResourcesModel::Location).toString());
        m_ui->lblId->setText(model->data(idx, Qt::UserRole + KisAllResourcesModel::Id).toString());

        const QSize thumbSize = m_ui->lblThumbnail->size();

        QImage thumbLabel = m_thumbnailPainter.getReadyThumbnail(idx, thumbSize*devicePixelRatioF(), palette());
        thumbLabel.setDevicePixelRatio(devicePixelRatioF());

        const QPixmap pix = QPixmap::fromImage(thumbLabel);
        m_ui->lblThumbnail->setScaledContents(true);
        m_ui->lblThumbnail->setPixmap(pix);

        const QMap<QString, QVariant> metadata =
            model->data(idx, Qt::UserRole + KisAllResourcesModel::MetaData).toMap();

        m_ui->lblMetadata->setDisabled(false);
        m_ui->lblFilename->setDisabled(false);
        m_ui->lblLocation->setDisabled(false);
        m_ui->lblThumbnail->setDisabled(false);
        m_ui->lneName->setDisabled(false);
        m_ui->lblId->setDisabled(false);
        m_ui->lblMetadata->setText(constructMetadata(metadata, m_wdgResourcePreview->getCurrentResourceType()));
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

        m_ui->lblId->setText(multipleSelectedText);
        m_ui->lblMetadata->setText("");
        m_ui->lblFilename->setText(multipleSelectedText);
        m_ui->lblLocation->setText(commonLocationFound ? commonLocation : multipleSelectedText);
        m_ui->lneName->setText(multipleSelectedText);
        m_ui->lblThumbnail->setText(multipleSelectedText);
        QPixmap pix;
        m_ui->lblThumbnail->setPixmap(pix);

        m_ui->lblMetadata->setDisabled(true);
        m_ui->lblFilename->setDisabled(true);
        m_ui->lblLocation->setDisabled(!commonLocationFound);
        m_ui->lblThumbnail->setDisabled(true);
        m_ui->lneName->setDisabled(true);
        m_ui->lblId->setDisabled(true);
    } else {
        QString noneSelectedText = i18nc("In Resource manager, this is text shown instead of filename, name or location, "
                                             "when no resource is shown so there is no specific filename", "(None selected)");

        m_ui->lblId->setText(noneSelectedText);
        m_ui->lblMetadata->setText(noneSelectedText);
        m_ui->lblFilename->setText(noneSelectedText);
        m_ui->lblLocation->setText(noneSelectedText);
        m_ui->lneName->setText(noneSelectedText);
        m_ui->lblThumbnail->setText(noneSelectedText);
        m_ui->lblThumbnail->setPixmap({});

        m_ui->lblMetadata->setDisabled(true);
        m_ui->lblFilename->setDisabled(true);
        m_ui->lblLocation->setDisabled(true);
        m_ui->lblThumbnail->setDisabled(true);
        m_ui->lneName->setDisabled(true);
        m_ui->lblId->setDisabled(true);
    }

    QList<int> resourceIds;
    Q_FOREACH(QModelIndex idx, list) {
        int resourceId = model->data(idx, Qt::UserRole + KisAllResourcesModel::Id).toInt();
        resourceIds << resourceId;
    }
    updateDeleteButtonState(list);
    m_tagsController->setResourceIds(m_wdgResourcePreview->getCurrentResourceType(), resourceIds);
}

void DlgResourceManager::slotDeleteResources()
{
    QModelIndexList list = m_wdgResourcePreview->geResourceItemsSelected();
    QMap<QString, KisTagFilterResourceProxyModel*> resourceProxyModelsForResourceType = m_wdgResourcePreview->getResourceProxyModelsForResourceType();

    if (!resourceProxyModelsForResourceType.contains(m_wdgResourcePreview->getCurrentResourceType()) || list.empty()) {
        return;
    }
    KisTagFilterResourceProxyModel *model = resourceProxyModelsForResourceType[m_wdgResourcePreview->getCurrentResourceType()];
    KisAllResourcesModel *allModel = KisResourceModelProvider::resourceModel(m_wdgResourcePreview->getCurrentResourceType());

    if (static_cast<QAbstractItemModel*>(model) != m_wdgResourcePreview->getModel()) {
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

        QModelIndex index = allModel->indexForResourceId(resourceId);
        allModel->setResourceActive(index, m_undeleteMode);
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

QString DlgResourceManager::constructMetadata(const QMap<QString, QVariant> &metadata, const QString &resourceType)
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
        QString descriptionKey = "description";
        QString description = metadata.contains(descriptionKey) ? metadata[descriptionKey].toString() : "";
        response.append(description);
    } else {
        Q_FOREACH(QString key, metadata.keys()) {
            response.append(key).append(": ").append(metadata[key].toString()).append("\n");
        }
    }
    return response;

}
