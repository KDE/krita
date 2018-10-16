/*
 * This file is part of the KDE project
 * Copyright (C) 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_input_configuration_page_item.h"

#include "kis_icon_utils.h"

#include "input/kis_abstract_input_action.h"
#include "input/kis_input_profile_manager.h"
#include "kis_action_shortcuts_model.h"
#include "kis_input_type_delegate.h"
#include "kis_input_mode_delegate.h"
#include "kis_input_editor_delegate.h"
#include "ui_kis_input_configuration_page_item.h"

#include <QAction>

KisInputConfigurationPageItem::KisInputConfigurationPageItem(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    ui = new Ui::KisInputConfigurationPageItem;
    this->setContentsMargins(0,0,0,0);
    ui->setupUi(this);

    m_shortcutsModel = new KisActionShortcutsModel(this);
    ui->shortcutsView->setModel(m_shortcutsModel);
    ui->shortcutsView->setItemDelegateForColumn(0, new KisInputTypeDelegate(ui->shortcutsView));
    ui->shortcutsView->setItemDelegateForColumn(1, new KisInputEditorDelegate(ui->shortcutsView));
    ui->shortcutsView->setItemDelegateForColumn(2, new KisInputModeDelegate(ui->shortcutsView));
    ui->shortcutsView->header()->setSectionResizeMode(QHeaderView::Stretch);
    setExpanded(false);

    QAction *deleteAction = new QAction(KisIconUtils::loadIcon("edit-delete"), i18n("Delete Shortcut"), ui->shortcutsView);
    connect(deleteAction, SIGNAL(triggered(bool)), SLOT(deleteShortcut()));
    ui->shortcutsView->addAction(deleteAction);
    ui->shortcutsView->setContextMenuPolicy(Qt::ActionsContextMenu);

    connect(ui->collapseButton, SIGNAL(clicked(bool)), SLOT(setExpanded(bool)));
}

KisInputConfigurationPageItem::~KisInputConfigurationPageItem()
{
    delete ui;
}

void KisInputConfigurationPageItem::setAction(KisAbstractInputAction *action)
{
    m_action = action;
    ui->collapseButton->setText(action->name());
    ui->descriptionLabel->setText(action->description());
    m_shortcutsModel->setProfile(KisInputProfileManager::instance()->currentProfile());
    m_shortcutsModel->setAction(action);
    qobject_cast<KisInputModeDelegate *>(ui->shortcutsView->itemDelegateForColumn(2))->setAction(action);
}

void KisInputConfigurationPageItem::setExpanded(bool expand)
{
    if (expand) {
        ui->descriptionLabel->setVisible(true);
        ui->shortcutsView->setVisible(true);
        ui->collapseButton->setArrowType(Qt::DownArrow);
    }
    else {
        ui->descriptionLabel->setVisible(false);
        ui->shortcutsView->setVisible(false);
        ui->collapseButton->setArrowType(Qt::RightArrow);
    }
}

void KisInputConfigurationPageItem::deleteShortcut()
{
    int row = ui->shortcutsView->selectionModel()->currentIndex().row();

    if (m_shortcutsModel->canRemoveRow(row)) {
        m_shortcutsModel->removeRow(row, QModelIndex());
    }
}
