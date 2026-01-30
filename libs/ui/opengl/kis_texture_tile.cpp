/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#define GL_GLEXT_PROTOTYPES
#include "kis_texture_tile.h"
#include "kis_texture_tile_update_info.h"
#include "KisOpenGLBufferCircularStorage.h"

#include <kis_debug.h>
#if !defined(QT_OPENGL_ES)
#include <QOpenGLBuffer>
#endif

#ifndef GL_BGRA
#define GL_BGRA 0x814F
#endif

#ifndef GL_RGBA16_EXT
#define GL_RGBA16_EXT 0x805B
#endif


void KisTextureTile::setTextureParameters()
{

    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 0);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, m_numMipmapLevels);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, m_numMipmapLevels);

    if ((m_texturesInfo->internalFormat == GL_RGBA8 && m_texturesInfo->format == GL_RGBA)
#ifndef QT_OPENGL_ES_2
        || (m_texturesInfo->internalFormat == GL_RGBA16 && m_texturesInfo->format == GL_RGBA)
#endif
        || (m_texturesInfo->internalFormat == GL_RGBA16_EXT && m_texturesInfo->format == GL_RGBA)
    ) {
        // If image format is RGBA8, swap the red and blue channels for the proper color
        // This is for OpenGL ES support and only used if lacking GL_EXT_texture_format_BGRA8888
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_BLUE);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
    }

    f->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

void KisTextureTile::restoreTextureParameters()
{
    // QPainter::drawText relies on this.
    // Ref: https://bugreports.qt.io/browse/QTBUG-65496
    f->glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
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

#include "kis_debug.h"

KisTextureTile::KisTextureTile(const QRect &imageRect, const KisGLTexturesInfo *texturesInfo,
                               const QByteArray &fillData, KisOpenGL::FilterMode filter,
                               KisOpenGLBufferCircularStorage *bufferStorage, int numMipmapLevels, QOpenGLFunctions *fcn)

    : m_textureId(0)
    , m_tileRectInImagePixels(imageRect)
    , m_filter(filter)
    , m_texturesInfo(texturesInfo)
    , m_needsMipmapRegeneration(false)
    , m_preparedLodPlane(0)
    , m_numMipmapLevels(numMipmapLevels)
    , f(fcn)
    , m_bufferStorage(bufferStorage)
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

    KisOpenGLBufferCircularStorage::BufferBinder binder(
        m_bufferStorage, &fd, fillData.size());

    f->glTexImage2D(GL_TEXTURE_2D, 0,
                 m_texturesInfo->internalFormat,
                 m_texturesInfo->width,
                 m_texturesInfo->height, 0,
                 m_texturesInfo->format,
                 m_texturesInfo->type, fd);

    restoreTextureParameters();

    setNeedsMipmapRegeneration();
}

KisTextureTile::~KisTextureTile()
{
    f->glDeleteTextures(1, &m_textureId);
}

int KisTextureTile::bindToActiveTexture(bool blockMipmapRegeneration)
{
    f->glBindTexture(GL_TEXTURE_2D, m_textureId);

    if (m_needsMipmapRegeneration && !blockMipmapRegeneration) {
        regenerateMipmap();
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

void KisTextureTile::regenerateMipmap()
{
    f->glGenerateMipmap(GL_TEXTURE_2D);
    m_mipmapHasBeenAllocated = true;
    m_needsMipmapRegeneration = false;
}

void KisTextureTile::update(const KisTextureTileUpdateInfo &updateInfo, bool blockMipmapRegeneration)
{
    f->glBindTexture(GL_TEXTURE_2D, m_textureId);

    setTextureParameters();

    const int patchLevelOfDetail = updateInfo.patchLevelOfDetail();
    const QSize patchSize = updateInfo.realPatchSize();
    const QPoint patchOffset = updateInfo.realPatchOffset();

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
     *
     * Another case is when the user has Bilinear or
     * Nearest Neighbour filtering selected and tries to use LoD
     * functionality in animation. glTexSubImage2D() and textureLod()
     * are defined only when all the planes were explicitly initialized
     * with glTexImage2D(), which doesn't happen in case of bilinear- or
     * nn-filtering. In this case !m_mipmapHasBeenAllocated condition
     * comes in.
     */
    if (!blockMipmapRegeneration &&
        patchLevelOfDetail > 0 &&
        (m_needsMipmapRegeneration &&
         !updateInfo.isEntireTileUpdated())
            || !m_mipmapHasBeenAllocated) {

        regenerateMipmap();
    }

    /*
     * To avoid unsightly seams in wraparound mode, we extend the tile at the
     * edges by just repeating the top/left/right/bottom row/column until the
     * end of the texture. For that, we shuffle the data into a buffer, then
     * pass that to the PBO. Previously, this code used to allocate up to four
     * additional, single-pixel thick (and sometimes zero-pixel long) PBOs and
     * hammer out the extensions through multiple calls to glTexSubImage2D, but
     * that caused very long stalls on Android. If you are in the future and
     * for some reason need to revert that, also update KisOpenGLImageTextures
     * to properly account for these extra PBOs being used, since it assumes
     * each tile only grabs one PBO from the circular buffer storage.
     */
    QSize tileSize = updateInfo.realTileSize();
    int pixelSize = updateInfo.pixelSize();
    int centerWidth = patchSize.width();
    int centerHeight = patchSize.height();

    int topHeight;
    if (updateInfo.isTopmost()) {
        topHeight = patchOffset.y();
    } else {
        topHeight = 0;
    }

    int leftWidth;
    if (updateInfo.isLeftmost()) {
        leftWidth = patchOffset.x();
    } else {
        leftWidth = 0;
    }

    int rightWidth;
    if (updateInfo.isRightmost()) {
        rightWidth = tileSize.width() - patchOffset.x() - centerWidth;
    } else {
        rightWidth = 0;
    }

    int bottomHeight;
    if (updateInfo.isBottommost()) {
        bottomHeight = tileSize.height() - patchOffset.y() - centerHeight;
    } else {
        bottomHeight = 0;
    }

    if (topHeight > 0 || leftWidth > 0 || rightWidth > 0 || bottomHeight > 0) {
        int bufWidth = leftWidth + centerWidth + rightWidth;
        int bufHeight = topHeight + centerHeight + bottomHeight;

        int centerStride = centerWidth * pixelSize;
        int leftStride = leftWidth * pixelSize;
        int rightStride = rightWidth * pixelSize;
        int bufStride = bufWidth * pixelSize;

        QByteArray buf(bufStride * bufHeight, 0);
        quint8 *bufData = reinterpret_cast<quint8 *>(buf.data());
        const quint8 *patchData = updateInfo.data();

        if (topHeight > 0) {
            const quint8 *topSrcPtr = patchData;
            quint8 *topDstPtr = bufData + leftStride;
            for (int y = 0; y < topHeight; ++y) {
                memcpy(topDstPtr, topSrcPtr, centerStride);
                topDstPtr += bufStride;
            }
        }

        if (leftWidth > 0) {
            int leftDstSkip = centerStride + rightStride;
            const quint8 *leftSrcPtr = patchData;
            quint8 *leftDstPtr = bufData + (topHeight * bufStride);
            for (int y = 0; y < centerHeight; ++y) {
                for (int x = 0; x < leftWidth; ++x) {
                    memcpy(leftDstPtr, leftSrcPtr, pixelSize);
                    leftDstPtr += pixelSize;
                }
                leftSrcPtr += centerStride;
                leftDstPtr += leftDstSkip;
            }
        }

        {
            const quint8 *centerSrcPtr = patchData;
            quint8 *centerDstPtr = bufData + (topHeight * bufStride) + leftStride;
            for (int y = 0; y < centerHeight; ++y) {
                memcpy(centerDstPtr, centerSrcPtr, centerStride);
                centerSrcPtr += centerStride;
                centerDstPtr += bufStride;
            }
        }

        if (rightWidth > 0) {
            int rightDstSkip = leftStride + centerStride;
            const quint8 *rightSrcPtr = patchData + (centerStride - pixelSize);
            quint8 *rightDstPtr = bufData + (topHeight * bufStride) + rightDstSkip;
            for (int y = 0; y < centerHeight; ++y) {
                for (int x = 0; x < rightWidth; ++x) {
                    memcpy(rightDstPtr, rightSrcPtr, pixelSize);
                    rightDstPtr += pixelSize;
                }
                rightSrcPtr += centerStride;
                rightDstPtr += rightDstSkip;
            }
        }

        if (bottomHeight > 0) {
            const quint8 *bottomSrcPtr = patchData + ((centerHeight - 1) * centerStride);
            quint8 *bottomDstPtr = bufData + ((topHeight + centerHeight) * bufStride) + leftStride;
            for (int y = 0; y < bottomHeight; ++y) {
                memcpy(bottomDstPtr, bottomSrcPtr, centerStride);
                bottomDstPtr += bufStride;
            }
        }

        const GLvoid *fd = bufData;
        KisOpenGLBufferCircularStorage::BufferBinder b(m_bufferStorage, &fd, buf.size());
        f->glTexSubImage2D(GL_TEXTURE_2D,
                           patchLevelOfDetail,
                           patchOffset.x() - leftWidth,
                           patchOffset.y() - topHeight,
                           bufWidth,
                           bufHeight,
                           m_texturesInfo->format,
                           m_texturesInfo->type,
                           fd);

    } else if (updateInfo.isEntireTileUpdated()) {
        const GLvoid *fd = updateInfo.data();
        KisOpenGLBufferCircularStorage::BufferBinder b(
            m_bufferStorage, &fd, updateInfo.patchPixelsLength());

        f->glTexImage2D(GL_TEXTURE_2D, patchLevelOfDetail,
                     m_texturesInfo->internalFormat,
                     patchSize.width(),
                     patchSize.height(), 0,
                     m_texturesInfo->format,
                     m_texturesInfo->type,
                     fd);

    } else {
        const GLvoid *fd = updateInfo.data();
        const int size = centerWidth * centerHeight * pixelSize;
        KisOpenGLBufferCircularStorage::BufferBinder b(
            m_bufferStorage, &fd, size);

        f->glTexSubImage2D(GL_TEXTURE_2D, patchLevelOfDetail,
                        patchOffset.x(), patchOffset.y(),
                        patchSize.width(), patchSize.height(),
                        m_texturesInfo->format,
                        m_texturesInfo->type,
                        fd);
    }

    //// Uncomment this warning if you see any weird flickering when
    //// Instant Preview updates
    // if (!updateInfo.isEntireTileUpdated() &&
    //     !(!patchLevelOfDetail || !m_preparedLodPlane || patchLevelOfDetail == m_preparedLodPlane)) {
    //     qDebug() << "WARNING: LodN switch is requested for the partial tile update!. Flickering is possible..." << ppVar(patchSize);
    //     qDebug() << "    " << ppVar(m_preparedLodPlane);
    //     qDebug() << "    " << ppVar(patchLevelOfDetail);
    // }

    restoreTextureParameters();

    if (!patchLevelOfDetail) {
        if (m_mipmapHasBeenAllocated &&
                (m_filter == KisOpenGL::BilinearFilterMode ||
                 m_filter == KisOpenGL::NearestFilterMode)) {
            /**
             * When in a mode that doesn't use mipmaps we should
             * just switch back onto lod0 plane instead of requesting
             * full mipmap regeneration.
             */

            setPreparedLodPlane(0);
        } else {
            setNeedsMipmapRegeneration();
        }
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
