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

#include <kis_canvas2.h>
#include <kis_icon_utils.h>

struct SnapshotDocker::Private
{
    Private();
    ~Private();

    QScopedPointer<KisSnapshotModel> model;
    QPointer<QListView> view;
    QPointer<KisCanvas2> canvas;
    QPointer<QToolButton> bnAdd;
    QPointer<QToolButton> bnRemove;
};

SnapshotDocker::Private::Private()
    : model(new KisSnapshotModel)
    , view(new QListView)
    , canvas(0)
    , bnAdd(new QToolButton)
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
    connect(m_d->view, &QListView::activated, m_d->model.data(), &KisSnapshotModel::slotSwitchToActivatedSnapshot);
    m_d->view->setModel(m_d->model.data());
    mainLayout->addWidget(m_d->view);

    QHBoxLayout *buttonsLayout = new QHBoxLayout(widget);
    m_d->bnAdd->setIcon(KisIconUtils::loadIcon("addlayer"));
    connect(m_d->bnAdd, &QToolButton::clicked, m_d->model.data(), &KisSnapshotModel::slotCreateSnapshot);
    buttonsLayout->addWidget(m_d->bnAdd);
    m_d->bnRemove->setIcon(KisIconUtils::loadIcon("deletelayer"));
    connect(m_d->bnRemove, &QToolButton::clicked, m_d->model.data(), &KisSnapshotModel::slotRemoveActivatedSnapshot);
    buttonsLayout->addWidget(m_d->bnRemove);
    mainLayout->addLayout(buttonsLayout);

    setWidget(widget);
    setWindowTitle(i18n("Snapshot Docker"));
}

SnapshotDocker::~SnapshotDocker()
{
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

#include "SnapshotDocker.moc"
