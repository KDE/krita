/*
 *  SPDX-FileCopyrightText: 2019 Tusooa Zhu <tusooa@vista.aero>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "SnapshotDocker.h"

#include <QWidget>
#include <QListView>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "KisSnapshotModel.h"
#include "KisSnapshotView.h"

#include <kis_canvas2.h>
#include <kis_icon_utils.h>
#include <kis_action.h>
#include <kis_action_manager.h>
#include <KisViewManager.h>
#include <kis_signal_auto_connection.h>

struct SnapshotDocker::Private
{
    Private();
    ~Private();

    QScopedPointer<KisSnapshotModel> model;
    QPointer<KisSnapshotView> view;
    QPointer<KisCanvas2> canvas;
    QPointer<QToolButton> bnAdd;
    QPointer<QToolButton> bnSwitchTo;
    QPointer<QToolButton> bnRemove;
    KisSignalAutoConnectionsStore connections;
};

SnapshotDocker::Private::Private()
    : model(new KisSnapshotModel)
    , view(new KisSnapshotView)
    , canvas(0)
    , bnAdd(new QToolButton)
    , bnSwitchTo(new QToolButton)
    , bnRemove(new QToolButton)
{
}

SnapshotDocker::Private::~Private()
{
}

SnapshotDocker::SnapshotDocker()
    : QDockWidget()
    , m_d(new Private)
{
    QWidget *widget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(widget);
    m_d->view->setModel(m_d->model.data());
    mainLayout->addWidget(m_d->view);
    QHBoxLayout *buttonsLayout = new QHBoxLayout(widget);
    m_d->bnAdd->setIcon(KisIconUtils::loadIcon("list-add"));
    m_d->bnAdd->setToolTip(i18nc("@info:tooltip", "Create snapshot"));
    m_d->bnAdd->setAutoRaise(true);

    connect(m_d->bnAdd, &QToolButton::clicked, this, &SnapshotDocker::slotBnAddClicked);
    buttonsLayout->addWidget(m_d->bnAdd);
    m_d->bnSwitchTo->setIcon(KisIconUtils::loadIcon("snapshot-load"));
    m_d->bnSwitchTo->setToolTip(i18nc("@info:tooltip", "Switch to selected snapshot"));
    m_d->bnSwitchTo->setAutoRaise(true);
    connect(m_d->bnSwitchTo, &QToolButton::clicked, this, &SnapshotDocker::slotBnSwitchToClicked);

    buttonsLayout->addWidget(m_d->bnSwitchTo);
    m_d->bnRemove->setIcon(KisIconUtils::loadIcon("edit-delete"));
    m_d->bnRemove->setToolTip(i18nc("@info:tooltip", "Remove selected snapshot"));
    m_d->bnRemove->setAutoRaise(true);
    connect(m_d->bnRemove, &QToolButton::clicked, this, &SnapshotDocker::slotBnRemoveClicked);
    buttonsLayout->addWidget(m_d->bnRemove);
    mainLayout->addLayout(buttonsLayout);

    setWidget(widget);
    setWindowTitle(i18n("Snapshot Docker"));
}

SnapshotDocker::~SnapshotDocker()
{
}

void SnapshotDocker::setViewManager(KisViewManager *viewManager)
{
    m_d->connections.clear();
    KisAction *action = viewManager->actionManager()->createAction("create_snapshot");
    m_d->connections.addConnection(action, &KisAction::triggered,
                                   m_d->model.data(), &KisSnapshotModel::slotCreateSnapshot);
    action = viewManager->actionManager()->createAction("switchto_snapshot");
    m_d->connections.addConnection(action, &KisAction::triggered,
                                   m_d->view, &KisSnapshotView::slotSwitchToSelectedSnapshot);
    action = viewManager->actionManager()->createAction("remove_snapshot");
    m_d->connections.addConnection(action, &KisAction::triggered,
                                   m_d->view, &KisSnapshotView::slotRemoveSelectedSnapshot);
}

void SnapshotDocker::setCanvas(KoCanvasBase *canvas)
{
    KisCanvas2 *c = dynamic_cast<KisCanvas2 *>(canvas);
    if (c) {
        if (m_d->canvas == c) {
            return;
        }
    }
    m_d->canvas = c;
    m_d->model->setCanvas(c);
}

void SnapshotDocker::unsetCanvas()
{
    setCanvas(0);
}

void SnapshotDocker::slotBnAddClicked()
{
    if (m_d->canvas) {
        KisAction *action = m_d->canvas->viewManager()->actionManager()->actionByName("create_snapshot");
        action->trigger();
    }
}

void SnapshotDocker::slotBnSwitchToClicked()
{
    if (m_d->canvas) {
        KisAction *action = m_d->canvas->viewManager()->actionManager()->actionByName("switchto_snapshot");
        action->trigger();
    }
}

void SnapshotDocker::slotBnRemoveClicked()
{
    if (m_d->canvas) {
        KisAction *action = m_d->canvas->viewManager()->actionManager()->actionByName("remove_snapshot");
        action->trigger();
    }
}
