/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISWIDGETWITHIDLETASK_H
#define KISWIDGETWITHIDLETASK_H

#include <QWidget>
#include "KisIdleTasksManager.h"

class KisCanvas2;
class KoColorSpace;

template <typename BaseWidget>
class KisWidgetWithIdleTask : public BaseWidget
{
public:
    KisWidgetWithIdleTask(QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags())
        : BaseWidget (parent, flags)
    {
    }

    ~KisWidgetWithIdleTask() override = default;

    virtual void setCanvas(KisCanvas2 *canvas) {
        if (m_canvas) {
            m_idleTaskGuard = KisIdleTasksManager::TaskGuard();
        }

        m_canvas = canvas;

        if (m_canvas) {
            if (this->isVisible()) {
                m_idleTaskGuard = registerIdleTask(m_canvas);
            }
        }

        clearCachedState();
        this->update();
    }

    void showEvent(QShowEvent *event) override {
        BaseWidget::showEvent(event);

        // see a comment at the declaration of `m_isVisibleState`
        if (!m_isVisibleState) {
            m_isVisibleState = true;

            KIS_SAFE_ASSERT_RECOVER(!m_idleTaskGuard.isValid()) {
                m_idleTaskGuard = KisIdleTasksManager::TaskGuard();
            }

            if (m_canvas) {
                m_idleTaskGuard = registerIdleTask(m_canvas);
            }
            if (m_idleTaskGuard.isValid()) {
                m_idleTaskGuard.trigger();
            }
        }
    }

    void hideEvent(QHideEvent *event) override {
        BaseWidget::hideEvent(event);

        // see a comment at the declaration of `m_isVisibleState`
        if (m_isVisibleState) {
            m_isVisibleState = false;

            KIS_SAFE_ASSERT_RECOVER_NOOP(!m_canvas || m_idleTaskGuard.isValid());
            m_idleTaskGuard = KisIdleTasksManager::TaskGuard();

            clearCachedState();
        }
    }

    void triggerCacheUpdate() {
        if (m_idleTaskGuard.isValid()) {
            m_idleTaskGuard.trigger();
        }
    }

    [[nodiscard]]
    virtual KisIdleTasksManager::TaskGuard registerIdleTask(KisCanvas2 *canvas) = 0;
    virtual void clearCachedState() = 0;

protected:
    KisCanvas2 *m_canvas {0};
    KisIdleTasksManager::TaskGuard m_idleTaskGuard;

    /**
     * Hide/show events may be unbalanced so we track their parity with
     * an internal state. Also, we cannot rely on this->isVisible(),
     * because its state is different on different platforms during the
     * event delivery.
     */
    bool m_isVisibleState {false};
};

#endif // KISWIDGETWITHIDLETASK_H
