/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisIdleTasksManager.h"

#include <QQueue>
#include <kis_idle_watcher.h>
#include <kis_image.h>
#include <KisMpl.h>
#include <boost/none.hpp>


namespace {
struct TaskStruct {
    int id = 0;
    KisIdleTaskStrokeStrategyFactory factory;
};
}

struct KisIdleTasksManager::Private
{
    KisImageWSP image;
    KisIdleWatcher idleWatcher;
    QVector<TaskStruct> tasks;
    QQueue<int> queue;
    QWeakPointer<boost::none_t> currentTaskCookie;
};

KisIdleTasksManager::KisIdleTasksManager()
    : m_d(new Private())
{
    connect(&m_d->idleWatcher, SIGNAL(startedIdleMode()), SLOT(slotImageIsIdle()));
    connect(&m_d->idleWatcher, SIGNAL(imageModified()), SLOT(slotImageIsModified()));
}

KisIdleTasksManager::~KisIdleTasksManager() = default;

void KisIdleTasksManager::setImage(KisImageSP image)
{
    m_d->idleWatcher.setTrackedImage(image);
    m_d->image = image;
    m_d->queue.clear();

    if (image) {
        slotImageIsModified();
        m_d->idleWatcher.triggerCountdownNoDelay();
    }
}

int KisIdleTasksManager::addIdleTask(KisIdleTaskStrokeStrategyFactory factory)
{
    /**
     * TODO: don't restart the whole queue on the the task change, just
     * restart the currently added task
     */

    const int newId =
        !m_d->tasks.isEmpty() ?
        m_d->tasks.last().id + 1 : 0;

    m_d->tasks.append({newId, factory});
    triggerIdleTask(newId);

    return newId;
}

void KisIdleTasksManager::removeIdleTask(int id)
{
    {
        auto it = std::remove_if(m_d->tasks.begin(), m_d->tasks.end(),
                                 kismpl::mem_equal_to(&TaskStruct::id, id));
        KIS_SAFE_ASSERT_RECOVER_NOOP(it != m_d->tasks.end());
        m_d->tasks.erase(it, m_d->tasks.end());
    }

    {
        auto it = std::remove(m_d->queue.begin(), m_d->queue.end(), id);
        m_d->queue.erase(it, m_d->queue.end());
    }
}

void KisIdleTasksManager::triggerIdleTask(int id)
{
    {
        // just verify that this tasks actually exists
        auto it = std::find_if(m_d->tasks.begin(), m_d->tasks.end(),
                               kismpl::mem_equal_to(&TaskStruct::id, id));
        KIS_SAFE_ASSERT_RECOVER_NOOP(it != m_d->tasks.end());
    }

    auto it = std::find(m_d->queue.begin(), m_d->queue.end(), id);
    if (it == m_d->queue.end()) {
        m_d->queue.enqueue(id);
    }

    m_d->idleWatcher.triggerCountdownNoDelay();
}

KisIdleTasksManager::TaskGuard
KisIdleTasksManager::addIdleTaskWithGuard(KisIdleTaskStrokeStrategyFactory factory)
{
    return {addIdleTask(factory), this};
}

void KisIdleTasksManager::slotImageIsModified()
{
    m_d->queue.clear();
    m_d->queue.reserve(m_d->tasks.size());
    std::transform(m_d->tasks.begin(), m_d->tasks.end(),
                   std::back_inserter(m_d->queue),
                   std::mem_fn(&TaskStruct::id));
}

void KisIdleTasksManager::slotImageIsIdle()
{
    KisImageSP image = m_d->image;
    if (!image) return;

    if (m_d->currentTaskCookie) {
        m_d->idleWatcher.restartCountdown();
        return;
    }

    if (m_d->queue.isEmpty()) return;

    const int newTaskId = m_d->queue.dequeue();

    auto it = std::find_if(m_d->tasks.begin(), m_d->tasks.end(),
                           kismpl::mem_equal_to(&TaskStruct::id, newTaskId));
    KIS_SAFE_ASSERT_RECOVER_NOOP(it != m_d->tasks.end());

    KisIdleTaskStrokeStrategy *strategy = it->factory(image);

    connect(strategy, SIGNAL(sigIdleTaskFinished()), SLOT(slotTaskIsCompleted()));
    m_d->currentTaskCookie = strategy->idleTaskCookie();

    KisStrokeId strokeId = image->startStroke(strategy);
    image->endStroke(strokeId);
}

void KisIdleTasksManager::slotTaskIsCompleted()
{
    if (m_d->queue.isEmpty()) {
        // all tasks are completed!
    } else {
        if (m_d->idleWatcher.isIdle()) {
            slotImageIsIdle();
        } else if (!m_d->idleWatcher.isCounting()) {
            m_d->idleWatcher.restartCountdown();
        }
    }
}

