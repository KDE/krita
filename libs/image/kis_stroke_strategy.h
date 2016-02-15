/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_STROKE_STRATEGY_H
#define __KIS_STROKE_STRATEGY_H

#include <QString>
#include "kis_types.h"
#include "kundo2magicstring.h"
#include "kritaimage_export.h"

class KisStrokeJobStrategy;
class KisStrokeJobData;

class KRITAIMAGE_EXPORT KisStrokeStrategy
{
public:
    KisStrokeStrategy(QString id = QString(), const KUndo2MagicString &name = KUndo2MagicString());
    virtual ~KisStrokeStrategy();

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

    bool isExclusive() const;
    bool supportsWrapAroundMode() const;
    bool needsIndirectPainting() const;
    QString indirectPaintingCompositeOp() const;

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

    QString id() const;
    KUndo2MagicString name() const;

    /**
     * Set up by the strokes queue during the stroke initialization
     */
    void setCancelStrokeId(KisStrokeId id) { m_cancelStrokeId = id; }

protected:
    /**
     * The cancel job may populate the stroke with some new jobs
     * for cancelling. To achieve this it needs the stroke id.
     *
     * WARNING: you can't add new jobs in any places other than
     * cancel job, because the stroke may be ended in any moment
     * by the user and the sequence of jobs will be broken
     */
    KisStrokeId cancelStrokeId() { return m_cancelStrokeId; }

    // you are not supposed to change these parameters
    // after the KisStroke object has been created

    void setExclusive(bool value);
    void setSupportsWrapAroundMode(bool value);
    void setNeedsIndirectPainting(bool value);
    void setIndirectPaintingCompositeOp(const QString &id);
    void setClearsRedoOnStart(bool value);
    void setRequestsOtherStrokesToEnd(bool value);

protected:
    /**
     * Protected c-tor, used for cloning of hi-level strategies
     */
    KisStrokeStrategy(const KisStrokeStrategy &rhs);

private:
    bool m_exclusive;
    bool m_supportsWrapAroundMode;
    bool m_needsIndirectPainting;
    QString m_indirectPaintingCompositeOp;
    bool m_clearsRedoOnStart;
    bool m_requestsOtherStrokesToEnd;

    QString m_id;
    KUndo2MagicString m_name;

    KisStrokeId m_cancelStrokeId;
};

#endif /* __KIS_STROKE_STRATEGY_H */
