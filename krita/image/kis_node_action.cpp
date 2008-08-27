/*
 * Copyright (C) (C) 2007, Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_node_action.h"
#include "kis_node.h"
#include "KoProgressUpdater.h"

struct KisNodeAction::Private {
    KisNodeSP node;
    KoProgressUpdater * updater;
};

KisNodeAction::KisNodeAction(QObject * parent, KisNodeSP node, KoProgressProxy * progressProxy)
        : KoAction(parent)
        , m_d(new Private)
{
    m_d->node = node;
    m_d->node->setLocked(true);
    m_d->updater = new KoProgressUpdater(progressProxy);
    connect(this, SIGNAL(triggered(const QVariant &)), this, SLOT(slotTriggered()), Qt::DirectConnection);
    connect(this, SIGNAL(updateUi(const QVariant &)), this, SLOT(slotUpdateGUI()), Qt::DirectConnection);
}

KisNodeAction::~KisNodeAction()
{
    delete m_d->updater;
    delete m_d;
}

void KisNodeAction::slotUpdateGUI()
{
    // XXX: make sure the gui responds to this
    m_d->node->setLocked(false);
}

#include "kis_node_action.moc"
