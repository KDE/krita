/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2023 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "DlgDbExplorer.h"

#include <klocalizedstring.h>
#include <kis_debug.h>

#include <QDataWidgetMapper>
#include <QTableView>
#include <QtSql>
#include <QStyledItemDelegate>

#include <KisResourceItemDelegate.h>
#include <KisResourceItemListView.h>
#include <KisResourceModel.h>
#include <KisResourceTypeModel.h>
#include <KisStorageModel.h>
#include <KisTagFilterResourceProxyModel.h>
#include <KisTagModel.h>
#include <KisTagResourceModel.h>

#include "KisResourceCacheDb.h"
#include "KisResourceModelProvider.h"
#include "TableModel.h"

DlgDbExplorer::DlgDbExplorer(QWidget *parent)
    : KoDialog(parent)
{
    setCaption(i18n("Resources Cache Database Explorer"));

    setButtons(Ok);

    m_page = new WdgDbExplorer(this);
    Q_CHECK_PTR(m_page);

    setMainWidget(m_page);

    m_resourceTypeModel = new KisResourceTypeModel(this);
    m_tagModel = new KisTagModel("", this);

    {
        KisStorageModel *model = new KisStorageModel(this);
        TableDelegate *delegate = new TableDelegate(m_page->tableResources);
        m_page->tableStorages->setModel(model);
        delegate->addDateTimeColumn(3); // creation date
        delegate->addBooleanColumn(4); // preinstalled
        delegate->addBooleanColumn(5); // active
        m_page->tableStorages->setItemDelegate(delegate);
        m_page->tableStorages->hideColumn(0);
        m_page->tableStorages->setSelectionMode(QAbstractItemView::SingleSelection);
        m_page->tableStorages->resizeColumnsToContents();
    }

    {
        KisResourceModel *resourcesModel = new KisResourceModel(ResourceType::Brushes, this);
        TableDelegate *resourcesDelegate = new TableDelegate(m_page->tableResources);
        resourcesDelegate->addBooleanColumn(6); // Status
        resourcesDelegate->addBooleanColumn(12); // Dirty
        resourcesDelegate->addBooleanColumn(14); // Active
        m_page->tableResources->setModel(resourcesModel);
        m_page->tableResources->setItemDelegate(resourcesDelegate);
        m_page->tableResources->hideColumn(0);
        m_page->tableResources->setSelectionMode(QAbstractItemView::SingleSelection);

        m_page->cmbResourceTypes->setModel(m_resourceTypeModel);
        m_page->cmbResourceTypes->setModelColumn(KisResourceTypeModel::Name);

        connect(m_page->cmbResourceTypes, SIGNAL(activated(int)), SLOT(slotTbResourceTypeSelected(int)));
        connect(m_page->tableResources, SIGNAL(clicked(QModelIndex)), SLOT(slotTbResourceItemSelected()));
    }

    {
        TableModel *resourceVersionsModel = new TableModel(this, QSqlDatabase::database());
        TableDelegate *versionsDelegate = new TableDelegate(m_page->tableVersions);
        resourceVersionsModel->setTable("versioned_resources");
        resourceVersionsModel->setHeaderData(0, Qt::Horizontal, i18n("Id"));
        resourceVersionsModel->setHeaderData(1, Qt::Horizontal, i18n("Resource ID"));
        resourceVersionsModel->setHeaderData(2, Qt::Horizontal, i18n("Storage ID"));
        resourceVersionsModel->setHeaderData(3, Qt::Horizontal, i18n("Version"));
        resourceVersionsModel->setHeaderData(4, Qt::Horizontal, i18n("File name"));
        resourceVersionsModel->setHeaderData(5, Qt::Horizontal, i18n("md5sum"));
        resourceVersionsModel->setHeaderData(6, Qt::Horizontal, i18n("Creation Date"));
        resourceVersionsModel->addDateTimeColumn(6);
        versionsDelegate->addDateTimeColumn(6);
        resourceVersionsModel->select();

        m_page->tableVersions->setModel(resourceVersionsModel);
        m_page->tableVersions->hideColumn(0);
        m_page->tableVersions->setItemDelegate(versionsDelegate);
        m_page->tableVersions->setSelectionMode(QAbstractItemView::NoSelection);
        m_page->tableVersions->resizeColumnsToContents();
    }

    {
        TableModel *tagsModel = new TableModel(this, QSqlDatabase::database());
        TableDelegate *tagsDelegate = new TableDelegate(m_page->tableStorages);
        tagsModel->setTable("tags");
        tagsModel->setHeaderData(0, Qt::Horizontal, i18n("Id"));
        tagsModel->setHeaderData(1, Qt::Horizontal, i18n("Type"));
        tagsModel->setRelation(1, QSqlRelation("resource_types", "id", "name"));
        tagsModel->setHeaderData(2, Qt::Horizontal, i18n("Tag"));
        tagsModel->setHeaderData(3, Qt::Horizontal, i18n("Name"));
        tagsModel->setHeaderData(4, Qt::Horizontal, i18n("Comment"));
        tagsModel->setHeaderData(5, Qt::Horizontal, i18n("File name"));
        tagsModel->setHeaderData(6, Qt::Horizontal, i18n("Active"));
        tagsModel->setHeaderData(7, Qt::Horizontal, i18n("Thumbnail"));
        tagsModel->setHeaderData(8, Qt::Horizontal, i18n("Display Name"));
        tagsModel->addBooleanColumn(6);
        tagsDelegate->addBooleanColumn(6);
        tagsModel->select();

        m_page->tableTags->setModel(tagsModel);
        m_page->tableTags->hideColumn(0);
        m_page->tableTags->setItemDelegate(tagsDelegate);
        m_page->tableTags->setSelectionMode(QAbstractItemView::SingleSelection);
        m_page->tableTags->resizeColumnsToContents();

        connect(m_page->tableTags, &QTableView::clicked, this, &DlgDbExplorer::slotTbTagSelected);
    }

    {
        KisTagResourceModel *model = new KisTagResourceModel(ResourceType::Brushes, this);
        TableDelegate *delegate = new TableDelegate(m_page->tableTaggedResources);
        delegate->addBooleanColumn(6); // status
        delegate->addBooleanColumn(12); // dirty
        delegate->addBooleanColumn(14); // resource active
        delegate->addBooleanColumn(15); // storage active
        delegate->addBooleanColumn(20); // resource active
        delegate->addBooleanColumn(21); // tag active
        delegate->addBooleanColumn(22); // storage active

        m_page->tableTaggedResources->setModel(model);
        m_page->tableTaggedResources->setItemDelegate(delegate);
        m_page->tableTaggedResources->setSelectionMode(QAbstractItemView::NoSelection);
        m_page->tableTaggedResources->resizeColumnsToContents();
    }

    {
        TableModel *versionModel = new TableModel(this, QSqlDatabase::database());
        TableDelegate *delegate = new TableDelegate(m_page->tableSchema);
        versionModel->setTable("version_information");
        versionModel->setHeaderData(0, Qt::Horizontal, i18n("Id"));
        versionModel->setHeaderData(1, Qt::Horizontal, i18n("Database version"));
        versionModel->setHeaderData(2, Qt::Horizontal, i18n("Krita version"));
        versionModel->setHeaderData(3, Qt::Horizontal, i18n("Upgrade date"));
        versionModel->addDateTimeColumn(3);
        delegate->addDateTimeColumn(3);
        versionModel->select();

        const QSqlRecord r = versionModel->record(0);
        m_page->lblDatabaseVersion->setText(r.value(1).toString());
        m_page->lblKritaVersion->setText(r.value(2).toString());
        m_page->lblCreationDate->setText(QDateTime::fromSecsSinceEpoch(r.value(3).value<int>()).toString());

        m_page->tableSchema->setModel(versionModel);
        m_page->tableSchema->hideColumn(0);
        m_page->tableSchema->setItemDelegate(delegate);
        m_page->tableSchema->setSelectionMode(QAbstractItemView::NoSelection);
        m_page->tableSchema->resizeColumnsToContents();
    }


    {
        m_page->cmbRvResourceTypes->setModel(m_resourceTypeModel);
        m_page->cmbRvResourceTypes->setModelColumn(KisResourceTypeModel::Name);
        connect(m_page->cmbRvResourceTypes, SIGNAL(activated(int)), SLOT(slotRvResourceTypeSelected(int)));

        m_page->cmbRvTags->setModelColumn(KisAllTagsModel::Name);
        m_page->cmbRvTags->setModel(m_tagModel);
        connect(m_page->cmbRvTags, SIGNAL(activated(int)), SLOT(slotRvTagSelected(int)));

        m_page->cmbRvResourceTypes->setCurrentIndex(0);
        slotRvResourceTypeSelected(0);

        m_page->resourceItemView->setItemDelegate(new KisResourceItemDelegate(this));
        m_page->resourceItemView->setSelectionMode(QAbstractItemView::SingleSelection);
    }

}

DlgDbExplorer::~DlgDbExplorer()
{
}

void DlgDbExplorer::updateTagModel(const QString& resourceType)
{
    m_tagModel = new KisTagModel(resourceType, this);
    m_page->cmbRvTags->setModelColumn(KisAllTagsModel::Name);
    m_page->cmbRvTags->setModel(m_tagModel);
    m_page->cmbRvTags->update();
    qDebug() << "number of tags in " << resourceType << " tag model: " << m_tagModel->rowCount();
}

void DlgDbExplorer::slotRvResourceTypeSelected(int index)
{
    QModelIndex idx = m_page->cmbResourceTypes->model()->index(index, KisResourceTypeModel::ResourceType);
    QString resourceType = idx.data(Qt::DisplayRole).toString();

    updateTagModel(resourceType);

    KisTagFilterResourceProxyModel *tagFilterModel = new KisTagFilterResourceProxyModel(resourceType, this);

    m_filterProxyModel = tagFilterModel;

    m_page->resourceItemView->setModel(tagFilterModel);
}

void DlgDbExplorer::slotTbResourceTypeSelected(int index)
{
    QModelIndex idx = m_page->cmbRvResourceTypes->model()->index(index, KisResourceTypeModel::ResourceType);
    QString resourceType = idx.data(Qt::DisplayRole).toString();

    m_tagModel = new KisTagModel(resourceType, this);

    KisResourceModel *resourceModel = new KisResourceModel(resourceType, this);
    m_page->tableResources->setModel(resourceModel);
    m_page->tableResources->setCurrentIndex(m_page->tableResources->model()->index(0, 0));
    slotTbResourceItemSelected();
    m_page->tableResources->resizeColumnsToContents();
}

void DlgDbExplorer::slotTbResourceItemSelected()
{
    if (m_page->tableResources->selectionModel()->selectedIndexes().isEmpty()) return;

    QModelIndex idx = m_page->tableResources->selectionModel()->selectedIndexes().first();

    QImage thumb = idx.data(Qt::UserRole + KisAbstractResourceModel::Thumbnail).value<QImage>();
    Qt::TransformationMode mode = Qt::SmoothTransformation;
    if (thumb.size().width() < 100 && thumb.size().height() < 100) {
        mode = Qt::FastTransformation;
    }

    if (!thumb.isNull()) {
        m_page->lblThumbnail->setPixmap(QPixmap::fromImage(thumb.scaled(100, 100, Qt::KeepAspectRatio, mode)));
    } else {
        m_page->lblThumbnail->setPixmap(QPixmap());
    }

    TableModel *model = dynamic_cast<TableModel *>(m_page->tableVersions->model());

    if (model) {
        const QString md5 = idx.data(Qt::UserRole + KisAbstractResourceModel::MD5).value<QString>();
        model->setFilter(QString("md5sum='%1'").arg(md5));
        m_page->tableVersions->resizeColumnsToContents();
    }
}

void DlgDbExplorer::slotTbTagSelected(const QModelIndex &index)
{
    // 0 == tag ID
    const QModelIndex idx = m_page->tableTags->model()->index(index.row(), 0);
    // get tag's resource type
    const QModelIndex idx2 = m_page->tableTags->model()->index(index.row(), 1);

    KisTagResourceModel *newModel = new KisTagResourceModel(idx2.data().value<QString>(), this);
    newModel->setResourceFilter(KisTagResourceModel::ShowAllResources);
    newModel->setTagsFilter(QVector<int>{idx.data().value<int>()});
    QAbstractItemModel *oldModel = m_page->tableTaggedResources->model();
    m_page->tableTaggedResources->setModel(newModel);
    m_page->tableTaggedResources->resizeColumnsToContents();
    oldModel->deleteLater();
}

void DlgDbExplorer::slotRvTagSelected(int index)
{
    qDebug() << "selected tag" << index;
    QModelIndex idx = m_tagModel->index(index, 0);
    KisTagSP tag = m_tagModel->tagForIndex(idx);

    if (m_filterProxyModel && !tag.isNull() && tag->valid()) {
        m_filterProxyModel->setTagFilter(tag);
        m_page->tableTaggedResources->resizeColumnsToContents();
    }
}
