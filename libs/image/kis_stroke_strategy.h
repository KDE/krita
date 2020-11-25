/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_STROKE_STRATEGY_H
#define __KIS_STROKE_STRATEGY_H

#include <QString>
#include "kis_types.h"
#include "kundo2magicstring.h"
#include "kritaimage_export.h"
#include "KisLodPreferences.h"

class KisStrokeJobStrategy;
class KisStrokeJobData;
class KisStrokesQueueMutatedJobInterface;

class KRITAIMAGE_EXPORT KisStrokeStrategy
{
public:
    KisStrokeStrategy(const QLatin1String &id, const KUndo2MagicString &name = KUndo2MagicString());
    virtual ~KisStrokeStrategy();

    /**
     * notifyUserStartedStroke() is a callback used by the strokes system to notify
     * when the user adds the stroke to the strokes queue. That moment corresponds
     * to the user calling strokesFacade->startStroke(strategy) and might happen much
     * earlier than the first job being executed.
     *
     * NOTE: this method will be executed in the context of the GUI thread!
     */
    virtual void notifyUserStartedStroke();

    /**
     * notifyUserEndedStroke() is a callback used by the strokes system to notify
     * when the user ends the stroke. That moment corresponds to the user calling
     * strokesFacade->endStroke(id) and might happen much earlier when the stroke
     * even started its execution.
     *
     * NOTE: this method will be executed in the context of the GUI thread!
     */
    virtual void notifyUserEndedStroke();

    virtual KisStrokeJobStrategy* createInitStrategy();
    virtual KisStrokeJobStrategy* createFinishStrategy();
    virtual KisStrokeJobStrategy* createCancelStrategy();
    virtual KisStrokeJobStrategy* createDabStrategy();
    virtual KisStrokeJobStrategy* createSuspendStrategy();
    virtual KisStrokeJobStrategy* createResumeStrategy();

    virtual KisStrokeJobData* createInitData();
    virtual KisStrokeJobData* createFinishData();
    virtual KisStrokeJobData* createCancelData();
    virtual KisStrokeJobData* createSuspendData();
    virtual KisStrokeJobData* createResumeData();

    virtual KisStrokeStrategy* createLodClone(int levelOfDetail);
    virtual KisStrokeStrategy* createLegacyInitializingStroke();

    bool isExclusive() const;
    bool supportsWrapAroundMode() const;


    /**
     * Returns true if mere start of the stroke should cancel all the
     * pending redo tasks.
     *
     * This method should return true in almost all circumstances
     * except if we are running an undo or redo stroke.
     */
    bool clearsRedoOnStart() const;

    /**
     * Returns true if the other currently running strokes should be
     * politely asked to exit. The default value is 'true'.
     *
     * The only known exception right now is
     * KisRegenerateFrameStrokeStrategy which does not requests ending
     * of any actions, since it performs purely background action.
     */
    bool requestsOtherStrokesToEnd() const;

    /**
     * Returns true if the update scheduler can cancel this stroke
     * when some other stroke is going to be started. This makes the
     * "forgettable" stroke very low priority.
     *
     * Default is 'false'.
     */
    bool canForgetAboutMe() const;

    bool needsExplicitCancel() const;

    /**
     * \see setBalancingRatioOverride() for details
     */
    qreal balancingRatioOverride() const;

    QString id() const;
    KUndo2MagicString name() const;

    /**
     * Returns current lod preferences of the strokes queue
     */
    KisLodPreferences currentLodPreferences() const;

    void setMutatedJobsInterface(KisStrokesQueueMutatedJobInterface *mutatedJobsInterface, KisStrokeId strokeId);

protected:
    // testing surrogate class
    friend class KisMutatableDabStrategy;

    /**
     * This function is supposed to be called by internal asynchronous
     * jobs. It allows adding subtasks that may be executed concurrently.
     *
     * Requirements:
     *   * must be called *only* from within the context of the strokes
     *     worker thread during execution of one of its jobs
     *
     * Guarantees:
     *   * the added job is guaranteed to be executed in some time after
     *     the currently executed job, *before* the next SEQUENTIAL or
     *     BARRIER job
     *   * if the currently executed job is CUNCURRENTthe mutated job *may*
     *     start execution right after adding to the queue without waiting for
     *     its parent to complete. Though this behavior is *not* guaranteed,
     *     because addMutatedJob does not initiate processQueues(), because
     *     it may lead to a deadlock.
     */
    void addMutatedJobs(const QVector<KisStrokeJobData*> list);

    /**
     * Convenience override for addMutatedJobs()
     */
    void addMutatedJob(KisStrokeJobData *data);


    // you are not supposed to change these parameters
    // after the KisStroke object has been created

    void setExclusive(bool value);
    void setSupportsWrapAroundMode(bool value);
    void setClearsRedoOnStart(bool value);
    void setRequestsOtherStrokesToEnd(bool value);
    void setCanForgetAboutMe(bool value);
    void setNeedsExplicitCancel(bool value);

    /**
     * Set override for the desired scheduler balancing ratio:
     *
     * ratio = stroke jobs / update jobs
     *
     * If \p value < 1.0, then the priority is given to updates, if
     * the value is higher than 1.0, then the priority is given
     * to stroke jobs.
     *
     * Special value -1.0, suggests the scheduler to use the default value
     * set by the user's config file (which is 100.0 by default).
     */
    void setBalancingRatioOverride(qreal value);

protected:
    /**
     * Protected c-tor, used for cloning of hi-level strategies
     */
    KisStrokeStrategy(const KisStrokeStrategy &rhs);

private:
    bool m_exclusive;
    bool m_supportsWrapAroundMode;

    bool m_clearsRedoOnStart;
    bool m_requestsOtherStrokesToEnd;
    bool m_canForgetAboutMe;
    bool m_needsExplicitCancel;
    qreal m_balancingRatioOverride;

    QLatin1String m_id;
    KUndo2MagicString m_name;

    KisStrokeId m_strokeId;
    KisStrokesQueueMutatedJobInterface *m_mutatedJobsInterface;
};

#endif /* __KIS_STROKE_STRATEGY_H */
