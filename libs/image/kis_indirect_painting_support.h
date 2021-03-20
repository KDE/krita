/*
 *  SPDX-FileCopyrightText: 2004 Bart Coppens <kde@bartcoppens.be>
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_INDIRECT_PAINTING_SUPPORT_H_
#define KIS_INDIRECT_PAINTING_SUPPORT_H_

#include "kritaimage_export.h"
#include "kis_types.h"

class QBitArray;
class KisUndoAdapter;
class KisPostExecutionUndoAdapter;
class KisPainter;
class KUndo2MagicString;
class KoCompositeOp;
class KoColor;

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

    virtual void setCurrentColor(const KoColor &color);
    void setTemporaryTarget(KisPaintDeviceSP t);
    void setTemporaryCompositeOp(const QString &id);
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
    virtual void mergeToLayer(KisNodeSP layer, KisPostExecutionUndoAdapter *undoAdapter, const KUndo2MagicString &transactionText, int timedID = -1);

    KisPaintDeviceSP temporaryTarget() const;

    virtual bool supportsNonIndirectPainting() const;

    /**
     * A guard object to lock the temporary target for read
     */
    struct ReadLocker {
        ReadLocker(const KisIndirectPaintingSupport *lock) : m_lock(lock) {
            m_lock->lockTemporaryTarget();
        }
        ~ReadLocker() {
            m_lock->unlockTemporaryTarget();
        }

    private:
        const KisIndirectPaintingSupport *m_lock;
    };

protected:
    void mergeToLayerImpl(KisPaintDeviceSP dst, KisPostExecutionUndoAdapter *undoAdapter, const KUndo2MagicString &transactionText, int timedID = -1, bool cleanResources = true);
    virtual void writeMergeData(KisPainter *painter, KisPaintDeviceSP src);
    void lockTemporaryTargetForWrite() const;

    /**
     * A guard object to lock the temporary target for write
     */
    struct WriteLocker {
        WriteLocker(KisIndirectPaintingSupport *lock) : m_lock(lock) {
            m_lock->lockTemporaryTargetForWrite();
        }
        ~WriteLocker() {
            m_lock->unlockTemporaryTarget();
        }

    private:
        KisIndirectPaintingSupport *m_lock;
    };

    QString temporaryCompositeOp() const;
    void releaseResources();

private:
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

private:
    friend class KisPainterBasedStrokeStrategy;

    /**
     * Only for debugging purposes. Please use setupTemporaryPainer()
     * instead.
     */
    KisSelectionSP temporarySelection() const;

private:
    struct Private;
    Private* const d;
};


#endif /* KIS_INDIRECT_PAINTING_SUPPORT_H_ */
