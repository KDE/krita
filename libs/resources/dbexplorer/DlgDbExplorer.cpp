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

#include <QTableView>
#include <QtSql/QSqlTableModel>

DlgDbExplorer::DlgDbExplorer(QWidget *parent)
    : KoDialog(parent)
{
    setCaption(i18n("Please paste this information in your bug report"));

    setButtons(Ok);

    m_page = new WdgDbExplorer(this);
    Q_CHECK_PTR(m_page);

    setMainWidget(m_page);

    QSqlTableModel *storagesModel = new QSqlTableModel(this);
    QSqlTableModel *tagsModel = new QSqlTableModel(this);
    QSqlTableModel *resourcesModel = new QSqlTableModel(this);
    QSqlTableModel *versionModel = new QSqlTableModel(this);
}

DlgDbExplorer::~DlgDbExplorer()
{
}
