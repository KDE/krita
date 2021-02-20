/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_STROKE_JOB_STRATEGY_H
#define __KIS_STROKE_JOB_STRATEGY_H

#include "kritaimage_export.h"
#include <QLatin1String>


class KRITAIMAGE_EXPORT KisStrokeJobData
{
public:
    enum Sequentiality {
        CONCURRENT,
        SEQUENTIAL,
        BARRIER,
        UNIQUELY_CONCURRENT
    };

    enum Exclusivity {
        NORMAL,
        EXCLUSIVE
    };

public:
    KisStrokeJobData(Sequentiality sequentiality = SEQUENTIAL,
                     Exclusivity exclusivity = NORMAL);
    virtual ~KisStrokeJobData();

    bool isBarrier() const;
    bool isSequential() const;
    bool isExclusive() const;

    Sequentiality sequentiality() { return m_sequentiality; }
    Exclusivity exclusivity() { return m_exclusivity; }

    virtual KisStrokeJobData* createLodClone(int levelOfDetail);

    bool isCancellable() const;
    void setCancellable(bool value);

    int levelOfDetailOverride() const;
    void setLevelOfDetailOverride(int value);

protected:
    KisStrokeJobData(const KisStrokeJobData &rhs);

private:
    Sequentiality m_sequentiality;
    Exclusivity m_exclusivity;
    bool m_isCancellable;
    int m_levelOfDetailOverride;
};


class KRITAIMAGE_EXPORT KisStrokeJobStrategy
{
public:
    KisStrokeJobStrategy();
    virtual ~KisStrokeJobStrategy();

    virtual void run(KisStrokeJobData *data) = 0;
    virtual QString debugId() const = 0;


private:
};

#endif /* __KIS_STROKE_JOB_STRATEGY_H */
