/*
 *  Copyright (c) 2007 Adrian Page <adrian@pagenet.plus.com>
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
#ifndef KIS_OPENGL_HDR_EXPOSURE_PROGRAM_H_
#define KIS_OPENGL_HDR_EXPOSURE_PROGRAM_H_

#include <GL/glew.h>

#include "krita_export.h"
#include "kis_opengl_program.h"

/**
 * An OpenGL program for displaying high dynamic range images using an exposure control.
 */
class KRITAUI_EXPORT KisOpenGLHDRExposureProgram : public KisOpenGLProgram {
    typedef KisOpenGLProgram super;

public:
    KisOpenGLHDRExposureProgram();

    /**
     * Set the exposure. A typical range of values is -10 to 10.
     * @param exposure the exposure value
     */
    void setExposure(float exposure);

    /**
     * Activate the program ready for rendering.
     */
    virtual void activate();

private:
    virtual void createProgram();
    void setExposureUniformVariable();

    static const GLint ImageTextureUnit = 0;
    static const GLint ImageTextureUnitEnum = GL_TEXTURE0 + ImageTextureUnit;

    GLfloat m_exposure;
};

#endif // KIS_OPENGL_HDR_EXPOSURE_PROGRAM_H_

