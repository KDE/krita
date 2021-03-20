/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISRUNNABLESTROKEJOBSINTERFACE_H
#define KISRUNNABLESTROKEJOBSINTERFACE_H

#include "kritaimage_export.h"
#include <QtGlobal>
#include "kis_pointer_utils.h"

class KisRunnableStrokeJobDataBase;


class KRITAIMAGE_EXPORT KisRunnableStrokeJobsInterface
{
public:
    virtual ~KisRunnableStrokeJobsInterface();

    void addRunnableJob(KisRunnableStrokeJobDataBase *data);
    virtual void addRunnableJobs(const QVector<KisRunnableStrokeJobDataBase*> &list) = 0;

    template <typename T>
    void addRunnableJobs(const QVector<T*> &list) {
        this->addRunnableJobs(implicitCastList<KisRunnableStrokeJobDataBase*>(list));
    }
};

#endif // KISRUNNABLESTROKEJOBSINTERFACE_H
