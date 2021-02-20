/*
 * SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_OPENGL_CANVAS_2_P_H
#define KIS_OPENGL_CANVAS_2_P_H

#include <opengl/kis_opengl.h>

/**
 * This is a workaround for a very slow updates in OpenGL canvas (~6ms).
 * The delay happens because of VSync in the swapBuffers() call. At first
 * we try to disable VSync. If it fails we just disable Double Buffer
 * completely.
 *
 * This file is effectively a bit of copy-paste from qgl_x11.cpp
 */

namespace Sync {
    //For checking sync status
    enum SyncStatus {
        Signaled,
        Unsignaled
    };

#ifndef GL_SYNC_GPU_COMMANDS_COMPLETE
    #define GL_SYNC_GPU_COMMANDS_COMPLETE 0x9117
#endif
#ifndef GL_UNSIGNALED
    #define GL_UNSIGNALED 0x9118
#endif
#ifndef GL_SIGNALED
    #define GL_SIGNALED 0x9119
#endif
#ifndef GL_SYNC_STATUS
    #define GL_SYNC_STATUS 0x9114
#endif

    //Function pointers for glFenceSync and glGetSynciv
    typedef GLsync (*kis_glFenceSync)(GLenum, GLbitfield);
    static kis_glFenceSync k_glFenceSync = 0;
    typedef void (*kis_glGetSynciv)(GLsync, GLenum, GLsizei, GLsizei*, GLint*);
    static kis_glGetSynciv k_glGetSynciv = 0;
    typedef void (*kis_glDeleteSync)(GLsync);
    static kis_glDeleteSync k_glDeleteSync = 0;
    typedef GLenum (*kis_glClientWaitSync)(GLsync,GLbitfield,GLuint64);
    static kis_glClientWaitSync k_glClientWaitSync = 0;

    //Initialise the function pointers for glFenceSync and glGetSynciv
    //Note: Assumes a current OpenGL context.
    void init(QOpenGLContext* ctx) {
#if defined Q_OS_WIN
        if (KisOpenGL::supportsFenceSync()) {
#ifdef ENV64BIT
            k_glFenceSync  = (kis_glFenceSync)ctx->getProcAddress("glFenceSync");
            k_glGetSynciv  = (kis_glGetSynciv)ctx->getProcAddress("glGetSynciv");
            k_glDeleteSync = (kis_glDeleteSync)ctx->getProcAddress("glDeleteSync");
#else
            k_glFenceSync  = (kis_glFenceSync)ctx->getProcAddress("glFenceSyncARB");
            k_glGetSynciv  = (kis_glGetSynciv)ctx->getProcAddress("glGetSyncivARB");
            k_glDeleteSync = (kis_glDeleteSync)ctx->getProcAddress("glDeleteSyncARB");
#endif
            k_glClientWaitSync = (kis_glClientWaitSync)ctx->getProcAddress("glClientWaitSync");
        }
#elif defined Q_OS_LINUX || defined Q_OS_MACOS
        if (KisOpenGL::supportsFenceSync()) {
            k_glFenceSync  = (kis_glFenceSync)ctx->getProcAddress("glFenceSync");
            k_glGetSynciv  = (kis_glGetSynciv)ctx->getProcAddress("glGetSynciv");
            k_glDeleteSync = (kis_glDeleteSync)ctx->getProcAddress("glDeleteSync");
            k_glClientWaitSync = (kis_glClientWaitSync)ctx->getProcAddress("glClientWaitSync");
        }
#endif
        if (k_glFenceSync  == 0 || k_glGetSynciv      == 0 ||
            k_glDeleteSync == 0 || k_glClientWaitSync == 0) {
            warnUI << "Could not find sync functions, disabling sync notification.";
        }
    }

    //Get a fence sync object from OpenGL
    GLsync getSync() {
        if(k_glFenceSync) {
            GLsync sync = k_glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
            if (KisOpenGL::needsFenceWorkaround()) {
                k_glClientWaitSync(sync, 0, 1);
            }
            return sync;
        }
        return 0;
    }

    //Check the status of a sync object
    SyncStatus syncStatus(GLsync syncObject) {
        if(syncObject && k_glGetSynciv) {
            GLint status = -1;
            k_glGetSynciv(syncObject, GL_SYNC_STATUS, 1, 0, &status);
            return status == GL_SIGNALED ? Sync::Signaled : Sync::Unsignaled;
        }
        return Sync::Signaled;
    }

    void deleteSync(GLsync syncObject) {
        if(syncObject && k_glDeleteSync) {
            k_glDeleteSync(syncObject);
        }
    }
}

#endif // KIS_OPENGL_CANVAS_2_P_H
