/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
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
#ifndef KIS_OPENGL_IMAGE_CONTEXT_H_
#define KIS_OPENGL_IMAGE_CONTEXT_H_

#include <map>

#include <qobject.h>

#include <qvaluevector.h>

#include <koffice_export.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kis_types.h"
#ifdef HAVE_GL
#include <qgl.h>
#endif

class QRegion;

class KisOpenGLImageContext;
typedef KSharedPtr<KisOpenGLImageContext> KisOpenGLImageContextSP;
class KisColorSpace;

#ifndef HAVE_GL
typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void GLvoid;
#endif

class KRITACORE_EXPORT KisOpenGLImageContext : public QObject , public KShared {

    Q_OBJECT

public:
    static KisOpenGLImageContextSP getImageContext(KisImageSP image, KisProfile *monitorProfile);

    KisOpenGLImageContext();
    virtual ~KisOpenGLImageContext();

public:
    // In order to use the image textures, the caller must pass
    // the sharedContextWidget() as the shareWidget argument to the 
    // QGLWidget constructor.
    QGLWidget *sharedContextWidget() const;

    void setMonitorProfile(KisProfile *profile);
    void setHDRExposure(float exposure);

    GLuint backgroundTexture() const;

    static const int BACKGROUND_TEXTURE_WIDTH = 32;
    static const int BACKGROUND_TEXTURE_HEIGHT = 32;

    // Get the image texture tile containing the point (pixelX, pixelY).
    GLuint imageTextureTile(int pixelX, int pixelY) const;

    int imageTextureTileWidth() const;
    int imageTextureTileHeight() const;

signals:
    /**
     * Clients using the KisOpenGLImageContext should connect to the
     * following signals rather than to the KisImage's own equivalent
     * signals. This ensures that the image textures are always up to date
     * when used.
     */

    /**
     * Emitted whenever an action has caused the image to be recomposited.
     *
     * @param image  The image 
     * @param rc     The rect that has been recomposited.
     */
    void sigImageUpdated(KisImageSP image, const QRect& rc);

    /**
     * Emitted whenever the image size changes.
     *
     * @param image  The image 
     * @param width  New image width 
     * @param height New image height
     */
    void sigSizeChanged(KisImageSP image, Q_INT32 width, Q_INT32 height);

protected:
    KisOpenGLImageContext(KisImageSP image, KisProfile *monitorProfile);

    void generateBackgroundTexture();
    void createImageTextureTiles();
    void destroyImageTextureTiles();
    int imageTextureTileIndex(int x, int y) const;
    void updateImageTextureTiles(const QRect& rect);

    static KisColorSpace* textureColorSpaceForImageColorSpace(KisColorSpace *imageColorSpace);
    static bool imageCanShareImageContext(KisImageSP image);

protected slots:
    void slotImageUpdated(KisImageSP image, const QRect& r);
    void slotImageSizeChanged(KisImageSP image, Q_INT32 w, Q_INT32 h);

private:
    KisImageSP m_image;
    KisProfile *m_monitorProfile;
    float m_exposure;

    GLuint m_backgroundTexture;

    static const int PREFERRED_IMAGE_TEXTURE_WIDTH = 256;
    static const int PREFERRED_IMAGE_TEXTURE_HEIGHT = 256;

    QValueVector<GLuint> m_imageTextureTiles;
    int m_imageTextureTileWidth;
    int m_imageTextureTileHeight;
    int m_numImageTextureTileColumns;

    // We create a single OpenGL context and share it between all views
    // in the process. Apparently with some OpenGL implementations, only
    // one context will be hardware accelerated.
    static QGLWidget *SharedContextWidget;
    static int SharedContextWidgetRefCount;

    typedef std::map<KisImageSP, KisOpenGLImageContext*> ImageContextMap;

    static ImageContextMap imageContextMap;
};

#endif // KIS_OPENGL_IMAGE_CONTEXT_H_

