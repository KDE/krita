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
#include "kis_stroke_job_strategy.h"
#include "kis_types.h"

class KisStrokeStrategy;


class KRITAIMAGE_EXPORT KisStrokeStrategy
{
public:
    KisStrokeStrategy(QString id = QString(), QString name = QString());
    virtual ~KisStrokeStrategy();

    virtual KisStrokeJobStrategy* createInitStrategy();
    virtual KisStrokeJobStrategy* createFinishStrategy();
    virtual KisStrokeJobStrategy* createCancelStrategy();
    virtual KisStrokeJobStrategy* createDabStrategy();

    virtual KisStrokeJobData* createInitData();
    virtual KisStrokeJobData* createFinishData();
    virtual KisStrokeJobData* createCancelData();

    bool isExclusive() const;
    bool needsIndirectPainting() const;
    QString indirectPaintingCompositeOp() const;

    QString id() const;
    QString name() const;

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
    void setNeedsIndirectPainting(bool value);
    void setIndirectPaintingCompositeOp(const QString &id);

private:
    bool m_exclusive;
    bool m_needsIndirectPainting;
    QString m_indirectPaintingCompositeOp;

    QString m_id;
    QString m_name;

    KisStrokeId m_cancelStrokeId;
};

#endif /* __KIS_STROKE_STRATEGY_H */
