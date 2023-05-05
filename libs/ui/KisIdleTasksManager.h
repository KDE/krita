/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISIDLETASKSMANAGER_H
#define KISIDLETASKSMANAGER_H

#include "kritaui_export.h"
#include <KisIdleTaskStrokeStrategy.h>

class KRITAUI_EXPORT KisIdleTasksManager : public QObject
{
    Q_OBJECT

public:
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

        bool isValid() const {
            return manager;
        }

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

    int addIdleTask(KisIdleTaskStrokeStrategyFactory factory);
    void removeIdleTask(int id);
    void triggerIdleTask(int id);

    TaskGuard addIdleTaskWithGuard(KisIdleTaskStrokeStrategyFactory factory);


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
