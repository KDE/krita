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

#include <mutex>

class QBitArray;
class KisUndoAdapter;
class KisPostExecutionUndoAdapter;
class KisPainter;
class KUndo2MagicString;
class KoCompositeOp;
class KoColor;
class KisRunnableStrokeJobData;

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
    void mergeToLayer(KisNodeSP layer, KisPostExecutionUndoAdapter *undoAdapter, const KUndo2MagicString &transactionText, int timedID);
    virtual void mergeToLayerThreaded(KisNodeSP layer, KisPostExecutionUndoAdapter *undoAdapter, const KUndo2MagicString &transactionText, int timedID, QVector<KisRunnableStrokeJobData *> *jobs);

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

    /**
     * A simple RAII-styled class to release the write lock for the
     * final merge while the stroke is suspended.
     */
    struct FinalMergeSuspender {
        FinalMergeSuspender(KisIndirectPaintingSupport *indirect);
        ~FinalMergeSuspender();

    private:
        const KisIndirectPaintingSupport *m_lock;
    };
    using FinalMergeSuspenderSP = QSharedPointer<FinalMergeSuspender>;

    /**
     * When the stroke uses multithreaded final merge and supports
     * suspension, then it should also suspend the final merge explicitly
     * by requesting a special RAII object for the whole period of
     * suspension.
     */
    FinalMergeSuspenderSP trySuspendFinalMerge();

protected:
    /**
     * A guard object to lock the temporary target for write
     */
    struct WriteLocker {
        WriteLocker(KisIndirectPaintingSupport *lock)
            : m_lock(lock),
              m_locked(true)
        {
            m_lock->lockTemporaryTargetForWrite();
        }

        WriteLocker(KisIndirectPaintingSupport *lock, std::defer_lock_t)
            : m_lock(lock),
              m_locked(false)
        {
        }

        ~WriteLocker() {
            if (m_locked) {
                m_lock->unlockTemporaryTarget();
            }
        }

        void unlock() {
            KIS_SAFE_ASSERT_RECOVER_RETURN(m_locked);
            m_lock->unlockTemporaryTarget();
            m_locked = false;
        }

        void relock() {
            KIS_SAFE_ASSERT_RECOVER_RETURN(!m_locked);
            m_lock->lockTemporaryTargetForWrite();
            m_locked = true;
        }

        bool isLocked() {
            return m_locked;
        }

    private:
        KisIndirectPaintingSupport *m_lock;
        bool m_locked = false;
    };

    using WriteLockerSP = QSharedPointer<WriteLocker>;

    void mergeToLayerImpl(KisPaintDeviceSP dst, KisPostExecutionUndoAdapter *undoAdapter, const KUndo2MagicString &transactionText, int timedID, bool cleanResources, WriteLockerSP sharedWriteLock, QVector<KisRunnableStrokeJobData *> *jobs);
    virtual void writeMergeData(KisPainter *painter, KisPaintDeviceSP src, const QRect &rc);
    void lockTemporaryTargetForWrite() const;

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
