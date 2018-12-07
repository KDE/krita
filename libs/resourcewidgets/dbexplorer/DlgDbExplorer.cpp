/*
 * Copyright (c) 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
#include <KisResourceTypeModel.h>
#include <KisTagModel.h>

DlgDbExplorer::DlgDbExplorer(QWidget *parent)
    : KoDialog(parent)
{
    setCaption(i18n("Resources Cache Database Explorer"));

    setButtons(Ok);

    m_page = new WdgDbExplorer(this);
    Q_CHECK_PTR(m_page);

    setMainWidget(m_page);

    {
        TableModel *storagesModel = new TableModel(this, QSqlDatabase::database());
        TableDelegate *storagesDelegate = new TableDelegate(m_page->tableStorages);
        storagesModel->setTable("storages");
        storagesModel->setHeaderData(0, Qt::Horizontal, i18n("Id"));
        storagesModel->setHeaderData(1, Qt::Horizontal, i18n("Type"));
        storagesModel->setRelation(1, QSqlRelation("storage_types", "id", "name"));
        storagesModel->setHeaderData(2, Qt::Horizontal, i18n("Location"));
        storagesModel->setHeaderData(3, Qt::Horizontal, i18n("Creation Date"));
        storagesModel->addDateTimeColumn(3);
        storagesDelegate->addDateTimeColumn(3);
        storagesModel->setHeaderData(4, Qt::Horizontal, i18n("Preinstalled"));
        storagesModel->addBooleanColumn(4);
        storagesDelegate->addBooleanColumn(4);
        storagesModel->setHeaderData(5, Qt::Horizontal, i18n("Active"));
        storagesModel->addBooleanColumn(5);
        storagesDelegate->addBooleanColumn(5);
        storagesModel->select();
        m_page->tableStorages->setModel(storagesModel);
        m_page->tableStorages->hideColumn(0);
        m_page->tableStorages->setItemDelegate(storagesDelegate);
        m_page->tableStorages->setSelectionMode(QAbstractItemView::SingleSelection);;
        m_page->tableStorages->resizeColumnsToContents();
    }

    {
        KisResourceModel *resourcesModel = new KisResourceModel("gradients");
        m_page->tableResources->setModel(resourcesModel);
        m_page->tableResources->hideColumn(0);
        m_page->tableResources->setSelectionMode(QAbstractItemView::SingleSelection);;
    }

    {
        TableModel *tagsModel = new TableModel(this, QSqlDatabase::database());
        TableDelegate *tagsDelegate = new TableDelegate(m_page->tableStorages);
        tagsDelegate->setEditable(true);
        tagsModel->setTable("tags");
        tagsModel->setHeaderData(0, Qt::Horizontal, i18n("Id"));
        tagsModel->setHeaderData(1, Qt::Horizontal, i18n("Type"));
        tagsModel->setRelation(1, QSqlRelation("resource_types", "id", "name"));
        tagsModel->setHeaderData(2, Qt::Horizontal, i18n("Tag"));
        tagsModel->setHeaderData(3, Qt::Horizontal, i18n("Name"));
        tagsModel->setHeaderData(4, Qt::Horizontal, i18n("Comment"));
        tagsModel->setHeaderData(5, Qt::Horizontal, i18n("Active"));
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
        m_resourceTypeModel = new KisResourceTypeModel(this);
//        m_page->cmbRvResourceTypes->addItems(QStringList() << "bla" << "asdsad" << "Adasd" << "wrwerwe");
        m_page->cmbRvResourceTypes->setModelColumn(KisResourceTypeModel::Name);
        m_page->cmbRvResourceTypes->setModel(m_resourceTypeModel);
        m_page->cmbRvResourceTypes->setItemDelegate(new KisResourceTypeDelegate(this));
        connect(m_page->cmbRvResourceTypes, SIGNAL(activated(int)), SLOT(slotRvResourceTypeSelected(int)));

        qDebug() << "combobox count" << m_page->cmbRvResourceTypes->count();

        m_tagModel = new KisTagModel("", this);
        m_page->cmbRvTags->setModelColumn(KisTagModel::Name);
        m_page->cmbRvTags->setModel(m_tagModel);
        connect(m_page->cmbRvTags, SIGNAL(activated(int)), SLOT(slotRvTagSelected(int)));

        m_resourceModel = 0;


        m_page->cmbRvResourceTypes->setCurrentIndex(0);

    }

}

DlgDbExplorer::~DlgDbExplorer()
{
}

void DlgDbExplorer::slotRvResourceTypeSelected(int index)
{
    QModelIndex idx = m_page->cmbRvResourceTypes->model()->index(index, KisResourceTypeModel::ResourceType);
    m_tagModel->setResourceType(idx.data(Qt::DisplayRole).toString());
}

void DlgDbExplorer::slotRvTagSelected(int index)
{
    qDebug() << "selected tag" << index;
}
