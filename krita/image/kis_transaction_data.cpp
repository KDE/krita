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

#include "kis_pixel_selection.h"
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

    bool savedOutlineCacheValid;
    QPainterPath savedOutlineCache;
    KUndo2Command *flattenUndoCommand;
    bool resetSelectionOutlineCache;
};

KisTransactionData::KisTransactionData(const QString& name, KisPaintDeviceSP device, bool resetSelectionOutlineCache, KUndo2Command* parent)
    : KUndo2Command(name, parent)
    , m_d(new Private())
{
    m_d->resetSelectionOutlineCache = resetSelectionOutlineCache;

    init(device);
    saveSelectionOutlineCache();
}

void KisTransactionData::init(KisPaintDeviceSP device)
{
    m_d->device = device;
    DEBUG_ACTION("Transaction started");
    m_d->memento = device->dataManager()->getMemento();
    m_d->oldOffset = QPoint(device->x(), device->y());
    m_d->firstRedo = true;
    m_d->transactionFinished = false;
    m_d->flattenUndoCommand = 0;
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

void KisTransactionData::possiblyNotifySelectionChanged()
{
    KisPixelSelectionSP pixelSelection =
        dynamic_cast<KisPixelSelection*>(m_d->device.data());

    KisSelectionSP selection;
    if (pixelSelection && (selection = pixelSelection->parentSelection())) {
        selection->notifySelectionChanged();
    }
}

void KisTransactionData::possiblyResetOutlineCache()
{
    KisPixelSelectionSP pixelSelection;

    if (m_d->resetSelectionOutlineCache &&
        (pixelSelection =
         dynamic_cast<KisPixelSelection*>(m_d->device.data()))) {

        pixelSelection->invalidateOutlineCache();
    }
}

void KisTransactionData::redo()
{
    //KUndo2QStack calls redo(), so the first call needs to be blocked
    if (m_d->firstRedo) {
        m_d->firstRedo = false;

        possiblyResetOutlineCache();
        possiblyNotifySelectionChanged();
        return;
    }

    restoreSelectionOutlineCache(false);

    DEBUG_ACTION("Redo()");

    Q_ASSERT(m_d->memento);
    m_d->device->dataManager()->rollforward(m_d->memento);

    if (m_d->newOffset != m_d->oldOffset) {
        m_d->device->move(m_d->newOffset);
    }

    startUpdates();
    possiblyNotifySelectionChanged();
}

void KisTransactionData::undo()
{
    DEBUG_ACTION("Undo()");

    Q_ASSERT(m_d->memento);
    m_d->device->dataManager()->rollback(m_d->memento);

    if (m_d->newOffset != m_d->oldOffset) {
        m_d->device->move(m_d->oldOffset);
    }

    restoreSelectionOutlineCache(true);

    startUpdates();
    possiblyNotifySelectionChanged();
}

void KisTransactionData::saveSelectionOutlineCache()
{
    m_d->savedOutlineCacheValid = false;

    KisPixelSelectionSP pixelSelection =
        dynamic_cast<KisPixelSelection*>(m_d->device.data());

    if (pixelSelection) {
        m_d->savedOutlineCacheValid = pixelSelection->outlineCacheValid();
        if (m_d->savedOutlineCacheValid) {
            m_d->savedOutlineCache = pixelSelection->outlineCache();

            possiblyResetOutlineCache();
        }

        KisSelectionSP selection = pixelSelection->parentSelection();
        if (selection) {
            m_d->flattenUndoCommand = selection->flatten();
            if (m_d->flattenUndoCommand) {
                m_d->flattenUndoCommand->redo();
            }
        }
    }
}

void KisTransactionData::restoreSelectionOutlineCache(bool undo)
{
    KisPixelSelectionSP pixelSelection =
        dynamic_cast<KisPixelSelection*>(m_d->device.data());

    if (pixelSelection) {
        bool savedOutlineCacheValid;
        QPainterPath savedOutlineCache;

        savedOutlineCacheValid = pixelSelection->outlineCacheValid();
        if (savedOutlineCacheValid) {
            savedOutlineCache = pixelSelection->outlineCache();
        }

        if (m_d->savedOutlineCacheValid) {
            pixelSelection->setOutlineCache(m_d->savedOutlineCache);
        } else {
            pixelSelection->invalidateOutlineCache();
        }

        m_d->savedOutlineCacheValid = savedOutlineCacheValid;
        if (m_d->savedOutlineCacheValid) {
            m_d->savedOutlineCache = savedOutlineCache;
        }

        if (m_d->flattenUndoCommand) {
            if (undo) {
                m_d->flattenUndoCommand->undo();
            } else {
                m_d->flattenUndoCommand->redo();
            }
        }
    }
}
