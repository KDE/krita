/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisRunnableStrokeJobsInterface.h"

#include <QVector>

KisRunnableStrokeJobsInterface::~KisRunnableStrokeJobsInterface()
{

}

void KisRunnableStrokeJobsInterface::addRunnableJob(KisRunnableStrokeJobDataBase *data)
{
    addRunnableJobs({data});
}
