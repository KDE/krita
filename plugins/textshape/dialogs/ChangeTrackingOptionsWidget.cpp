/* This file is part of the KDE project
 * Copyright (C) 2011 Ganesh Paramasivam <ganesh@crystalfab.com>
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

#include "ChangeTrackingOptionsWidget.h"
#include <QDebug>

ChangeTrackingOptionsWidget::ChangeTrackingOptionsWidget(TextTool *tool, QWidget *parent)
    : QWidget(parent)
    , m_tool(tool)
{
    widget.setupUi(this);
    connect(widget.recordChangesCheckBox, SIGNAL(stateChanged(int)), this, SLOT(recordChangesChanged(int)));
    connect(widget.showChangesCheckBox, SIGNAL(stateChanged(int)), this, SLOT(showChangesChanged(int)));
    connect(widget.configureChangeTrackingButton, SIGNAL(clicked(bool)), this, SLOT(configureSettingsPressed()));
}

void ChangeTrackingOptionsWidget::toggleShowChanges(bool isChecked)
{
    widget.showChangesCheckBox->setChecked(isChecked);
}

void ChangeTrackingOptionsWidget::toggleRecordChanges(bool isChecked)
{
    widget.recordChangesCheckBox->setChecked(isChecked);
}

void ChangeTrackingOptionsWidget::recordChangesChanged(int isChecked)
{
    m_tool->toggleRecordChanges(isChecked);
    emit doneWithFocus();
}

void ChangeTrackingOptionsWidget::showChangesChanged(int isChecked)
{
    m_tool->toggleShowChanges(isChecked);
    emit doneWithFocus();
}

void ChangeTrackingOptionsWidget::configureSettingsPressed()
{
    m_tool->configureChangeTracking();
    emit doneWithFocus();
}

