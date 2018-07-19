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


DlgDbExplorer::DlgDbExplorer(QWidget *parent)
    : KoDialog(parent)
{
    setCaption(i18n("Please paste this information in your bug report"));

    setButtons(Ok);

    m_page = new WdgDbExplorer(this);
    Q_CHECK_PTR(m_page);

    setMainWidget(m_page);

    QSqlRelationalTableModel *storagesModel = new QSqlRelationalTableModel(this, QSqlDatabase::database());
    storagesModel->setTable("storages");
    storagesModel->setHeaderData(0, Qt::Horizontal, i18n("Id"));
    storagesModel->setHeaderData(1, Qt::Horizontal, i18n("Type"));
    storagesModel->setRelation(1, QSqlRelation("storage_types", "id", "name"));
    storagesModel->setHeaderData(2, Qt::Horizontal, i18n("Location"));
    storagesModel->setHeaderData(3, Qt::Horizontal, i18n("Creation Date"));
    storagesModel->setHeaderData(4, Qt::Horizontal, i18n("Preinstalled"));
    storagesModel->setHeaderData(5, Qt::Horizontal, i18n("Active"));
    storagesModel->select();
    m_page->tableStorages->setModel(storagesModel);
    m_page->tableStorages->hideColumn(0);
    m_page->tableStorages->setItemDelegate(new QSqlRelationalDelegate(m_page->tableStorages));
    m_page->tableStorages->setSelectionMode(QAbstractItemView::SingleSelection);;


    QSqlTableModel *resourcesModel = new QSqlTableModel(this, QSqlDatabase::database());
    resourcesModel->setTable("resources");
    resourcesModel->select();
    m_page->tableResources->setModel(resourcesModel);
    m_page->tableResources->hideColumn(0);
    m_page->tableResources->setSelectionMode(QAbstractItemView::SingleSelection);;

    QSqlTableModel *tagsModel = new QSqlTableModel(this, QSqlDatabase::database());
    tagsModel->setTable("tags");
    tagsModel->select();
    m_page->tableTags->setModel(tagsModel);
    m_page->tableTags->hideColumn(0);
    m_page->tableTags->setSelectionMode(QAbstractItemView::SingleSelection);;

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


}

DlgDbExplorer::~DlgDbExplorer()
{
}
