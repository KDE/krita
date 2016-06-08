/* This file is part of the KDE project
 * Copyright (C) 2011 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "TableOfContentsStyleConfigure.h"
#include "ui_TableOfContentsStyleConfigure.h"

#include "KoStyleManager.h"
#include "KoParagraphStyle.h"

#include <QTableView>
#include <QHeaderView>

TableOfContentsStyleConfigure::TableOfContentsStyleConfigure(KoStyleManager *manager, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TableOfContentsStyleConfigure)
    , m_stylesTree(0)
    , m_styleManager(manager)
    , m_tocInfo(0)
    , m_stylesModel(0)
{
    ui->setupUi(this);
    setWindowTitle(i18n("Table of Contents - Configure Styles"));

    Q_ASSERT(manager);

    ui->stylesAvailableLabel->setText(i18n("Styles available"));
    connect(this, SIGNAL(accepted()), this, SLOT(save()));
}

TableOfContentsStyleConfigure::~TableOfContentsStyleConfigure()
{
    delete ui;
}

void TableOfContentsStyleConfigure::initializeUi(KoTableOfContentsGeneratorInfo *info)
{
    Q_ASSERT(info);

    m_tocInfo = info;

    connect(this, SIGNAL(accepted()), this, SLOT(save()));
    connect(this, SIGNAL(rejected()), this, SLOT(discardChanges()));

    m_stylesModel = new TableOfContentsStyleModel(m_styleManager, m_tocInfo);
    ui->tableView->setModel(m_stylesModel);

    ui->tableView->setItemDelegateForColumn(1, &m_delegate);

    ui->tableView->setShowGrid(false);
    ui->tableView->verticalHeader()->hide();
    ui->tableView->setEditTriggers(QAbstractItemView::CurrentChanged | QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->resizeSection(1, 100);

    this->setVisible(true);
}

void TableOfContentsStyleConfigure::save()
{
    if (m_stylesModel) {
        m_stylesModel->saveData();
        delete m_stylesModel;
        m_stylesModel = 0;
    }

    disconnect(this, SIGNAL(accepted()), this, SLOT(save()));
    disconnect(this, SIGNAL(rejected()), this, SLOT(discardChanges()));
}

void TableOfContentsStyleConfigure::discardChanges()
{
    if (m_stylesModel) {
        delete m_stylesModel;
        m_stylesModel = 0;
    }

    disconnect(this, SIGNAL(accepted()), this, SLOT(save()));
    disconnect(this, SIGNAL(rejected()), this, SLOT(discardChanges()));
}
