/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TRANSACTION_H_
#define KIS_TRANSACTION_H_

#include <kundo2command.h>

#include "kis_types.h"

#include "kis_transaction_data.h"
#include "kis_paint_device.h"
#include "kis_pixel_selection.h"
#include "kis_image_config.h"

#include "kis_undo_adapter.h"
#include "kis_post_execution_undo_adapter.h"

class KisTransaction
{
public:

    KisTransaction(const KUndo2MagicString& name, KisPaintDeviceSP device, AutoKeyMode autoKeyMode, KUndo2Command* parent = 0, int timedID = -1) {
        m_transactionData = new KisTransactionData(name, device, true, autoKeyMode, parent);
        m_transactionData->setTimedID(timedID);
    }

    KisTransaction(KisPaintDeviceSP device, AutoKeyMode autoKeyMode, KUndo2Command* parent = 0, int timedID = -1)
        : KisTransaction(KUndo2MagicString(), device, autoKeyMode, parent, timedID){
    }

    KisTransaction(const KUndo2MagicString& name, KisPaintDeviceSP device, KUndo2Command* parent = 0,int timedID = -1) {

        KisImageConfig cfg(true);
        AutoKeyMode autoKeyMode = AUTOKEY_DISABLED;

        if (cfg.autoKeyEnabled()) {
            if (cfg.autoKeyModeDuplicate()) {
                autoKeyMode = AUTOKEY_DUPLICATE;
            } else {
                autoKeyMode = AUTOKEY_BLANK;
            }
        }

        m_transactionData = new KisTransactionData(name, device, true, autoKeyMode, parent);
        m_transactionData->setTimedID(timedID);

    }

    KisTransaction(KisPaintDeviceSP device, KUndo2Command* parent = 0,int timedID = -1)
        : KisTransaction(KUndo2MagicString(), device, parent, timedID){
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

    KUndo2MagicString text() const {
        Q_ASSERT_X(m_transactionData, "KisTransaction::text()",
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
    KisSelectionTransaction(KisPixelSelectionSP pixelSelection, KUndo2Command* parent = 0)
    {
        m_transactionData = new KisTransactionData(KUndo2MagicString(), pixelSelection, false, AUTOKEY_DISABLED, parent);
    }

    KisSelectionTransaction(const KUndo2MagicString& name, KisPixelSelectionSP pixelSelection, KUndo2Command* parent = 0)
    {
        m_transactionData = new KisTransactionData(name, pixelSelection, false, AUTOKEY_DISABLED, parent);
    }
};

#endif /* KIS_TRANSACTION_H_ */

