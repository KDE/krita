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
