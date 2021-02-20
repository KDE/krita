/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SUSPEND_PROJECTION_UPDATES_STROKE_STRATEGY_H
#define __KIS_SUSPEND_PROJECTION_UPDATES_STROKE_STRATEGY_H

#include <KisRunnableBasedStrokeStrategy.h>

#include <QScopedPointer>

class KisSuspendProjectionUpdatesStrokeStrategy : public KisRunnableBasedStrokeStrategy
{
public:
    struct SharedData {
        KisProjectionUpdatesFilterCookie installedFilterCookie = {};
    };
    using SharedDataSP = QSharedPointer<SharedData>;

public:
    KisSuspendProjectionUpdatesStrokeStrategy(KisImageWSP image, bool suspend, SharedDataSP sharedData);
    ~KisSuspendProjectionUpdatesStrokeStrategy() override;

    static QList<KisStrokeJobData*> createSuspendJobsData(KisImageWSP image);
    static QList<KisStrokeJobData*> createResumeJobsData(KisImageWSP image);
    static SharedDataSP createSharedData();
private:
    void initStrokeCallback() override;
    void doStrokeCallback(KisStrokeJobData *data) override;
    void cancelStrokeCallback() override;

    void suspendStrokeCallback() override;
    void resumeStrokeCallback() override;


    void resumeAndIssueUpdates(bool dropUpdates);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_SUSPEND_PROJECTION_UPDATES_STROKE_STRATEGY_H */
