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
#ifndef KIS_OPENGL_GRADIENT_PROGRAM_H
#define KIS_OPENGL_GRADIENT_PROGRAM_H

#include <GL/glew.h>

#include <QPoint>

#include "krita_export.h"

#include "kis_opengl_program.h"
#include "kis_gradient_painter.h"

class KisGradient;
class KisOpenGLGradientShader;
class KoColorProfile;
class KoColorSpace;

class KRITAUI_EXPORT KisOpenGLGradientProgram : public KisOpenGLProgram
{
public:
    KisOpenGLGradientProgram(const KoSegmentGradient *gradient,
                             KisGradientPainter::enumGradientShape shape,
                             KisGradientPainter::enumGradientRepeat repeat,
                             bool reverseGradient,
                             KoColorSpace *colorSpace,
                             KoColorProfile *monitorProfile,
                             double opacity);
    ~KisOpenGLGradientProgram();

    void activate(const QPointF &gradientVectorStart, const QPointF &gradientVectorEnd);

private:
    void createGradientColorsTexture(const KisGradient *gradient,
                                     KisGradientPainter::enumGradientRepeat repeat,
                                     bool reverseGradient,
                                     KoColorSpace *colorSpace,
                                     KoColorProfile *monitorProfile,
                                     double opacity);
    GLuint m_gradientColorsTexture;
    KisOpenGLGradientShader *m_gradientShader;
};

#endif // KIS_OPENGL_GRADIENT_PROGRAM_H

