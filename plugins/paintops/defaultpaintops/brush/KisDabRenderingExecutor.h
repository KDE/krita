/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISDABRENDERINGEXECUTOR_H
#define KISDABRENDERINGEXECUTOR_H

#include "kritadefaultpaintops_export.h"

#include <QScopedPointer>

#include <QList>
struct KisRenderedDab;

#include "KisDabCacheUtils.h"

class KisPressureMirrorOption;
class KisPrecisionOption;
class KisRunnableStrokeJobsInterface;


class KRITADEFAULTPAINTOPS_EXPORT KisDabRenderingExecutor
{
public:
    KisDabRenderingExecutor(const KoColorSpace *cs,
                            KisDabCacheUtils::ResourcesFactory resourcesFactory,
                            KisRunnableStrokeJobsInterface *runnableJobsInterface,
                            KisPressureMirrorOption *mirrorOption = 0,
                            KisPrecisionOption *precisionOption = 0);
    ~KisDabRenderingExecutor();

    void addDab(const KisDabCacheUtils::DabRequestInfo &request,
                qreal opacity, qreal flow);

    QList<KisRenderedDab> takeReadyDabs(bool returnMutableDabs = false, int oneTimeLimit = -1, bool *someDabsLeft = 0);

    bool hasPreparedDabs() const;

    qreal averageDabRenderingTime() const; // msecs
    int averageDabSize() const;

private:
    KisDabRenderingExecutor(const KisDabRenderingExecutor &rhs) = delete;

    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISDABRENDERINGEXECUTOR_H
