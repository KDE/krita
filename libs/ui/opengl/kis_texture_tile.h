/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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
#ifndef KIS_TEXTURE_TILE_H_
#define KIS_TEXTURE_TILE_H_

#include <QRect>
#include <QRectF>
// no forward-declaration, used to get GL* primitive types defined
#include <QOpenGLFunctions>

#include "kis_opengl.h"

#if !defined(QT_OPENGL_ES)
#define USE_PIXEL_BUFFERS
#endif

class KisTextureTileUpdateInfo;
class QOpenGLBuffer;


struct KisGLTexturesInfo {

    KisGLTexturesInfo()
        : width(0)
        , height(0)
        , effectiveWidth(1)
        , effectiveHeight(1)
        , border(0)
    {}

    // real width and height
    int width;
    int height;

    // width and height minus border padding?
    int effectiveWidth;
    int effectiveHeight;

    // size of the border padding
    int border;

    GLint internalFormat;
    GLint format;
    GLint type;
};

class KisTextureTile
{
public:
    KisTextureTile(const QRect &imageRect, const KisGLTexturesInfo *texturesInfo,
                   const QByteArray &fillData, KisOpenGL::FilterMode mode,
                   bool useBuffer, int numMipmapLevels, QOpenGLFunctions *f);
    ~KisTextureTile();

    void setUseBuffer(bool useBuffer) {
        m_useBuffer = useBuffer;
    }

    void setNumMipmapLevels(int num) {
        m_numMipmapLevels = num;
    }

    void update(const KisTextureTileUpdateInfo &updateInfo);

    inline QRect tileRectInImagePixels() {
        return m_tileRectInImagePixels;
    }

    inline QRect textureRectInImagePixels() {
        return m_textureRectInImagePixels;
    }

    inline QRectF tileRectInTexturePixels() {
        return m_tileRectInTexturePixels;
    }

    QRectF imageRectInTexturePixels(const QRect &imageRect) const;

    void bindToActiveTexture();
    int currentLodPlane() const;

private:
    inline void setTextureParameters();

    void setNeedsMipmapRegeneration();
    void setCurrentLodPlane(int lod);

    GLuint m_textureId;

#ifdef USE_PIXEL_BUFFERS
    void createTextureBuffer(const char*data, int size);
    QOpenGLBuffer *m_glBuffer;
#endif

    QRect m_tileRectInImagePixels;
    QRectF m_tileRectInTexturePixels;
    QRect m_textureRectInImagePixels;
    KisOpenGL::FilterMode m_filter;
    const KisGLTexturesInfo *m_texturesInfo;
    bool m_needsMipmapRegeneration;
    int m_currentLodPlane;
    bool m_useBuffer;
    int m_numMipmapLevels;
    QOpenGLFunctions *f;
    Q_DISABLE_COPY(KisTextureTile)
};

#endif /* KIS_TEXTURE_TILE_H_ */

