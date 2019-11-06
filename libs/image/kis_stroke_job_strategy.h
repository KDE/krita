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

#ifndef __KIS_STROKE_JOB_STRATEGY_H
#define __KIS_STROKE_JOB_STRATEGY_H

#include "kritaimage_export.h"


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

protected:
    KisStrokeJobData(const KisStrokeJobData &rhs);

private:
    Sequentiality m_sequentiality;
    Exclusivity m_exclusivity;
    bool m_isCancellable;
};


class KRITAIMAGE_EXPORT KisStrokeJobStrategy
{
public:
    KisStrokeJobStrategy();
    virtual ~KisStrokeJobStrategy();

    virtual void run(KisStrokeJobData *data) = 0;


private:
};

#endif /* __KIS_STROKE_JOB_STRATEGY_H */
