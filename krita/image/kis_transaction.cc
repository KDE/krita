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
#include "kis_memento.h"
#include "kis_paint_device.h"
#include "kis_painterly_overlay.h"
#include "kis_datamanager.h"

class KisTransaction::Private {
public:
    QString name;
    KisPaintDeviceSP device;
    KisMementoSP memento;
    KisMementoSP overlayMemento;
    bool firstRedo;
};

KisTransaction::KisTransaction(const QString& name, KisPaintDeviceSP device, QUndoCommand* parent)
    : QUndoCommand(name, parent)
    , m_private( new Private() )
{
    m_private->device = device;
    m_private->memento = device->dataManager()->getMemento();

    if (m_private->device->painterlyOverlay())
      m_private->overlayMemento = device->painterlyOverlay()->dataManager()->getMemento();

    m_private->firstRedo = true;
}

KisTransaction::~KisTransaction()
{
    if (m_private->memento) {
        // For debugging purposes
        m_private->memento->setInvalid();
    }
    if (m_private->overlayMemento) {
        // For debugging purposes
        m_private->overlayMemento->setInvalid();
    }

    delete m_private;
}

void KisTransaction::redo()
{
    //QUndoStack calls redo(), so the first call needs to be blocked
    if(m_private->firstRedo)
    {
        m_private->firstRedo = false;
        return;
    }

    Q_ASSERT(!m_private->memento.isNull());

    m_private->device->dataManager()->rollforward(m_private->memento);

    QRect rc;
    qint32 x, y, width, height;
    m_private->memento->extent(x,y,width,height);
    rc.setRect(x + m_private->device->x(), y + m_private->device->y(), width, height);

    m_private->device->setDirty( rc );

    if (!m_private->overlayMemento.isNull()) {
        m_private->device->painterlyOverlay()->dataManager()->rollforward(m_private->overlayMemento);

        m_private->overlayMemento->extent(x,y,width,height);
        rc.setRect(x + m_private->device->painterlyOverlay()->x(),
                   y + m_private->device->painterlyOverlay()->y(), width, height);

        m_private->device->painterlyOverlay()->setDirty( rc );
    }
}

void KisTransaction::undo()
{
    Q_ASSERT(!m_private->memento.isNull());
    m_private->device->dataManager()->rollback(m_private->memento);

    QRect rc;
    qint32 x, y, width, height;
    m_private->memento->extent(x,y,width,height);
    rc.setRect(x + m_private->device->x(), y + m_private->device->y(), width, height);

    m_private->device->setDirty( rc );

    if (!m_private->overlayMemento.isNull()) {
        m_private->device->painterlyOverlay()->dataManager()->rollback(m_private->overlayMemento);

        m_private->overlayMemento->extent(x,y,width,height);
        rc.setRect(x + m_private->device->painterlyOverlay()->x(),
                   y + m_private->device->painterlyOverlay()->y(), width, height);

        m_private->device->painterlyOverlay()->setDirty( rc );
    }
}

void KisTransaction::undoNoUpdate()
{
    Q_ASSERT(!m_private->memento.isNull());

    m_private->device->dataManager()->rollback(m_private->memento);

    if (!m_private->overlayMemento.isNull())
        m_private->device->painterlyOverlay()->dataManager()->rollback(m_private->overlayMemento);
}
