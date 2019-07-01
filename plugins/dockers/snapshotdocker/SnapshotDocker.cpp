/*
 *  Copyright (c) 2019 Tusooa Zhu <tusooa@vista.aero>
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
    m_d->bnAdd->setIcon(KisIconUtils::loadIcon("addlayer"));
    m_d->bnAdd->setToolTip(i18nc("@info:tooltip", "Create snapshot"));
    connect(m_d->bnAdd, &QToolButton::clicked, this, &SnapshotDocker::slotBnAddClicked);
    buttonsLayout->addWidget(m_d->bnAdd);
    m_d->bnSwitchTo->setIcon(KisIconUtils::loadIcon("draw-freehand")); /// XXX: which icon to use?
    m_d->bnSwitchTo->setToolTip(i18nc("@info:tooltip", "Switch to selected snapshot"));
    connect(m_d->bnSwitchTo, &QToolButton::clicked, this, &SnapshotDocker::slotBnSwitchToClicked);
    buttonsLayout->addWidget(m_d->bnSwitchTo);
    m_d->bnRemove->setIcon(KisIconUtils::loadIcon("deletelayer"));
    m_d->bnRemove->setToolTip(i18nc("@info:tooltip", "Remove selected snapshot"));
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

#include "SnapshotDocker.moc"
