/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisOpenGLContextSwitchLock.h"

#include <QOpenGLContext>
#include <QOpenGLWidget>


namespace {

inline bool shouldUseLock()
{
#if defined(Q_OS_MACOS) || defined(Q_OS_ANDROID)
    /**
     * On OSX openGL different (shared) contexts have different execution queues.
     * It means that the textures uploading and their painting can be easily reordered.
     * To overcome the issue, we should ensure that the textures are uploaded in the
     * same openGL context as the painting is done.
     */
    return true;
#else
    static const bool s_shouldUseLock = qEnvironmentVariableIsSet("KRITA_USE_STRICT_OPENGL_CONTEXT_SWITCH");
    return s_shouldUseLock;
#endif
}

}

KisOpenGLContextSwitchLockAdapter::KisOpenGLContextSwitchLockAdapter(QOpenGLWidget *targetWidget)
    : m_targetWidget(targetWidget)
{
}

void KisOpenGLContextSwitchLockAdapter::lock() {
    if (!shouldUseLock()) return;
    m_oldContext = QOpenGLContext::currentContext();
    m_oldSurface = m_oldContext ? m_oldContext->surface() : nullptr;
    m_targetWidget->makeCurrent();
}

void KisOpenGLContextSwitchLockAdapter::unlock() {
    if (!shouldUseLock()) return;
    if (m_oldContext) {
        m_oldContext->makeCurrent(m_oldSurface);
    } else {
        m_targetWidget->doneCurrent();
    }
}

