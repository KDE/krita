/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    KisRunnableBasedStrokeStrategy(const QLatin1String &id, const KUndo2MagicString &name = KUndo2MagicString());
    KisRunnableBasedStrokeStrategy(const KisRunnableBasedStrokeStrategy &rhs);
    ~KisRunnableBasedStrokeStrategy();

    void doStrokeCallback(KisStrokeJobData *data) override;

    KisRunnableStrokeJobsInterface *runnableJobsInterface() const;

private:
    const QScopedPointer<KisRunnableStrokeJobsInterface> m_jobsInterface;
};

#endif // KISRUNNABLEBASEDSTROKESTRATEGY_H
