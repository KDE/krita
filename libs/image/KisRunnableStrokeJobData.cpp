/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisRunnableStrokeJobData.h"

#include <QRunnable>
#include <kis_assert.h>

KisRunnableStrokeJobData::KisRunnableStrokeJobData(QRunnable *runnable, KisStrokeJobData::Sequentiality sequentiality, KisStrokeJobData::Exclusivity exclusivity)
    : KisRunnableStrokeJobDataBase(sequentiality, exclusivity),
      m_runnable(runnable)
{
}

KisRunnableStrokeJobData::KisRunnableStrokeJobData(std::function<void ()> func, KisStrokeJobData::Sequentiality sequentiality, KisStrokeJobData::Exclusivity exclusivity)
    : KisRunnableStrokeJobDataBase(sequentiality, exclusivity),
      m_func(func)
{
}

KisRunnableStrokeJobData::~KisRunnableStrokeJobData() {
    if (m_runnable && m_runnable->autoDelete()) {
        delete m_runnable;
    }
}

void KisRunnableStrokeJobData::run() {
    if (m_runnable) {
        m_runnable->run();
    } else if (m_func) {
        m_func();
    }
}
