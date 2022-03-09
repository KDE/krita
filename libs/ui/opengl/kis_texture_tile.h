/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_TEXTURE_TILE_H_
#define KIS_TEXTURE_TILE_H_

#include <QRect>
#include <QRectF>
// no forward-declaration, used to get GL* primitive types defined
#include <QOpenGLFunctions>

#include "kis_opengl.h"


class KisOpenGLBufferCircularStorage;
class KisTextureTileUpdateInfo;
class QOpenGLBuffer;


struct KisGLTexturesInfo {

    // real width and height
    int width {0};
    int height {0};

    // width and height minus border padding?
    int effectiveWidth {1};
    int effectiveHeight {1};

    // size of the border padding
    int border {0};

    GLint internalFormat {0};
    GLint format {0};
    GLint type {0};
};

class KisTextureTile
{
public:
    KisTextureTile(const QRect &imageRect, const KisGLTexturesInfo *texturesInfo,
                   const QByteArray &fillData, KisOpenGL::FilterMode mode,
                   KisOpenGLBufferCircularStorage *bufferStorage, int numMipmapLevels, QOpenGLFunctions *f);
    ~KisTextureTile();

    void setBufferStorage(KisOpenGLBufferCircularStorage *bufferStorage) {
        m_bufferStorage = bufferStorage;
    }

    void setNumMipmapLevels(int num) {
        m_numMipmapLevels = num;
    }

    void update(const KisTextureTileUpdateInfo &updateInfo, bool blockMipmapRegeneration);

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

    /**
     * Binds the tile's testure to the current GL_TEXTURE_2D binding point,
     * regenerates the mipmap if needed and returns the levelOfDetail that
     * should be used for painting
     */
    int bindToActiveTexture(bool blockMipmapRegeneration);

private:
    inline void setTextureParameters();

    void setNeedsMipmapRegeneration();
    void setPreparedLodPlane(int lod);

    GLuint m_textureId;

    QRect m_tileRectInImagePixels;
    QRectF m_tileRectInTexturePixels;
    QRect m_textureRectInImagePixels;
    KisOpenGL::FilterMode m_filter;
    const KisGLTexturesInfo *m_texturesInfo;
    bool m_needsMipmapRegeneration;
    int m_preparedLodPlane;
    int m_numMipmapLevels;
    QOpenGLFunctions *f;
    KisOpenGLBufferCircularStorage *m_bufferStorage;
    Q_DISABLE_COPY(KisTextureTile)
};

#endif /* KIS_TEXTURE_TILE_H_ */

