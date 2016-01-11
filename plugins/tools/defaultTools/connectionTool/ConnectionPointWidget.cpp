/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>
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

#include "ConnectionPointWidget.h"
#include "ConnectionTool.h"
#include <QAction>

ConnectionPointWidget::ConnectionPointWidget(ConnectionTool *tool, QWidget *parent)
    : QWidget(parent)
{
    widget.setupUi(this);
    widget.alignLeft->setDefaultAction(tool->action("align-left"));
    widget.alignCenterH->setDefaultAction(tool->action("align-centerh"));
    widget.alignRight->setDefaultAction(tool->action("align-right"));
    widget.alignTop->setDefaultAction(tool->action("align-top"));
    widget.alignCenterV->setDefaultAction(tool->action("align-centerv"));
    widget.alignBottom->setDefaultAction(tool->action("align-bottom"));
    widget.alignPercent->setDefaultAction(tool->action("align-relative"));

    widget.escapeAll->setDefaultAction(tool->action("escape-all"));
    widget.escapeHorz->setDefaultAction(tool->action("escape-horizontal"));
    widget.escapeVert->setDefaultAction(tool->action("escape-vertical"));
    widget.escapeLeft->setDefaultAction(tool->action("escape-left"));
    widget.escapeRight->setDefaultAction(tool->action("escape-right"));
    widget.escapeUp->setDefaultAction(tool->action("escape-up"));
    widget.escapeDown->setDefaultAction(tool->action("escape-down"));

    connect(widget.toggleEditMode, SIGNAL(stateChanged(int)), tool, SLOT(toggleConnectionPointEditMode(int)));
    connect(tool, SIGNAL(sendConnectionPointEditState(bool)), this, SLOT(toggleEditModeCheckbox(bool)));
}

void ConnectionPointWidget::toggleEditModeCheckbox(bool checked)
{
    widget.toggleEditMode->blockSignals(true);
    if (checked) {
        widget.toggleEditMode->setCheckState(Qt::Checked);
    } else {
        widget.toggleEditMode->setCheckState(Qt::Unchecked);
    }
    widget.toggleEditMode->blockSignals(false);
}
