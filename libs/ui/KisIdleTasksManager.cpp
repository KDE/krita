/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisIdleTasksManager.h"

#include <kis_idle_watcher.h>
#include <kis_image.h>
#include <KisMpl.h>

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
    QWeakPointer<bool> currentTaskCookie;
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

    slotImageIsModified();
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
    slotImageIsModified();

    return newId;
}

void KisIdleTasksManager::removeIdleTask(int id)
{
    auto it = std::remove_if(m_d->tasks.begin(), m_d->tasks.end(),
                             kismpl::mem_equal_to(&TaskStruct::id, id));
    KIS_SAFE_ASSERT_RECOVER_NOOP(it != m_d->tasks.end());
    m_d->tasks.erase(it, m_d->tasks.end());
    m_d->queue.clear();
}

void KisIdleTasksManager::triggerIdleTask(int id)
{
    auto it = std::find_if(m_d->tasks.begin(), m_d->tasks.end(),
                           kismpl::mem_equal_to(&TaskStruct::id, id));
    KIS_SAFE_ASSERT_RECOVER_NOOP(it != m_d->tasks.end());

    const int index = std::distance(m_d->tasks.begin(), it);
    if (std::find(m_d->queue.begin(), m_d->queue.end(), index) == m_d->queue.end()) {
        m_d->queue.enqueue(index);
    }

    m_d->idleWatcher.restartCountdown();
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
    for (int i = 0; i < m_d->tasks.size(); i++) {
        m_d->queue.enqueue(i);
    }
}

void KisIdleTasksManager::slotImageIsIdle()
{
    KisImageSP image = m_d->image;
    if (!image) return;

    if (m_d->currentTaskCookie) {
        m_d->idleWatcher.restartCountdown();
        return;
    }

    const int newTaskIndex = m_d->queue.dequeue();

    KisIdleTaskStrokeStrategy *strategy = m_d->tasks[newTaskIndex].factory(image);
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

