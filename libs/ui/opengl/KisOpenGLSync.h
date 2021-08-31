/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISOPENGLSYNC_H
#define KISOPENGLSYNC_H

#ifndef Q_OS_MACOS
#include <QOpenGLFunctions>
#else
#include <QOpenGLFunctions_3_2_Core>
#endif

#include <opengl/kis_opengl.h>

class KisOpenGLSync
{
public:
    KisOpenGLSync();
    ~KisOpenGLSync();

    bool isSignaled();

    static void init(QOpenGLContext *ctx);

private:
    GLsync m_syncObject = 0;
};

#endif // KISOPENGLSYNC_H
