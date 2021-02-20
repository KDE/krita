/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "arrange_docker_widget.h"
#include "ui_arrange_docker_widget.h"

#include "kis_debug.h"

#include "kactioncollection.h"

#include <QAction>
#include <QToolButton>

struct ArrangeDockerWidget::Private
{
};

ArrangeDockerWidget::ArrangeDockerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ArrangeDockerWidget),
    m_d(new Private)
{
    ui->setupUi(this);
}

ArrangeDockerWidget::~ArrangeDockerWidget()
{
}


void replaceAction(QToolButton *button, QAction *newAction)
{
    Q_FOREACH (QAction *action, button->actions()) {
        button->removeAction(action);
    }

    if (newAction) {
        button->setDefaultAction(newAction);
    }
}

void ArrangeDockerWidget::setActionCollection(KActionCollection *collection)
{
    const bool enabled = collection->action("object_order_front");

    if (enabled) {
        replaceAction(ui->bringToFront, collection->action("object_order_front"));
        replaceAction(ui->raiseLevel, collection->action("object_order_raise"));
        replaceAction(ui->lowerLevel, collection->action("object_order_lower"));
        replaceAction(ui->sendBack, collection->action("object_order_back"));

        replaceAction(ui->leftAlign, collection->action("object_align_horizontal_left"));
        replaceAction(ui->hCenterAlign, collection->action("object_align_horizontal_center"));
        replaceAction(ui->rightAlign, collection->action("object_align_horizontal_right"));
        replaceAction(ui->topAlign, collection->action("object_align_vertical_top"));
        replaceAction(ui->vCenterAlign, collection->action("object_align_vertical_center"));
        replaceAction(ui->bottomAlign, collection->action("object_align_vertical_bottom"));

        replaceAction(ui->hDistributeLeft, collection->action("object_distribute_horizontal_left"));
        replaceAction(ui->hDistributeCenter, collection->action("object_distribute_horizontal_center"));
        replaceAction(ui->hDistributeRight, collection->action("object_distribute_horizontal_right"));
        replaceAction(ui->hDistributeGaps, collection->action("object_distribute_horizontal_gaps"));

        replaceAction(ui->vDistributeTop, collection->action("object_distribute_vertical_top"));
        replaceAction(ui->vDistributeCenter, collection->action("object_distribute_vertical_center"));
        replaceAction(ui->vDistributeBottom, collection->action("object_distribute_vertical_bottom"));
        replaceAction(ui->vDistributeGaps, collection->action("object_distribute_vertical_gaps"));


        replaceAction(ui->group, collection->action("object_group"));
        replaceAction(ui->ungroup, collection->action("object_ungroup"));
    }

    setEnabled(enabled);
}

void ArrangeDockerWidget::switchState(bool enabled)
{
    if (enabled) {
        ui->buttons->show();
        ui->disabledLabel->hide();
    } else {
        ui->buttons->hide();
        ui->disabledLabel->show();
    }
}


