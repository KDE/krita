/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TRANSACTION_DATA_H_
#define KIS_TRANSACTION_DATA_H_

#include <kundo2command.h>
#include "kis_types.h"
#include <kritaimage_export.h>


enum AutoKeyMode {
    AUTOKEY_DISABLED = 0,
    AUTOKEY_DUPLICATE,
    AUTOKEY_BLANK
};

/**
 * A tile based undo command.
 *
 * Ordinary KUndo2Command subclasses store parameters and apply the action in
 * the redo() command, however, Krita doesn't work like this. Undo replaces
 * the current tiles in a paint device with the old tiles, redo replaces them
 * again with the new tiles without actually executing the command that changed
 * the image data again.
 */
class KRITAIMAGE_EXPORT KisTransactionData : public KUndo2Command
{
public:
    KisTransactionData(const KUndo2MagicString& name, KisPaintDeviceSP device, bool resetSelectionOutlineCache, AutoKeyMode autoKeyMode, KUndo2Command* parent);
    ~KisTransactionData() override;

public:
    void redo() override;
    void undo() override;

    virtual void endTransaction();

protected:
    virtual void saveSelectionOutlineCache();
    virtual void restoreSelectionOutlineCache(bool undo);

private:
    void init(KisPaintDeviceSP device);
    void startUpdates();
    void possiblyNotifySelectionChanged();
    void possiblyResetOutlineCache();
    void possiblyFlattenSelection(KisPaintDeviceSP device);
    void doFlattenUndoRedo(bool undo);

private:
    class Private;
    Private * const m_d;
};

#endif /* KIS_TRANSACTION_DATA_H_ */

