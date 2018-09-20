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

#define GL_GLEXT_PROTOTYPES
#include "kis_texture_tile.h"
#include "kis_texture_tile_update_info.h"

#include <kis_debug.h>
#if !defined(QT_OPENGL_ES)
#include <QOpenGLBuffer>
#endif

#ifndef GL_BGRA
#define GL_BGRA 0x814F
#endif

void KisTextureTile::setTextureParameters()
{

    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 0);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, m_numMipmapLevels);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, m_numMipmapLevels);

    if (m_texturesInfo->internalFormat == GL_RGBA8 && m_texturesInfo->format == GL_RGBA) {
        // If image format is RGBA8, swap the red and blue channels for the proper colour
        // This is for OpenGL ES support and only used if lacking GL_EXT_texture_format_BGRA8888
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_BLUE);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
    }

    f->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

inline QRectF relativeRect(const QRect &br /* baseRect */,
                           const QRect &cr /* childRect */,
                           const KisGLTexturesInfo *texturesInfo)
{
    const qreal x = qreal(cr.x() - br.x()) / texturesInfo->width;
    const qreal y = qreal(cr.y() - br.y()) / texturesInfo->height;
    const qreal w = qreal(cr.width()) / texturesInfo->width;
    const qreal h = qreal(cr.height()) / texturesInfo->height;

    return QRectF(x, y, w, h);
}


KisTextureTile::KisTextureTile(const QRect &imageRect, const KisGLTexturesInfo *texturesInfo,
                               const QByteArray &fillData, KisOpenGL::FilterMode filter,
                               bool useBuffer, int numMipmapLevels, QOpenGLFunctions *fcn)

    : m_textureId(0)
#ifdef USE_PIXEL_BUFFERS
    , m_glBuffer(0)
#endif
    , m_tileRectInImagePixels(imageRect)
    , m_filter(filter)
    , m_texturesInfo(texturesInfo)
    , m_needsMipmapRegeneration(false)
    , m_preparedLodPlane(0)
    , m_useBuffer(useBuffer)
    , m_numMipmapLevels(numMipmapLevels)
    , f(fcn)
{
    const GLvoid *fd = fillData.constData();

    m_textureRectInImagePixels =
            kisGrowRect(m_tileRectInImagePixels, texturesInfo->border);

    m_tileRectInTexturePixels = relativeRect(m_textureRectInImagePixels,
                                             m_tileRectInImagePixels,
                                             m_texturesInfo);

    f->glGenTextures(1, &m_textureId);
    f->glBindTexture(GL_TEXTURE_2D, m_textureId);

    setTextureParameters();

#ifdef USE_PIXEL_BUFFERS
    createTextureBuffer(fillData.constData(), fillData.size());
    // we set fill data to 0 so the next glTexImage2D call uses our buffer
    fd = 0;
#endif

    f->glTexImage2D(GL_TEXTURE_2D, 0,
                 m_texturesInfo->internalFormat,
                 m_texturesInfo->width,
                 m_texturesInfo->height, 0,
                 m_texturesInfo->format,
                 m_texturesInfo->type, fd);

#ifdef USE_PIXEL_BUFFERS
    if (m_useBuffer && m_glBuffer) {
        m_glBuffer->release();
    }
#endif

    setNeedsMipmapRegeneration();
}

KisTextureTile::~KisTextureTile()
{
#ifdef USE_PIXEL_BUFFERS
    delete m_glBuffer;
#endif
    f->glDeleteTextures(1, &m_textureId);
}

int KisTextureTile::bindToActiveTexture(bool blockMipmapRegeneration)
{
    f->glBindTexture(GL_TEXTURE_2D, m_textureId);

    if (m_needsMipmapRegeneration && !blockMipmapRegeneration) {
        f->glGenerateMipmap(GL_TEXTURE_2D);
        setPreparedLodPlane(0);
    }

    return m_preparedLodPlane;
}

void KisTextureTile::setNeedsMipmapRegeneration()
{
    if (m_filter == KisOpenGL::TrilinearFilterMode ||
        m_filter == KisOpenGL::HighQualityFiltering) {

        m_needsMipmapRegeneration = true;
    }
}

void KisTextureTile::setPreparedLodPlane(int lod)
{
    m_preparedLodPlane = lod;
    m_needsMipmapRegeneration = false;
}

void KisTextureTile::update(const KisTextureTileUpdateInfo &updateInfo, bool blockMipmapRegeneration)
{
    f->initializeOpenGLFunctions();
    f->glBindTexture(GL_TEXTURE_2D, m_textureId);

    setTextureParameters();

    const int patchLevelOfDetail = updateInfo.patchLevelOfDetail();
    const QSize patchSize = updateInfo.realPatchSize();
    const QPoint patchOffset = updateInfo.realPatchOffset();

    const GLvoid *fd = updateInfo.data();
#ifdef USE_PIXEL_BUFFERS
    if (!m_glBuffer) {
        createTextureBuffer((const char*)updateInfo.data(), updateInfo.patchPixelsLength());
    }
#endif

    /**
     * In some special case, when the Lod0 stroke is cancelled the
     * following situation is possible:
     *
     * 1)  The stroke  is  cancelled,  Lod0 update  is  issued by  the
     *     image. LodN level of the openGL times is still dirty.
     *
     * 2) [here, ideally, the canvas should be re-rendered, so that
     *     the mipmap would be regenerated in bindToActiveTexture()
     *     call, by in some cases (if you cancel and paint to quickly),
     *     that doesn't have time to happen]
     *
     * 3) The new LodN stroke issues a *partial* update of a LodN
     *    plane of the tile. But the plane is still *dirty*! We update
     *    a part of it, but we cannot regenerate the mipmap anymore,
     *    because the Lod0 level is not known yet!
     *
     * To avoid this issue, we should regenerate the dirty mipmap
     * *before* doing anything with the low-resolution plane.
     */
    if (!blockMipmapRegeneration &&
        patchLevelOfDetail > 0 &&
        m_needsMipmapRegeneration &&
        !updateInfo.isEntireTileUpdated()) {

        f->glGenerateMipmap(GL_TEXTURE_2D);
        m_needsMipmapRegeneration = false;
    }


    if (updateInfo.isEntireTileUpdated()) {

#ifdef USE_PIXEL_BUFFERS
        if (m_useBuffer) {

            m_glBuffer->bind();
            m_glBuffer->allocate(updateInfo.patchPixelsLength());

            void *vid = m_glBuffer->map(QOpenGLBuffer::WriteOnly);
            memcpy(vid, fd, updateInfo.patchPixelsLength());
            m_glBuffer->unmap();

            // we set fill data to 0 so the next glTexImage2D call uses our buffer
            fd = 0;
        }
#endif

        f->glTexImage2D(GL_TEXTURE_2D, patchLevelOfDetail,
                     m_texturesInfo->internalFormat,
                     patchSize.width(),
                     patchSize.height(), 0,
                     m_texturesInfo->format,
                     m_texturesInfo->type,
                     fd);

#ifdef USE_PIXEL_BUFFERS
        if (m_useBuffer) {
            m_glBuffer->release();
        }
#endif

    }
    else {
#ifdef USE_PIXEL_BUFFERS
        if (m_useBuffer) {
            m_glBuffer->bind();
            int size = patchSize.width() * patchSize.height() * updateInfo.pixelSize();
            m_glBuffer->allocate(size);

            void *vid = m_glBuffer->map(QOpenGLBuffer::WriteOnly);
            memcpy(vid, fd, size);
            m_glBuffer->unmap();

            // we set fill data to 0 so the next glTexImage2D call uses our buffer
            fd = 0;
        }
#endif

        f->glTexSubImage2D(GL_TEXTURE_2D, patchLevelOfDetail,
                        patchOffset.x(), patchOffset.y(),
                        patchSize.width(), patchSize.height(),
                        m_texturesInfo->format,
                        m_texturesInfo->type,
                        fd);

#ifdef USE_PIXEL_BUFFERS
        if (m_useBuffer) {
            m_glBuffer->release();
        }
#endif

    }

    /**
     * On the boundaries of KisImage, there is a border-effect as well.
     * So we just repeat the bounding pixels of the image to make
     * bilinear interpolator happy.
     */

    /**
     * WARN: The width of the stripes will be equal to the broder
     *       width of the tiles.
     */

    const int pixelSize = updateInfo.pixelSize();
    const QSize tileSize = updateInfo.realTileSize();

    if(updateInfo.isTopmost()) {
        int start = 0;
        int end = patchOffset.y() - 1;
        for (int i = start; i <= end; i++) {
            f->glTexSubImage2D(GL_TEXTURE_2D, patchLevelOfDetail,
                            patchOffset.x(), i,
                            patchSize.width(), 1,
                            m_texturesInfo->format,
                            m_texturesInfo->type,
                            updateInfo.data());
        }
    }

    if (updateInfo.isBottommost()) {
        int shift = patchSize.width() * (patchSize.height() - 1) *
                pixelSize;

        int start = patchOffset.y() + patchSize.height();
        int end = tileSize.height() - 1;
        for (int i = start; i < end; i++) {
            f->glTexSubImage2D(GL_TEXTURE_2D, patchLevelOfDetail,
                            patchOffset.x(), i,
                            patchSize.width(), 1,
                            m_texturesInfo->format,
                            m_texturesInfo->type,
                            updateInfo.data() + shift);
        }
    }

    if (updateInfo.isLeftmost()) {

        QByteArray columnBuffer(patchSize.height() * pixelSize, 0);

        quint8 *srcPtr = updateInfo.data();
        quint8 *dstPtr = (quint8*) columnBuffer.data();
        for(int i = 0; i < patchSize.height(); i++) {
            memcpy(dstPtr, srcPtr, pixelSize);

            srcPtr += patchSize.width() * pixelSize;
            dstPtr += pixelSize;
        }

        int start = 0;
        int end = patchOffset.x() - 1;
        for (int i = start; i <= end; i++) {
            f->glTexSubImage2D(GL_TEXTURE_2D, patchLevelOfDetail,
                            i, patchOffset.y(),
                            1, patchSize.height(),
                            m_texturesInfo->format,
                            m_texturesInfo->type,
                            columnBuffer.constData());
        }
    }

    if (updateInfo.isRightmost()) {

        QByteArray columnBuffer(patchSize.height() * pixelSize, 0);

        quint8 *srcPtr = updateInfo.data() + (patchSize.width() - 1) * pixelSize;
        quint8 *dstPtr = (quint8*) columnBuffer.data();
        for(int i = 0; i < patchSize.height(); i++) {
            memcpy(dstPtr, srcPtr, pixelSize);

            srcPtr += patchSize.width() * pixelSize;
            dstPtr += pixelSize;
        }

        int start = patchOffset.x() + patchSize.width();
        int end = tileSize.width() - 1;
        for (int i = start; i <= end; i++) {
            f->glTexSubImage2D(GL_TEXTURE_2D, patchLevelOfDetail,
                            i, patchOffset.y(),
                            1, patchSize.height(),
                            m_texturesInfo->format,
                            m_texturesInfo->type,
                            columnBuffer.constData());
        }
    }

    //// Uncomment this warning if you see any weird flickering when
    //// Instant Preview updates
    // if (!updateInfo.isEntireTileUpdated() &&
    //     !(!patchLevelOfDetail || !m_preparedLodPlane || patchLevelOfDetail == m_preparedLodPlane)) {
    //     qDebug() << "WARNING: LodN switch is requested for the partial tile update!. Flickering is possible..." << ppVar(patchSize);
    //     qDebug() << "    " << ppVar(m_preparedLodPlane);
    //     qDebug() << "    " << ppVar(patchLevelOfDetail);
    // }

    if (!patchLevelOfDetail) {
        setNeedsMipmapRegeneration();
    } else {
        setPreparedLodPlane(patchLevelOfDetail);
    }
}

QRectF KisTextureTile::imageRectInTexturePixels(const QRect &imageRect) const
{
    return relativeRect(m_textureRectInImagePixels,
                        imageRect,
                        m_texturesInfo);

}

#ifdef USE_PIXEL_BUFFERS
void KisTextureTile::createTextureBuffer(const char *data, int size)
{
    if (m_useBuffer) {
        if (!m_glBuffer) {
            m_glBuffer = new QOpenGLBuffer(QOpenGLBuffer::PixelUnpackBuffer);
            m_glBuffer->setUsagePattern(QOpenGLBuffer::DynamicDraw);
            m_glBuffer->create();
            m_glBuffer->bind();
            m_glBuffer->allocate(size);
        }
        void *vid = m_glBuffer->map(QOpenGLBuffer::WriteOnly);
        memcpy(vid, data, size);
        m_glBuffer->unmap();

    }
    else {
        delete m_glBuffer;
        m_glBuffer = 0;
    }
}
#endif
