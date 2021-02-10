/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
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

#include <TableModel.h>
#include <KisResourceModel.h>
#include <KisTagFilterResourceProxyModel.h>
#include <KisResourceTypeModel.h>
#include <KisTagModel.h>
#include <KisStorageModel.h>
#include <KisResourceItemDelegate.h>
#include <KisResourceItemListView.h>


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
        m_page->tableStorages->setModel(new KisStorageModel(this));
        m_page->tableStorages->hideColumn(0);
        m_page->tableStorages->setSelectionMode(QAbstractItemView::SingleSelection);
        m_page->tableStorages->resizeColumnsToContents();
    }

    {
        KisResourceModel *resourcesModel = new KisResourceModel(ResourceType::Brushes, this);
        m_page->tableResources->setModel(resourcesModel);
        m_page->tableResources->hideColumn(0);
        m_page->tableResources->setSelectionMode(QAbstractItemView::SingleSelection);

        m_page->cmbResourceTypes->setModel(m_resourceTypeModel);
        m_page->cmbResourceTypes->setModelColumn(KisResourceTypeModel::Name);

        connect(m_page->cmbResourceTypes, SIGNAL(activated(int)), SLOT(slotTbResourceTypeSelected(int)));
        connect(m_page->tableResources, SIGNAL(clicked(QModelIndex)), SLOT(slotTbResourceItemSelected()));
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
        tagsModel->setHeaderData(5, Qt::Horizontal, i18n("Active"));
        tagsModel->setHeaderData(6, Qt::Horizontal, i18n("Thumbnail"));
        tagsModel->setHeaderData(7, Qt::Horizontal, i18n("Display Name"));
        tagsModel->addBooleanColumn(5);
        tagsDelegate->addBooleanColumn(5);
        tagsModel->select();

        m_page->tableTags->setModel(tagsModel);
        m_page->tableTags->hideColumn(0);
        m_page->tableTags->setItemDelegate(tagsDelegate);
        m_page->tableTags->setSelectionMode(QAbstractItemView::SingleSelection);
        m_page->tableTags->resizeColumnsToContents();
    }

    {
        QSqlTableModel *versionModel = new QSqlTableModel(this, QSqlDatabase::database());
        versionModel->setTable("version_information");
        versionModel->setHeaderData(0, Qt::Horizontal, "id");
        versionModel->setHeaderData(1, Qt::Horizontal, "database_version");
        versionModel->setHeaderData(2, Qt::Horizontal, "krita_version");
        versionModel->setHeaderData(3, Qt::Horizontal, "creation_date");
        versionModel->select();
        QSqlRecord r = versionModel->record(0);

        m_page->lblDatabaseVersion->setText(r.value("database_version").toString());
        m_page->lblKritaVersion->setText(r.value("krita_version").toString());
        m_page->lblCreationDate->setText(r.value("creation_date").toString());
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
    qDebug() << resourceType;

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

    m_page->lblThumbnail->setPixmap(QPixmap::fromImage(thumb.scaled(100, 100, Qt::KeepAspectRatio, mode)));
    //If we could get a list of versions for a given resource, this would be the moment to add them...
}

void DlgDbExplorer::slotRvTagSelected(int index)
{
    qDebug() << "selected tag" << index;
    QModelIndex idx = m_tagModel->index(index, 0);
    KisTagSP tag = m_tagModel->tagForIndex(idx);

    if (m_filterProxyModel && !tag.isNull() && tag->valid()) {
        m_filterProxyModel->setTagFilter(tag);
    }
}
