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

#ifndef __KIS_SIMPLE_STROKE_STRATEGY_H
#define __KIS_SIMPLE_STROKE_STRATEGY_H

#include <QVector>
#include "kis_stroke_strategy.h"


class KRITAIMAGE_EXPORT KisSimpleStrokeStrategy : public KisStrokeStrategy
{
public:
    enum JobType {
        JOB_INIT = 0,
        JOB_CANCEL,
        JOB_FINISH,
        JOB_DOSTROKE
    };

public:
    KisSimpleStrokeStrategy(QString id = QString(), QString name = QString());

    KisStrokeJobStrategy* createInitStrategy();
    KisStrokeJobStrategy* createFinishStrategy();
    KisStrokeJobStrategy* createCancelStrategy();
    KisStrokeJobStrategy* createDabStrategy();

    KisStrokeJobData* createInitData();
    KisStrokeJobData* createFinishData();
    KisStrokeJobData* createCancelData();

    virtual void initStrokeCallback();
    virtual void finishStrokeCallback();
    virtual void cancelStrokeCallback();
    virtual void doStrokeCallback(KisStrokeJobData *data);

protected:
    void enableJob(JobType type, bool enable = true);

private:
    KisStrokeJobStrategy* createStrategy(JobType type);

private:
    QVector<bool> m_jobEnabled;
};

#endif /* __KIS_SIMPLE_STROKE_STRATEGY_H */
