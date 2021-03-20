/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSTROKESQUEUEMUTATEDJOBINTERFACE_H
#define KISSTROKESQUEUEMUTATEDJOBINTERFACE_H

#include "kis_types.h"
#include "KisLodPreferences.h"

class KisStrokeJobData;

class KisStrokesQueueMutatedJobInterface
{
public:
    virtual ~KisStrokesQueueMutatedJobInterface();

    virtual void addMutatedJobs(KisStrokeId strokeId, const QVector<KisStrokeJobData*> list) = 0;
    virtual KisLodPreferences lodPreferences() const = 0;
};

#endif // KISSTROKESQUEUEMUTATEDJOBINTERFACE_H
