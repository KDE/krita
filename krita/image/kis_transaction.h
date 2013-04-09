/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KIS_TRANSACTION_H_
#define KIS_TRANSACTION_H_

#include <kundo2command.h>

#include "kis_types.h"
#include <krita_export.h>

#include "kis_transaction_data.h"
#include "kis_selection_transaction_data.h"
#include "kis_paint_device.h"

#include "kis_undo_adapter.h"
#include "kis_post_execution_undo_adapter.h"

class KisTransaction
{
public:
    KisTransaction(const QString& name, KisPaintDeviceSP device, KUndo2Command* parent = 0) {
        m_transactionData = new KisTransactionData(name, device, parent);
    }

    virtual ~KisTransaction() {
        delete m_transactionData;
    }

    KUndo2Command* undoCommand() {
        return m_transactionData;
    }

    void commit(KisUndoAdapter* undoAdapter) {
        Q_ASSERT_X(m_transactionData, "KisTransaction::commit()",
                   "the transaction has been tried to be committed twice");

        m_transactionData->endTransaction();
        undoAdapter->addCommand(m_transactionData);
        m_transactionData = 0;
    }

    void commit(KisPostExecutionUndoAdapter* undoAdapter) {
        Q_ASSERT_X(m_transactionData, "KisTransaction::commit()",
                   "the transaction has been tried to be committed twice");

        m_transactionData->endTransaction();
        m_transactionData->redo();
        undoAdapter->addCommand(KUndo2CommandSP(m_transactionData));
        m_transactionData = 0;
    }

    KUndo2Command* endAndTake() {
        Q_ASSERT_X(m_transactionData, "KisTransaction::endAndTake()",
                   "the transaction has been tried to be committed twice");

        KisTransactionData *transactionData = m_transactionData;
        m_transactionData = 0;

        transactionData->endTransaction();
        return transactionData;
    }

    void end() {
        Q_ASSERT_X(m_transactionData, "KisTransaction::end()",
                   "nothing to end!");
        /**
         * We will not call endTransaction for m_transactionData,
         * we'll just kill it, and it'll report about it's death to
         * the memento manager, so no commit will be made
         */
        delete m_transactionData;
        m_transactionData = 0;
    }

    void revert() {
        Q_ASSERT_X(m_transactionData, "KisTransaction::reverted()",
                   "the transaction is tried to be reverted()"
                   "after it has already been added to undo adapter");

        m_transactionData->endTransaction();
        /**
         * FIXME: Should we emulate first redo() here?
         */
        m_transactionData->undo();
        delete m_transactionData;
        m_transactionData = 0;
    }

    QString text() const {
        Q_ASSERT_X(m_transactionData, "KisTransaction::name()",
                   "the name has been requested after the transaction"
                   "has already been ended");
        return m_transactionData->text();
    }

protected:
    KisTransaction() : m_transactionData(0) {}
    KisTransactionData* m_transactionData;
};

class KisSelectionTransaction : public KisTransaction
{
public:
    KisSelectionTransaction(const QString& name, KisUndoAdapter *undoAdapter, KisSelectionSP selection, KUndo2Command* parent = 0)
    {
        m_transactionData = new KisSelectionTransactionData(name, undoAdapter, selection, parent);
    }
};

#endif /* KIS_TRANSACTION_H_ */

