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

#include "krita_export.h"


class KRITAIMAGE_EXPORT KisStrokeJobStrategy
{
public:
    class StrokeJobData
    {
    public:
        virtual ~StrokeJobData();
    };

public:
    KisStrokeJobStrategy(bool isSequential = true, bool isExclusive = false);
    virtual ~KisStrokeJobStrategy();

    virtual void run(StrokeJobData *data) = 0;
    bool isSequential() const;
    bool isExclusive() const;

private:
    bool m_isSequential;
    bool m_isExclusive;
};

#endif /* __KIS_STROKE_JOB_STRATEGY_H */
