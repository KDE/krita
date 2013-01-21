/*
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
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

#ifndef KIS_OPENGL_CANVAS_2_P_H
#define KIS_OPENGL_CANVAS_2_P_H

#include <opengl/kis_opengl.h>

#ifdef HAVE_OPENGL

/**
 * This is a workaround for a very slow updates in OpenGL canvas (~6ms).
 * The delay happens because of VSync in the swapBuffers() call. At first
 * we try to disable VSync. If it fails we just disable double buffer.
 */


#if defined Q_OS_LINUX

#include <QX11Info>
#include <GL/glxew.h>

extern const QString qt_gl_library_name();

namespace VSyncWorkaround {

    bool tryDisableVSync(QWidget *widget) {
        bool result = false;
        QX11Info info = widget->x11Info();
        Display *dpy = info.display();
        WId wid = widget->winId();

#ifdef GLX_EXT_swap_control

        if (glewIsSupported("GLX_EXT_swap_control")) {
            glXSwapIntervalEXT(dpy, wid, 0);

            unsigned int swap = 0;
            glXQueryDrawable(dpy, wid, GLX_SWAP_INTERVAL_EXT, &swap);

            result = !swap;
        }
#else
        if (0) {}
#endif /* GLX_EXT_swap_control */

#ifdef GLX_MESA_swap_control
        else if (glewIsSupported("GLX_EXT_swap_control")) {
                int retval = glXSwapIntervalMESA(0);
                int swap = glXGetSwapIntervalMESA();

                result = !retval && !swap;
        }
#endif /* GLX_MESA_swap_control */

        else {
            qDebug() << "There is neither GLX_EXT_swap_control or GLX_MESA_swap_control extension supported";
        }

        return result;
    }
}

#else  // defined Q_OS_LINUX

namespace VSyncWorkaround {
    bool tryDisableVSync(QWidget *) {
        return false;
    }
}

#endif // defined Q_OS_LINUX

#endif // HAVE_OPENGL
#endif // KIS_OPENGL_CANVAS_2_P_H
