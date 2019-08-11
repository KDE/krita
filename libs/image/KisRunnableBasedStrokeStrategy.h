/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISRUNNABLEBASEDSTROKESTRATEGY_H
#define KISRUNNABLEBASEDSTROKESTRATEGY_H

#include "kis_simple_stroke_strategy.h"

class KisRunnableStrokeJobsInterface;

class KRITAIMAGE_EXPORT KisRunnableBasedStrokeStrategy : public KisSimpleStrokeStrategy
{
private:
    struct JobsInterface;

public:
    KisRunnableBasedStrokeStrategy(QString id, const KUndo2MagicString &name = KUndo2MagicString());
    KisRunnableBasedStrokeStrategy(const KisRunnableBasedStrokeStrategy &rhs);
    ~KisRunnableBasedStrokeStrategy();

    void doStrokeCallback(KisStrokeJobData *data) override;

    KisRunnableStrokeJobsInterface *runnableJobsInterface() const;

private:
    const QScopedPointer<KisRunnableStrokeJobsInterface> m_jobsInterface;
};

#endif // KISRUNNABLEBASEDSTROKESTRATEGY_H
