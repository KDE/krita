/*
 *  Copyright (c) 2005-2007 Adrian Page <adrian@pagenet.plus.com>
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

#include "opengl/kis_opengl_image_textures.h"

#ifdef HAVE_OPENGL

#include <ksharedptr.h>

#include <QGLWidget>
#include <QImage>
#include <QApplication>

#include <KoConfig.h>

#ifdef HAVE_OPENEXR
#include <half.h>
#endif

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>
#include <KoIntegerMaths.h>
#include <KoColorModelStandardIds.h>

#include "kis_global.h"

#include "kis_image.h"
#include "kis_layer.h"
#include "kis_selection.h"
#include "kis_config.h"
#include "kis_debug.h"

#ifdef HAVE_GLEW
#include "opengl/kis_opengl_hdr_exposure_program.h"
#endif

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#ifndef GL_BGRA
#define GL_BGRA 0x80E1
#endif

using namespace std;

KisOpenGLImageTextures::ImageTexturesMap KisOpenGLImageTextures::imageTexturesMap;

const int KisOpenGLImageTextures::PREFERRED_IMAGE_TEXTURE_WIDTH = 256;
const int KisOpenGLImageTextures::PREFERRED_IMAGE_TEXTURE_HEIGHT = 256;

#ifdef HAVE_GLEW
KisOpenGLHDRExposureProgram *KisOpenGLImageTextures::HDRExposureProgram = 0;
#endif

KisOpenGLImageTextures::KisOpenGLImageTextures()
{
    m_image = 0;
    m_monitorProfile = 0;
    m_exposure = 0;
}

KisOpenGLImageTextures::KisOpenGLImageTextures(KisImageWSP image, KoColorProfile *monitorProfile)
{
    dbgUI << "Creating KisOpenGLImageTextures";

    m_image = image;
    m_monitorProfile = monitorProfile;
    m_exposure = 0;
    m_displaySelection = true;

    KisOpenGL::makeContextCurrent();

    glGenTextures(1, &m_backgroundTexture);

    GLint max_texture_size;

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);

    m_imageTextureTileWidth = qMin((GLint)PREFERRED_IMAGE_TEXTURE_WIDTH, max_texture_size);
    m_imageTextureTileHeight = qMin((GLint)PREFERRED_IMAGE_TEXTURE_HEIGHT, max_texture_size);

    createImageTextureTiles();

    KisOpenGLUpdateInfoSP info = updateCache(m_image->bounds());
    recalculateCache(info);
}

KisOpenGLImageTextures::~KisOpenGLImageTextures()
{
    dbgUI << "Destroying KisOpenGLImageTextures";

    ImageTexturesMap::iterator it = imageTexturesMap.find(m_image);

    if (it != imageTexturesMap.end()) {

        KisOpenGLImageTextures *textures = (*it).second;

        if (textures == this) {
            dbgUI << "Removing shared image context from map";
            imageTexturesMap.erase(m_image);
        }
    }
    destroyImageTextureTiles();
    glDeleteTextures(1, &m_backgroundTexture);
}

KisOpenGLImageTexturesSP KisOpenGLImageTextures::getImageTextures(KisImageWSP image, KoColorProfile *monitorProfile)
{
    KisOpenGL::makeContextCurrent();

#ifdef HAVE_GLEW
    createHDRExposureProgramIfCan();
#endif

    if (imageCanShareTextures(image)) {
        ImageTexturesMap::iterator it = imageTexturesMap.find(image);

        if (it != imageTexturesMap.end()) {

            dbgUI << "Sharing image textures from map";

            KisOpenGLImageTexturesSP textures = (*it).second;
            textures->setMonitorProfile(monitorProfile);

            return textures;
        } else {
            KisOpenGLImageTextures *imageTextures = new KisOpenGLImageTextures(image, monitorProfile);
            imageTexturesMap[image] = imageTextures;

            dbgUI << "Added shareable textures to map";

            return imageTextures;
        }
    } else {
        dbgUI << "Creating non-shareable image textures";

        return new KisOpenGLImageTextures(image, monitorProfile);
    }
}

bool KisOpenGLImageTextures::imageCanShareTextures(KisImageWSP image)
{
    return !image->colorSpace()->hasHighDynamicRange() || imageCanUseHDRExposureProgram(image);
}

void KisOpenGLImageTextures::createImageTextureTiles()
{
    KisOpenGL::makeContextCurrent();

    destroyImageTextureTiles();

    m_numImageTextureTileColumns = (m_image->width() + m_imageTextureTileWidth - 1) / m_imageTextureTileWidth;
    int numImageTextureTileRows = (m_image->height() + m_imageTextureTileHeight - 1) / m_imageTextureTileHeight;
    int numImageTextureTiles = m_numImageTextureTileColumns * numImageTextureTileRows;

    m_imageTextureTiles.resize(numImageTextureTiles);
    glGenTextures(numImageTextureTiles, &(m_imageTextureTiles[0]));

    setImageTextureFormat();

    // Fill with transparent black
    const int NUM_RGBA_COMPONENTS = 4;
    QByteArray emptyTilePixelData(m_imageTextureTileWidth * m_imageTextureTileHeight * NUM_RGBA_COMPONENTS, 0);

    for (int tileIndex = 0; tileIndex < numImageTextureTiles; ++tileIndex) {

        glBindTexture(GL_TEXTURE_2D, m_imageTextureTiles[tileIndex]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexImage2D(GL_TEXTURE_2D, 0, m_imageTextureInternalFormat, m_imageTextureTileWidth, m_imageTextureTileHeight, 0,
                     GL_BGRA, GL_UNSIGNED_BYTE, emptyTilePixelData.data());
    }
}

void KisOpenGLImageTextures::destroyImageTextureTiles()
{
    if (!m_imageTextureTiles.empty()) {
        KisOpenGL::makeContextCurrent();
        glDeleteTextures(m_imageTextureTiles.count(), &(m_imageTextureTiles[0]));
        m_imageTextureTiles.clear();
    }
}

KisOpenGLUpdateInfoSP
KisOpenGLImageTextures::updateCache(const QRect& rect)
{
    KisOpenGLUpdateInfoSP info = new KisOpenGLUpdateInfo();

    QRect updateRect = rect & m_image->bounds();
    if (updateRect.isEmpty()) return info;

    int firstColumn = updateRect.left() / m_imageTextureTileWidth;
    int lastColumn = updateRect.right() / m_imageTextureTileWidth;
    int firstRow = updateRect.top() / m_imageTextureTileHeight;
    int lastRow = updateRect.bottom() / m_imageTextureTileHeight;

    qint32 numItems = (lastColumn - firstColumn + 1) * (lastRow - firstRow + 1);
    info->tileList.reserve(numItems);

    for (int col = firstColumn; col <= lastColumn; col++) {
        for (int row = firstRow; row <= lastRow; row++) {

            QRect tileRect(col * m_imageTextureTileWidth, row * m_imageTextureTileHeight,
                           m_imageTextureTileWidth, m_imageTextureTileHeight);

            KisTextureTileUpdateInfo tile(tileRect, updateRect);
            tile.retrieveData(m_image);
            info->tileList.append(tile);
        }
    }
    return info;
}

void KisOpenGLImageTextures::recalculateCache(KisUpdateInfoSP info)
{
    KisOpenGLUpdateInfoSP glInfo = dynamic_cast<KisOpenGLUpdateInfo*>(info.data());
    if(!glInfo) return;

    KisOpenGL::makeContextCurrent();

    KIS_OPENGL_CLEAR_ERROR();

    KisTextureTileUpdateInfo tile;

    foreach(tile, glInfo->tileList) {
        glBindTexture(GL_TEXTURE_2D, imageTextureTile(tile.tileRect().x(), tile.tileRect().y()));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);//GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        const KoColorSpace *dstCS;

        switch(m_imageTextureType) {
        case GL_UNSIGNED_BYTE:
            dstCS = KoColorSpaceRegistry::instance()->rgb8(m_monitorProfile);
            break;
#if defined(HAVE_GLEW) && defined(HAVE_OPENEXR)
        case GL_UNSIGNED_SHORT:
            dstCS = KoColorSpaceRegistry::instance()->rgb16(m_monitorProfile);
            break;
        case GL_HALF_FLOAT_ARB:
            dstCS = KoColorSpaceRegistry::instance()->colorSpace("RGBA", "F16", m_monitorProfile);
            break;
        case GL_FLOAT:
            dstCS = KoColorSpaceRegistry::instance()->colorSpace("RGBA", "F32", m_monitorProfile);
            break;
#endif
        default:
            qFatal("Unknown m_imageTextureType");
        }

        tile.convertTo(dstCS);

        if (tile.isEntireTileUpdated()) {
            glTexImage2D(GL_TEXTURE_2D, 0, m_imageTextureInternalFormat, m_imageTextureTileWidth, m_imageTextureTileHeight, 0,
                         GL_BGRA, m_imageTextureType, tile.data());
        } else {
            QPoint patchOffset = tile.patchOffset();
            QSize patchSize = tile.patchSize();
            glTexSubImage2D(GL_TEXTURE_2D, 0, patchOffset.x(), patchOffset.y(),
                            patchSize.width(), patchSize.height(),
                            GL_BGRA, m_imageTextureType, tile.data());
        }

        tile.destroy();

        KIS_OPENGL_PRINT_ERROR();
    }
}

void KisOpenGLImageTextures::generateBackgroundTexture(const QImage & checkImage)
{
    KisOpenGL::makeContextCurrent();

    glBindTexture(GL_TEXTURE_2D, m_backgroundTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    Q_ASSERT(checkImage.width() == BACKGROUND_TEXTURE_SIZE);
    Q_ASSERT(checkImage.height() == BACKGROUND_TEXTURE_SIZE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, BACKGROUND_TEXTURE_SIZE, BACKGROUND_TEXTURE_SIZE, 0,
                 GL_BGRA, GL_UNSIGNED_BYTE, checkImage.bits());
}

GLuint KisOpenGLImageTextures::backgroundTexture() const
{
    return m_backgroundTexture;
}

int KisOpenGLImageTextures::imageTextureTileIndex(int x, int y) const
{
    int column = x / m_imageTextureTileWidth;
    int row = y / m_imageTextureTileHeight;

    return column + (row * m_numImageTextureTileColumns);
}

GLuint KisOpenGLImageTextures::imageTextureTile(int pixelX, int pixelY) const
{
    qint32 textureTileIndex = imageTextureTileIndex(pixelX, pixelY);

    textureTileIndex = CLAMP(textureTileIndex, 0, ((qint32)m_imageTextureTiles.count()) - 1);

    return m_imageTextureTiles[textureTileIndex];
}

int KisOpenGLImageTextures::imageTextureTileWidth() const
{
    return m_imageTextureTileWidth;
}

int KisOpenGLImageTextures::imageTextureTileHeight() const
{
    return m_imageTextureTileHeight;
}

void KisOpenGLImageTextures::update(const QRect& imageRect)
{
    KisOpenGLUpdateInfoSP info = updateCache(imageRect);
    recalculateCache(info);
}

void KisOpenGLImageTextures::setSelectionDisplayEnabled(bool enable)
{
    m_displaySelection = enable;
}

void KisOpenGLImageTextures::slotImageUpdated(const QRect &rc)
{
    QRect r = rc & m_image->bounds();

    KisOpenGLUpdateInfoSP info = updateCache(r);
    recalculateCache(info);

    emit sigImageUpdated(r);
}

void KisOpenGLImageTextures::slotImageSizeChanged(qint32 w, qint32 h)
{
    createImageTextureTiles();
    KisOpenGLUpdateInfoSP info = updateCache(m_image->bounds());
    recalculateCache(info);

    emit sigSizeChanged(w, h);
}

void KisOpenGLImageTextures::setMonitorProfile(KoColorProfile *monitorProfile)
{
    if (monitorProfile != m_monitorProfile) {
        m_monitorProfile = monitorProfile;
        KisOpenGLUpdateInfoSP info = updateCache(m_image->bounds());
        recalculateCache(info);
    }
}

void KisOpenGLImageTextures::setHDRExposure(float exposure)
{
    if (exposure != m_exposure) {
        m_exposure = exposure;

        if (m_image->colorSpace()->hasHighDynamicRange()) {
#ifdef HAVE_GLEW
            if (m_usingHDRExposureProgram) {
                HDRExposureProgram->setExposure(exposure);
            } else {
#endif
                QApplication::setOverrideCursor(Qt::WaitCursor);
                KisOpenGLUpdateInfoSP info = updateCache(m_image->bounds());
                recalculateCache(info);
                QApplication::restoreOverrideCursor();
#ifdef HAVE_GLEW
            }
#endif
        }
    }
}

void KisOpenGLImageTextures::createHDRExposureProgramIfCan()
{
    KisConfig cfg;
    if (!cfg.useOpenGLShaders()) return;

#ifdef HAVE_GLEW
    if (HDRExposureProgram == 0 && KisOpenGL::hasShadingLanguage()) {
        dbgUI << "Creating shared HDR exposure program";
        HDRExposureProgram = new KisOpenGLHDRExposureProgram();
        Q_CHECK_PTR(HDRExposureProgram);
    }
#endif
}

bool KisOpenGLImageTextures::usingHDRExposureProgram() const
{
#ifdef HAVE_GLEW
    return m_usingHDRExposureProgram;
#else
    return false;
#endif
}

void KisOpenGLImageTextures::activateHDRExposureProgram()
{
#ifdef HAVE_GLEW
    if (m_usingHDRExposureProgram) {
        HDRExposureProgram->activate();
    }
#endif
}

void KisOpenGLImageTextures::deactivateHDRExposureProgram()
{
#ifdef HAVE_GLEW
    if (m_usingHDRExposureProgram) {
        KisOpenGLProgram::deactivate();
    }
#endif
}


void KisOpenGLImageTextures::setImageTextureFormat()
{
    m_imageTextureInternalFormat = GL_RGBA8;
    m_imageTextureType = GL_UNSIGNED_BYTE;

#ifdef HAVE_GLEW
    m_usingHDRExposureProgram = imageCanUseHDRExposureProgram(m_image);

    KoID colorModelId = m_image->colorSpace()->colorModelId();
    KoID colorDepthId = m_image->colorSpace()->colorDepthId();

    dbgUI << "Choosing texture format:";

    if (colorModelId == RGBAColorModelID) {
        if (colorDepthId == Float16BitsColorDepthID && imageCanUseHDRExposureProgram(m_image)) {

            if (GLEW_ARB_texture_float) {
                m_imageTextureInternalFormat = GL_RGBA16F_ARB;
                dbgUI << "Using ARB half";
            } else {
                Q_ASSERT(GLEW_ATI_texture_float);
                m_imageTextureInternalFormat = GL_RGBA_FLOAT16_ATI;
                dbgUI << "Using ATI half";
            }

            if (GLEW_ARB_half_float_pixel) {
                dbgUI << "Pixel type half";
                m_imageTextureType = GL_HALF_FLOAT_ARB;
            } else {
                dbgUI << "Pixel type float";
                m_imageTextureType = GL_FLOAT;
            }

            m_usingHDRExposureProgram = true;

        }
        else if (colorDepthId == Float32BitsColorDepthID && imageCanUseHDRExposureProgram(m_image)) {

            if (GLEW_ARB_texture_float) {
                m_imageTextureInternalFormat = GL_RGBA32F_ARB;
                dbgUI << "Using ARB float";
            } else {
                Q_ASSERT(GLEW_ATI_texture_float);
                m_imageTextureInternalFormat = GL_RGBA_FLOAT32_ATI;
                dbgUI << "Using ATI float";
            }

            m_imageTextureType = GL_FLOAT;
            m_usingHDRExposureProgram = true;
        }
        else if (colorDepthId != Integer8BitsColorDepthID) {
            dbgUI << "Using 16 bits rgba";
            m_imageTextureInternalFormat = GL_RGBA16;
            m_imageTextureType = GL_UNSIGNED_SHORT;
        }
    }
    else {
        // We will convert the colorspace to 16 bits rgba, instead of 8 bits
        if (colorDepthId == Integer16BitsColorDepthID) {
            dbgUI << "Using conversion to 16 bits rgba";
            m_imageTextureInternalFormat = GL_RGBA16;
            m_imageTextureType = GL_UNSIGNED_SHORT;
        }
    }
#endif
}

bool KisOpenGLImageTextures::imageCanUseHDRExposureProgram(KisImageWSP image)
{
#ifdef HAVE_GLEW
    if (!image->colorSpace()->hasHighDynamicRange()) {
        return false;
    }

    KisConfig cfg;

    if (!cfg.useOpenGLShaders()) {
        return false;
    }
    if (HDRExposureProgram == 0) {
        return false;
    }
    if (!HDRExposureProgram->isValid()) {
        return false;
    }

    KisOpenGL::makeContextCurrent();
    KoID colorModelId = image->colorSpace()->colorModelId();
    KoID colorDepthId = image->colorSpace()->colorDepthId();

    if (!(    colorModelId == RGBAColorModelID
              && (colorDepthId == Float16BitsColorDepthID || colorDepthId == Float32BitsColorDepthID)
              && (GLEW_ARB_texture_float || GLEW_ATI_texture_float))) {
        return false;
    }
    return true;
#else
    Q_UNUSED(image);
    return false;
#endif
}

#include "kis_opengl_image_textures.moc"

#endif // HAVE_OPENGL

