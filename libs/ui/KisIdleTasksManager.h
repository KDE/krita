/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISIDLETASKSMANAGER_H
#define KISIDLETASKSMANAGER_H

#include "kritaui_export.h"
#include <KisIdleTaskStrokeStrategy.h>
#include <QPointer>


/**
 * A special per-main-window manager that handles  jobs that
 * should be run in the background while the user is idle. The
 * manager automatically restarts all the jobs when the image is
 * modified. E.g. updates the overview image or histogram.
 *
 * The manager is owned by KisViewManager, which automatically
 * connects it to the currently active image. When the image
 * becomes idle, the manager starts its tasks one-by-one.
 *
 * If you want to add a new task to the manager, just provide
 * a factory to addIdleTaskWithGuard() method. This factory
 * should create and return a stroke strategy for the idle
 * task to be run.
 *
 * The factory will be called by the task manager  every time
 * when it thinks that the idle task should be started.
 *
 *
 * addIdleTaskWithGuard() returns a TaskGuard handle, which
 * represents the registered task. It is a movable object
 * that will automatically de-register the idle task on
 * destruction.
 *
 * If your idle-task-factory is a lambda object, make sure
 * that the lifetime of the objects you capture into the
 * lambda's closure is longer than the lifetime of the
 * corresponding TaskGuard handle. In other words, if you
 * capture `this` into your lambda, make sure that
 * the corresponding `TaskGuard` also belongs to `this` or
 * destroyed manually in the destructor.
 */
class KRITAUI_EXPORT KisIdleTasksManager : public QObject
{
    Q_OBJECT

public:
    /**
     * A simple **movable** handle that represents a task
     * registered in the manager. When the handle is destroyed,
     * the task it automatically de-registered.
     */
    struct TaskGuard {
        TaskGuard() {}
        TaskGuard(int _taskId,
                  QPointer<KisIdleTasksManager> _manager)
            : taskId(_taskId)
            , manager(_manager)
        {}

        TaskGuard(const TaskGuard &rhs) = delete;
        TaskGuard& operator=(const TaskGuard &rhs) = delete;

        TaskGuard(TaskGuard &&rhs) {
            std::swap(taskId, rhs.taskId);
            std::swap(manager, rhs.manager);
        };

        TaskGuard& operator=(TaskGuard &&rhs) {
            std::swap(taskId, rhs.taskId);
            std::swap(manager, rhs.manager);
            return *this;
        }

        ~TaskGuard() {
            if (manager) {
                manager->removeIdleTask(taskId);
            }
        }

        /**
         * @return true if the task is valid and registered at the manager
         */
        bool isValid() const {
            return manager;
        }

        /**
         * Explicitly request restart of the task, e.g. explicitly
         * restart recalculation of the overview image when the display
         * profile is changed
         */
        void trigger() {
            KIS_SAFE_ASSERT_RECOVER_RETURN(manager);
            manager->triggerIdleTask(taskId);
        }

        int taskId = -1;
        QPointer<KisIdleTasksManager> manager;
    };

public:
    KisIdleTasksManager();
    ~KisIdleTasksManager();

    void setImage(KisImageSP image);

    /**
     * @brief Registers the factory for the idle task
     *
     * The manager will use this factory to start the task after
     * every image modification.
     *
     * @param factory is a functor creating a KisIdleTaskStrokeStrategy
     *                that will actually execute the task
     * @return a TaskGuard object that can be used for task manipulations
     */
    [[nodiscard]]
    TaskGuard addIdleTaskWithGuard(KisIdleTaskStrokeStrategyFactory factory);

private:
    int addIdleTask(KisIdleTaskStrokeStrategyFactory factory);
    void removeIdleTask(int id);
    void triggerIdleTask(int id);

private Q_SLOTS:
    void slotImageIsModified();
    void slotImageIsIdle();
    void slotTaskIsCompleted();

private:

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KISIDLETASKSMANAGER_H
