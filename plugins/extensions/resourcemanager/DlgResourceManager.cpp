/*
 * SPDX-FileCopyrightText: 2021 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "DlgResourceManager.h"

#include "ui_WdgDlgResourceManager.h"

#include <KisResourceTypeModel.h>

DlgResourceManager::DlgResourceManager(QWidget *parent)
    : KoDialog(parent)
    , m_ui(new Ui::WdgDlgResourceManager)
{
    setCaption(i18n("Manage Resources"));
    m_page = new QWidget(this);
    m_ui->setupUi(m_page);
    resize(m_page->size());
    setMainWidget(m_page);
    setButtons(Close);
    setDefaultButton(Close);

    m_resourceTypeModel = new KisResourceTypeModel(this);

    m_ui->cmbResourceType->setModel(m_resourceTypeModel);
    m_ui->cmbResourceType->setModelColumn(KisResourceTypeModel::Name);
    connect(m_ui->cmbResourceType, SIGNAL(activated(int)), SLOT(slotResourceTypeSelected(int)));

    connect(m_ui->btnImportBundle, SIGNAL(clicked(bool)), SLOT(slotImportBundle()));
    connect(m_ui->btnOpenResourceFolder, SIGNAL(clicked(bool)), SLOT(slotOpenResourceFolder()));
    connect(m_ui->btnImportResources, SIGNAL(clicked(bool)), SLOT(slotImportResources()));
    connect(m_ui->btnDeleteBackupFiles, SIGNAL(clicked(bool)), SLOT(slotDeleteBackupFiles()));
}

DlgResourceManager::~DlgResourceManager()
{

}

void DlgResourceManager::slotResourceTypeSelected(int)
{
    // TODO: handle resource type selection change
}

void DlgResourceManager::slotImportResources()
{

}

void DlgResourceManager::slotOpenResourceFolder()
{

}

void DlgResourceManager::slotImportBundle()
{

}

void DlgResourceManager::slotDeleteBackupFiles()
{

}
