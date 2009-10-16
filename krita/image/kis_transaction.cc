/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#include "kis_transaction.h"


#include "kis_types.h"
#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_datamanager.h"

#include "config-tiles.h" // For the next define
#include KIS_MEMENTO_HEADER


class KisTransaction::Private
{
public:
    QString name;
    KisPaintDeviceSP device;
    KisMementoSP memento;
    bool firstRedo;
};

KisTransaction::KisTransaction(const QString& name, KisPaintDeviceSP device, QUndoCommand* parent)
        : QUndoCommand(name, parent)
        , m_d(new Private())
{
    m_d->device = device;
    m_d->memento = device->dataManager()->getMemento();
    m_d->firstRedo = true;
}

KisTransaction::~KisTransaction()
{
    if (m_d->memento) {
        m_d->memento->setInvalid();
    }
    delete m_d;
}

void KisTransaction::redo()
{
    //QUndoStack calls redo(), so the first call needs to be blocked
    if (m_d->firstRedo) {
        m_d->firstRedo = false;
        return;
    }

    Q_ASSERT(!m_d->memento.isNull());

    m_d->device->dataManager()->rollforward(m_d->memento);

    QRect rc;
    qint32 x, y, width, height;
    m_d->memento->extent(x, y, width, height);
    rc.setRect(x + m_d->device->x(), y + m_d->device->y(), width, height);

    m_d->device->setDirty(rc);
}

void KisTransaction::undo()
{
    Q_ASSERT(!m_d->memento.isNull());
    m_d->device->dataManager()->rollback(m_d->memento);

    QRect rc;
    qint32 x, y, width, height;
    m_d->memento->extent(x, y, width, height);
    rc.setRect(x + m_d->device->x(), y + m_d->device->y(), width, height);

    m_d->device->setDirty(rc);
}

void KisTransaction::undoNoUpdate()
{
    Q_ASSERT(!m_d->memento.isNull());
    m_d->device->dataManager()->rollback(m_d->memento);
}

#include "kis_transaction.moc"
