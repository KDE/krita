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

#include "opengl/kis_opengl_hdr_exposure_program.h"


#include <cmath>

#include "opengl/kis_opengl.h"
#include "opengl/kis_opengl_fragment_shader.h"

KisOpenGLHDRExposureProgram::KisOpenGLHDRExposureProgram()
{
    m_exposure = 0;
    createProgram();
}

void KisOpenGLHDRExposureProgram::setExposure(float exposure)
{
    m_exposure = exposure;

    if (active()) {
        setExposureUniformVariable();
    }
}

void KisOpenGLHDRExposureProgram::setExposureUniformVariable()
{
    Q_ASSERT(active());

    float exposure = pow(2, m_exposure + 2.47393);

    setUniformVariable("exposure", exposure, exposure, exposure, 1.0);
}

void KisOpenGLHDRExposureProgram::activate()
{
    KisOpenGLProgram::activate();
    setExposureUniformVariable();
    setUniformVariable("image", ImageTextureUnit);
}

void KisOpenGLHDRExposureProgram::createProgram()
{
    KisOpenGLFragmentShader *shader = KisOpenGLFragmentShader::createFromSourceCodeFile("hdr_exposure.frag");
    Q_CHECK_PTR(shader);
    if (shader && shader->isValid()) {
        attachShader(*shader);
        link();
    }
    delete shader;
}

