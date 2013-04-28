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

#include "kis_transaction_data.h"

#include "kis_paint_device.h"
#include "kis_datamanager.h"

#include "config-tiles.h" // For the next define
#include KIS_MEMENTO_HEADER


//#define DEBUG_TRANSACTIONS

#ifdef DEBUG_TRANSACTIONS
    #define DEBUG_ACTION(action) qDebug() << action << "for" << m_d->device->dataManager()
#else
    #define DEBUG_ACTION(action)
#endif


class KisTransactionData::Private
{
public:
    KisPaintDeviceSP device;
    KisMementoSP memento;
    bool firstRedo;
    bool transactionFinished;
    QPoint oldOffset;
    QPoint newOffset;
};

KisTransactionData::KisTransactionData(const QString& name, KisPaintDeviceSP device, KUndo2Command* parent)
        : KUndo2Command(name, parent)
        , m_d(new Private())
{
    m_d->device = device;
    DEBUG_ACTION("Transaction started");
    m_d->memento = device->dataManager()->getMemento();
    m_d->oldOffset = QPoint(device->x(), device->y());
    m_d->firstRedo = true;
    m_d->transactionFinished = false;
}

KisTransactionData::~KisTransactionData()
{
    Q_ASSERT(m_d->memento);
    m_d->device->dataManager()->purgeHistory(m_d->memento);

    delete m_d;
}

void KisTransactionData::endTransaction()
{
    if(!m_d->transactionFinished) {
        DEBUG_ACTION("Transaction ended");
        m_d->transactionFinished = true;
        m_d->device->dataManager()->commit();
        m_d->newOffset = QPoint(m_d->device->x(), m_d->device->y());
    }
}

void KisTransactionData::startUpdates()
{
    QRect rc;
    QRect mementoExtent = m_d->memento->extent();

    if (m_d->newOffset == m_d->oldOffset) {
        rc = mementoExtent.translated(m_d->device->x(), m_d->device->y());
    } else {
        QRect totalExtent =
            m_d->device->dataManager()->extent() | mementoExtent;

        rc = totalExtent.translated(m_d->oldOffset) |
            totalExtent.translated(m_d->newOffset);
    }

    m_d->device->setDirty(rc);
}

void KisTransactionData::redo()
{
    //KUndo2QStack calls redo(), so the first call needs to be blocked
    if (m_d->firstRedo) {
        m_d->firstRedo = false;
        return;
    }

    DEBUG_ACTION("Redo()");

    Q_ASSERT(m_d->memento);
    m_d->device->dataManager()->rollforward(m_d->memento);

    if (m_d->newOffset != m_d->oldOffset) {
        m_d->device->move(m_d->newOffset);
    }

    startUpdates();
}

void KisTransactionData::undo()
{
    DEBUG_ACTION("Undo()");

    Q_ASSERT(m_d->memento);
    m_d->device->dataManager()->rollback(m_d->memento);

    if (m_d->newOffset != m_d->oldOffset) {
        m_d->device->move(m_d->oldOffset);
    }

    startUpdates();
}

void KisTransactionData::undoNoUpdate()
{
    Q_ASSERT(m_d->memento);
    m_d->device->dataManager()->rollback(m_d->memento);
}
