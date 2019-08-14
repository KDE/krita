/*
 *  Copyright (c) 2019 Boudewijn Rempt <boud@valdyas.org>
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

#include "ActionSearchWidget.h"

#include <QDebug>
#include <QPushButton>
#include <QFrame>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QAction>
#include <QVariant>

#include <kis_global.h>

#include "ActionModel.h"
#include "ActionFilterModel.h"

class ActionSearchWidget::Private
{
public:
    ActionFilterModel *searchModel;

};

ActionSearchWidget::ActionSearchWidget(KActionCollection *actionCollection, QWidget *parent)
    : QWidget(parent)
    , Ui_WdgActionSearch()
    , d(new ActionSearchWidget::Private())
{
    setupUi(this);
    connect(bnTrigger, SIGNAL(pressed()), SIGNAL(actionTriggered()));
    connect(lstAction, SIGNAL(activated(QModelIndex)), SLOT(actionSelected(QModelIndex)));
    ActionModel *actionModel = new ActionModel(actionCollection, this);
    d->searchModel = new ActionFilterModel(this);
    d->searchModel->setSourceModel(actionModel);
    lstAction->setModel(actionModel);
}

ActionSearchWidget::~ActionSearchWidget()
{
}

void ActionSearchWidget::actionSelected(const QModelIndex &idx)
{
    lblWhatsThis->setText(idx.data(Qt::WhatsThisRole).toString());
}

