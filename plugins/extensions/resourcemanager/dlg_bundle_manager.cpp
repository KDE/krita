/*
 *  Copyright (c) 2014 Victor Lafon metabolic.ewilan@hotmail.fr
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
#include "dlg_bundle_manager.h"
#include "ui_wdgdlgbundlemanager.h"

#include "resourcemanager.h"
#include "dlg_create_bundle.h"

#include <QListWidget>
#include <QTreeWidget>
#include <QListWidgetItem>
#include <QPainter>
#include <QPixmap>
#include <QMessageBox>

#include <kis_icon.h>
#include "kis_action.h"
#include <KisResourceServerProvider.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <KoIcon.h>

#include <KisStorageModel.h>

DlgBundleManager::DlgBundleManager(QWidget *parent)
    : KoDialog(parent)
    , m_page(new QWidget())
    , m_ui(new Ui::WdgDlgBundleManager)
{
    setCaption(i18n("Manage Resource Libraries"));
    m_ui->setupUi(m_page);
    setMainWidget(m_page);
    resize(m_page->sizeHint());

    m_ui->bnAdd->setIcon(KisIconUtils::loadIcon("list-add"));
    connect(m_ui->bnAdd, SIGNAL(clicked(bool)), SLOT(addBundle()));

    m_ui->bnNew->setIcon(KisIconUtils::loadIcon("document-new"));
    connect(m_ui->bnNew, SIGNAL(clicked(bool)), SLOT(createBundle()));

    m_ui->bnDelete->setIcon(KisIconUtils::loadIcon("edit-delete"));
    connect(m_ui->bnDelete, SIGNAL(clicked(bool)), SLOT(deleteBundle()));

    setButtons(Close);

    m_storageModel = new KisStorageModel();
    m_ui->tableView->setModel(m_storageModel);

}

void DlgBundleManager::addBundle()
{

}

void DlgBundleManager::createBundle()
{

}

void DlgBundleManager::deleteBundle()
{

}
