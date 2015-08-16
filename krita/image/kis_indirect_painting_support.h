/*
 *  Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>
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

#ifndef KIS_INDIRECT_PAINTING_SUPPORT_H_
#define KIS_INDIRECT_PAINTING_SUPPORT_H_

#include "kritaimage_export.h"
#include "kis_types.h"
#include "kis_node.h"

class QBitArray;
class KisUndoAdapter;
class KisPostExecutionUndoAdapter;
class KisPainter;
class KUndo2MagicString;

/**
 * For classes that support indirect painting.
 *
 * XXX: Name doesn't suggest an object -- is KisIndirectPaintingLayer
 * a better name? (BSAR)
 */
class KRITAIMAGE_EXPORT KisIndirectPaintingSupport
{
    KisIndirectPaintingSupport(const KisIndirectPaintingSupport&);
    KisIndirectPaintingSupport& operator=(const KisIndirectPaintingSupport&);
public:

    KisIndirectPaintingSupport();
    virtual ~KisIndirectPaintingSupport();

    bool hasTemporaryTarget() const;

    void setTemporaryTarget(KisPaintDeviceSP t);
    void setTemporaryCompositeOp(const KoCompositeOp* c);
    void setTemporaryOpacity(quint8 o);
    void setTemporaryChannelFlags(const QBitArray& channelFlags);
    void setTemporarySelection(KisSelectionSP selection);

    /**
     * Configures the painter to conform the painting parameters
     * stored for th temporary target, such as compositeOp, opacity,
     * channel flags and selection. Please do not setup them manually,
     * but use this function instead.
     */
    void setupTemporaryPainter(KisPainter *painter) const;

    /**
     * Writes the temporary target into the paint device of the layer.
     * This action will lock the temporary target itself.
     */
    void mergeToLayer(KisNodeSP layer, KisUndoAdapter *undoAdapter, const KUndo2MagicString &transactionText,int timedID = -1);
    void mergeToLayer(KisNodeSP layer, KisPostExecutionUndoAdapter *undoAdapter, const KUndo2MagicString &transactionText,int timedID = -1);


    /**
     * Lock the temporary target.
     * It should be done for guarding every access to
     * temporaryTarget() or original()
     * NOTE: well, not "every", but...
     */
    void lockTemporaryTarget() const;

    /**
     * Unlock the temporary target
     * 
     * \see lockTemporaryTarget()
     */
    void unlockTemporaryTarget() const;

    KisPaintDeviceSP temporaryTarget();
    const KisPaintDeviceSP temporaryTarget() const;

private:
    friend class KisPainterBasedStrokeStrategy;

    /**
     * Only for debugging purposes. Please use setupTemporaryPainer()
     * instead.
     */
    KisSelectionSP temporarySelection() const;

private:

    template<class UndoAdapter>
        void mergeToLayerImpl(KisNodeSP layer,
                              UndoAdapter *undoAdapter,
                              const KUndo2MagicString &transactionText,int timedID = -1);
    void releaseResources();

private:
    struct Private;
    Private* const d;
};


#endif /* KIS_INDIRECT_PAINTING_SUPPORT_H_ */
