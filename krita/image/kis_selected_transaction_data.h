/*
 *  Copyright (c) 2005 C. Boemann <cbo@boemann.dk>
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

#ifndef KIS_SELECTED_TRANSACTION_DATA_H_
#define KIS_SELECTED_TRANSACTION_DATA_H_

#include "kis_transaction_data.h"

#include "kis_selection.h"

/**
 * KisSelectedTransactionData records changes to the selection for the undo stack. There
 * are two selections in Krita: the global selection and the per-layer selection mask.
 * A particular action only works with one of these selections (in the future, we may
 * want to merge the global and local selection).
 *
 * KisSelectedTransactionData remembers which selection was changed.
 */
class KRITAIMAGE_EXPORT KisSelectedTransactionData : public KisTransactionData
{

public:
    KisSelectedTransactionData(const QString& name, KisNodeSP node, KUndo2Command* parent = 0);
    virtual ~KisSelectedTransactionData();

public:
    void redo();
    void undo();
    void undoNoUpdate();
protected:
    void endTransaction();

protected:
    KisLayerSP layer();
private:
    KisLayerSP m_layer;
    KisTransactionData *m_selTransaction;
    bool m_hadSelection;
    bool m_redoHasSelection;
};

#endif // KIS_SELECTED_TRANSACTION_DATA_H_
