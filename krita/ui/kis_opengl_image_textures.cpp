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

#include "kis_opengl_image_textures.h"

#ifdef HAVE_OPENGL

#include <kdebug.h>
#include <ksharedptr.h>

#include <QGLWidget>

#include "config-openexr.h"

#ifdef HAVE_OPENEXR
#include <half.h>
#endif

#include "KoColorSpaceRegistry.h"
#include "KoColorProfile.h"
#include "KoIntegerMaths.h"

#include "kis_global.h"
#include "kis_meta_registry.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_selection.h"
#include "kis_opengl.h"
#include "kis_config.h"
#include "kis_debug_areas.h"

#ifdef HAVE_GLEW
#include "kis_opengl_hdr_exposure_program.h"
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

KisOpenGLImageTextures::KisOpenGLImageTextures(KisImageSP image, KoColorProfile *monitorProfile)
{
    kDebug(DBG_AREA_UI) << "Creating KisOpenGLImageTextures\n";

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

    connect(m_image, SIGNAL(sigImageUpdated(QRect)), SLOT(slotImageUpdated(QRect)));
    connect(m_image, SIGNAL(sigSizeChanged(qint32, qint32)), SLOT(slotImageSizeChanged(qint32, qint32)));

    updateImageTextureTiles(m_image->bounds());
}

KisOpenGLImageTextures::~KisOpenGLImageTextures()
{
    kDebug(DBG_AREA_UI) << "Destroying KisOpenGLImageTextures\n";

    ImageTexturesMap::iterator it = imageTexturesMap.find(m_image);

    if (it != imageTexturesMap.end()) {

        KisOpenGLImageTextures *textures = (*it).second;

        if (textures == this) {
            kDebug(DBG_AREA_UI) << "Removing shared image context from map\n";
            imageTexturesMap.erase(m_image);
        }
    }
    destroyImageTextureTiles();
    glDeleteTextures(1, &m_backgroundTexture);
}

KisOpenGLImageTexturesSP KisOpenGLImageTextures::getImageTextures(KisImageSP image, KoColorProfile *monitorProfile)
{
    KisOpenGL::makeContextCurrent();

#ifdef HAVE_GLEW
    createHDRExposureProgramIfCan();
#endif

    if (imageCanShareTextures(image)) {
        ImageTexturesMap::iterator it = imageTexturesMap.find(image);

        if (it != imageTexturesMap.end()) {

            kDebug(DBG_AREA_UI) << "Sharing image textures from map\n";

            KisOpenGLImageTexturesSP textures = (*it).second;
            textures->setMonitorProfile(monitorProfile);

            return textures;
        } else {
            KisOpenGLImageTextures *imageTextures = new KisOpenGLImageTextures(image, monitorProfile);
            imageTexturesMap[image] = imageTextures;

            kDebug(DBG_AREA_UI) << "Added shareable textures to map\n";

            return imageTextures;
        }
    } else {
        kDebug(DBG_AREA_UI) << "Creating non-shareable image textures\n";

        return new KisOpenGLImageTextures(image, monitorProfile);
    }
}

bool KisOpenGLImageTextures::imageCanShareTextures(KisImageSP image)
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

void KisOpenGLImageTextures::updateImageTextureTiles(const QRect& rect)
{
    kDebug(DBG_AREA_UI) << "updateImageTextureTiles " << rect << endl;

    QRect updateRect = rect & m_image->bounds();

    if (!updateRect.isEmpty()) {

        KisOpenGL::makeContextCurrent();

        KIS_OPENGL_CLEAR_ERROR();

        int firstColumn = updateRect.left() / m_imageTextureTileWidth;
        int lastColumn = updateRect.right() / m_imageTextureTileWidth;
        int firstRow = updateRect.top() / m_imageTextureTileHeight;
        int lastRow = updateRect.bottom() / m_imageTextureTileHeight;

        for (int column = firstColumn; column <= lastColumn; column++) {
            for (int row = firstRow; row <= lastRow; row++) {

                QRect tileRect(column * m_imageTextureTileWidth, row * m_imageTextureTileHeight,
                               m_imageTextureTileWidth, m_imageTextureTileHeight);

                QRect tileUpdateRect = tileRect & updateRect;

                glBindTexture(GL_TEXTURE_2D, imageTextureTile(tileRect.x(), tileRect.y()));
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);//GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

                QImage tileUpdateImage;
                Q_UINT8 *pixels;
                bool deletePixelsAfterUse = false;

                if (m_imageTextureInternalFormat == GL_RGBA8) {
                    tileUpdateImage = m_image->convertToQImage(tileUpdateRect.x(), tileUpdateRect.y(),
                                                               tileUpdateRect.width(), tileUpdateRect.height(),
                                                               m_monitorProfile, m_exposure);

#if 0 // This is the old method of painting selections -- should be
      // ported to whatever Sven Langkamp is doing.
                    if (m_displaySelection) {
                        if (!m_image->activeLayer().isNull()) {
                            m_image->activeLayer()->paint(tileUpdateImage,
                                                          tileUpdateRect.x(), tileUpdateRect.y(),
                                                          tileUpdateRect.width(), tileUpdateRect.height());
                        }
                    }
#endif
                    pixels = tileUpdateImage.bits();
                } else {
                    pixels = new Q_UINT8[tileUpdateRect.width() * tileUpdateRect.height() * m_image->colorSpace()->pixelSize()];
                    Q_CHECK_PTR(pixels);
                    deletePixelsAfterUse = true;

                    m_image->mergedImage()->readBytes(pixels, tileUpdateRect.x(), tileUpdateRect.y(),
                                                      tileUpdateRect.width(), tileUpdateRect.height());

#if defined(HAVE_GLEW) && defined(HAVE_OPENEXR)
                    // XXX: generalise
                    if (m_image->colorSpace()->id() == "RGBAF16HALF") {
                        if (m_imageTextureType == GL_FLOAT) {

                            // Convert half to float as we don't have ARB_half_float_pixel
                            const int NUM_RGBA_COMPONENTS = 4;
                            qint32 halfCount = tileUpdateRect.width() * tileUpdateRect.height() * NUM_RGBA_COMPONENTS;
                            GLfloat *pixels_as_floats = new GLfloat[halfCount];
                            const half *half_pixel = reinterpret_cast<const half *>(pixels);
                            GLfloat *float_pixel = pixels_as_floats;

                            while (halfCount > 0) {
                                *float_pixel = *half_pixel;
                                ++float_pixel;
                                ++half_pixel;
                                --halfCount;
                            }
                            delete [] pixels;
                            pixels = reinterpret_cast<Q_UINT8 *>(pixels_as_floats);
                        } else {
                            Q_ASSERT(m_imageTextureType == GL_HALF_FLOAT_ARB);
                        }
                    }
#endif
                }

                if (tileUpdateRect.width() == m_imageTextureTileWidth && tileUpdateRect.height() == m_imageTextureTileHeight) {

                    glTexImage2D(GL_TEXTURE_2D, 0, m_imageTextureInternalFormat, m_imageTextureTileWidth, m_imageTextureTileHeight, 0,
                          GL_BGRA, m_imageTextureType, pixels);
                } else {
                    int xOffset = tileUpdateRect.x() - tileRect.x();
                    int yOffset = tileUpdateRect.y() - tileRect.y();

                    glTexSubImage2D(GL_TEXTURE_2D, 0, xOffset, yOffset, tileUpdateRect.width(), tileUpdateRect.height(),
                                    GL_BGRA, m_imageTextureType, pixels);
                }

                if (deletePixelsAfterUse) {
                    delete [] pixels;
                }

                KIS_OPENGL_PRINT_ERROR();
            }
        }
    }
}

void KisOpenGLImageTextures::generateBackgroundTexture(QImage checkImage)
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
    updateImageTextureTiles(imageRect);
}

void KisOpenGLImageTextures::setSelectionDisplayEnabled(bool enable)
{
    m_displaySelection = enable;
}

void KisOpenGLImageTextures::slotImageUpdated(const QRect &rc)
{
    QRect r = rc & m_image->bounds();

    updateImageTextureTiles(r);
    emit sigImageUpdated(r);
}

void KisOpenGLImageTextures::slotImageSizeChanged(qint32 w, qint32 h)
{
    createImageTextureTiles();
    updateImageTextureTiles(m_image->bounds());

    emit sigSizeChanged(w, h);
}

void KisOpenGLImageTextures::setMonitorProfile(KoColorProfile *monitorProfile)
{
    if (monitorProfile != m_monitorProfile) {
        m_monitorProfile = monitorProfile;
        updateImageTextureTiles(m_image->bounds());
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
                updateImageTextureTiles(m_image->bounds());
                QApplication::restoreOverrideCursor();
#ifdef HAVE_GLEW
            }
#endif
        }
    }
}

void KisOpenGLImageTextures::createHDRExposureProgramIfCan()
{
#ifdef HAVE_GLEW
    if (HDRExposureProgram == 0 && KisOpenGL::hasShadingLanguage()) {
        kDebug(DBG_AREA_UI) << "Creating shared HDR exposure program\n";
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

bool KisOpenGLImageTextures::haveHDRTextureFormat(KoColorSpace *colorSpace)
{
#ifdef HAVE_GLEW
    KisOpenGL::makeContextCurrent();
    QString colorSpaceId = colorSpace->id();

    if (colorSpaceId == "RGBAF16HALF") {
        if (GLEW_ARB_texture_float) {
            return true;
        }
        if (GLEW_ATI_texture_float) {
            return true;
        }
    }
    if (colorSpaceId == "RGBAF32") {
        if (GLEW_ARB_texture_float) {
            return true;
        }
        if (GLEW_ATI_texture_float) {
            return true;
        }
    }
#endif
    return false;
}

void KisOpenGLImageTextures::setImageTextureFormat()
{
    m_imageTextureInternalFormat = GL_RGBA8;
    m_imageTextureType = GL_UNSIGNED_BYTE;

#ifdef HAVE_GLEW
    QString colorSpaceId = m_image->colorSpace()->id();
    m_usingHDRExposureProgram = false;

    kDebug(DBG_AREA_UI) << "Choosing texture format:\n";

    if (imageCanUseHDRExposureProgram(m_image)) {

        if (colorSpaceId == "RGBAF16HALF") {

            if (GLEW_ARB_texture_float) {
                m_imageTextureInternalFormat = GL_RGBA16F_ARB;
                kDebug(DBG_AREA_UI) << "Using ARB half\n";
            } else {
                Q_ASSERT(GLEW_ATI_texture_float);
                m_imageTextureInternalFormat = GL_RGBA_FLOAT16_ATI;
                kDebug(DBG_AREA_UI) << "Using ATI half\n";
            }

            if (GLEW_ARB_half_float_pixel) {
                kDebug(DBG_AREA_UI) << "Pixel type half\n";
                m_imageTextureType = GL_HALF_FLOAT_ARB;
            } else {
                kDebug(DBG_AREA_UI) << "Pixel type float\n";
                m_imageTextureType = GL_FLOAT;
            }

            m_usingHDRExposureProgram = true;

        } else if (colorSpaceId == "RGBAF32") {

            if (GLEW_ARB_texture_float) {
                m_imageTextureInternalFormat = GL_RGBA32F_ARB;
                kDebug(DBG_AREA_UI) << "Using ARB float\n";
            } else {
                Q_ASSERT(GLEW_ATI_texture_float);
                m_imageTextureInternalFormat = GL_RGBA_FLOAT32_ATI;
                kDebug(DBG_AREA_UI) << "Using ATI float\n";
            }

            m_imageTextureType = GL_FLOAT;
            m_usingHDRExposureProgram = true;
        }
    } else {
        kDebug(DBG_AREA_UI) << "Using unsigned byte\n";
    }
#endif
}

bool KisOpenGLImageTextures::imageCanUseHDRExposureProgram(KisImageSP image)
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
    if (!haveHDRTextureFormat(image->colorSpace())) {
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

