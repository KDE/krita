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
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLContext>

#include <QMessageBox>
#include <QApplication>
#include <QDesktopWidget>

#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>
#include <KoColorModelStandardIds.h>

#include "kis_image.h"
#include "kis_config.h"
#include "KisPart.h"

#ifdef HAVE_OPENEXR
#include <half.h>
#endif

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#ifndef GL_BGRA
#define GL_BGRA 0x80E1
#endif


KisOpenGLImageTextures::ImageTexturesMap KisOpenGLImageTextures::imageTexturesMap;

KisOpenGLImageTextures::KisOpenGLImageTextures()
    : m_image(0)
    , m_monitorProfile(0)
    , m_tilesDestinationColorSpace(0)
    , m_internalColorManagementActive(true)
    , m_checkerTexture(0)
    , m_glFuncs(0)
    , m_allChannelsSelected(true)
    , m_useOcio(false)
    , m_initialized(false)
{
    KisConfig cfg;
    m_renderingIntent = (KoColorConversionTransformation::Intent)cfg.monitorRenderIntent();

    m_conversionFlags = KoColorConversionTransformation::HighQuality;
    if (cfg.useBlackPointCompensation()) m_conversionFlags |= KoColorConversionTransformation::BlackpointCompensation;
    if (!cfg.allowLCMSOptimization()) m_conversionFlags |= KoColorConversionTransformation::NoOptimization;

    m_useOcio = cfg.useOcio();
}

KisOpenGLImageTextures::KisOpenGLImageTextures(KisImageWSP image,
                                               const KoColorProfile *monitorProfile,
                                               KoColorConversionTransformation::Intent renderingIntent,
                                               KoColorConversionTransformation::ConversionFlags conversionFlags)
    : m_image(image)
    , m_monitorProfile(monitorProfile)
    , m_renderingIntent(renderingIntent)
    , m_conversionFlags(conversionFlags)
    , m_tilesDestinationColorSpace(0)
    , m_internalColorManagementActive(true)
    , m_checkerTexture(0)
    , m_glFuncs(0)
    , m_allChannelsSelected(true)
    , m_useOcio(false)
    , m_initialized(false)
{
    Q_ASSERT(renderingIntent < 4);
}

void KisOpenGLImageTextures::initGL(QOpenGLFunctions *f) {
    m_glFuncs = f;

    getTextureSize(&m_texturesInfo);
    m_glFuncs->glGenTextures(1, &m_checkerTexture);
    createImageTextureTiles();

    KisOpenGLUpdateInfoSP info = updateCache(m_image->bounds());
    recalculateCache(info);
}

KisOpenGLImageTextures::~KisOpenGLImageTextures()
{
    ImageTexturesMap::iterator it = imageTexturesMap.find(m_image);
    if (it != imageTexturesMap.end()) {
        KisOpenGLImageTextures *textures = it.value();
        if (textures == this) {
            dbgUI << "Removing shared image context from map";
            imageTexturesMap.erase(it);
        }
    }

    destroyImageTextureTiles();
    m_glFuncs->glDeleteTextures(1, &m_checkerTexture);
}

KisImageSP KisOpenGLImageTextures::image() const
{
    return m_image;
}

bool KisOpenGLImageTextures::imageCanShareTextures()
{
    KisConfig cfg;
    if (cfg.useOcio()) return false;
    if (KisPart::instance()->mainwindowCount() == 1) return true;
    if (qApp->desktop()->screenCount() == 1) return true;
    for (int i = 1; i < qApp->desktop()->screenCount(); i++) {
        if (cfg.displayProfile(i) != cfg.displayProfile(i - 1)) {
            return false;
        }
    }
    return true;
}

KisOpenGLImageTexturesSP KisOpenGLImageTextures::getImageTextures(KisImageWSP image,
                                                                  const KoColorProfile *monitorProfile,
                                                                  KoColorConversionTransformation::Intent renderingIntent,
                                                                  KoColorConversionTransformation::ConversionFlags conversionFlags)
{
    if (imageCanShareTextures()) {
        ImageTexturesMap::iterator it = imageTexturesMap.find(image);

        if (it != imageTexturesMap.end()) {
            KisOpenGLImageTexturesSP textures = it.value();
            textures->setMonitorProfile(monitorProfile, renderingIntent, conversionFlags);

            return textures;
        } else {
            KisOpenGLImageTextures *imageTextures = new KisOpenGLImageTextures(image, monitorProfile, renderingIntent, conversionFlags);
            imageTexturesMap[image] = imageTextures;
            dbgUI << "Added shareable textures to map";

            return imageTextures;
        }
    } else {
        return new KisOpenGLImageTextures(image, monitorProfile, renderingIntent, conversionFlags);
    }
}

QRect KisOpenGLImageTextures::calculateTileRect(int col, int row) const
{
    return m_image->bounds() &
            QRect(col * m_texturesInfo.effectiveWidth,
                  row * m_texturesInfo.effectiveHeight,
                  m_texturesInfo.effectiveWidth,
                  m_texturesInfo.effectiveHeight);
}

void KisOpenGLImageTextures::createImageTextureTiles()
{
    KisConfig cfg;

    destroyImageTextureTiles();
    updateTextureFormat();

    m_storedImageBounds = m_image->bounds();
    const int lastCol = xToCol(m_image->width());
    const int lastRow = yToRow(m_image->height());
    if (lastCol == 0) {
      return;
    }

    m_numCols = lastCol + 1;

    // Default color is transparent black
    const int pixelSize = m_tilesDestinationColorSpace->pixelSize();
    QByteArray emptyTileData((m_texturesInfo.width) * (m_texturesInfo.height) * pixelSize, 0);

    KisConfig config;
    KisTextureTile::FilterMode mode = (KisTextureTile::FilterMode)config.openGLFilteringMode();

    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (ctx) {
    QOpenGLFunctions *f = ctx->functions();

    m_initialized = true;
    qDebug()  << "OpenGL: creating texture tiles of size" << m_texturesInfo.height << "x" << m_texturesInfo.width;

    for (int row = 0; row <= lastRow; row++) {
        for (int col = 0; col <= lastCol; col++) {
            QRect tileRect = calculateTileRect(col, row);

            KisTextureTile *tile = new KisTextureTile(tileRect,
                                                      &m_texturesInfo,
                                                      emptyTileData,
                                                      mode,
                                                      cfg.useOpenGLTextureBuffer(),
                                                      cfg.numMipmapLevels(),
                                                      f);
            m_textureTiles.append(tile);
        }
    }
    }
    else {
        qDebug() << "Tried to init texture tiles without a current OpenGL Context.";
    }
}

void KisOpenGLImageTextures::destroyImageTextureTiles()
{
    if(m_textureTiles.isEmpty()) return;

    foreach(KisTextureTile *tile, m_textureTiles) {
        delete tile;
    }
    m_textureTiles.clear();
    m_storedImageBounds = QRect();
}

KisOpenGLUpdateInfoSP KisOpenGLImageTextures::updateCache(const QRect& rect)
{
    return updateCacheImpl(rect, true);
}

KisOpenGLUpdateInfoSP KisOpenGLImageTextures::updateCacheNoConversion(const QRect& rect)
{
    return updateCacheImpl(rect, false);
}

KisOpenGLUpdateInfoSP KisOpenGLImageTextures::updateCacheImpl(const QRect& rect, bool convertColorSpace)
{
    const KoColorSpace *dstCS = m_tilesDestinationColorSpace;

    ConversionOptions options;

    if (convertColorSpace) {
        options = ConversionOptions(dstCS, m_renderingIntent, m_conversionFlags);
    }

    KisOpenGLUpdateInfoSP info = new KisOpenGLUpdateInfo(options);

    QRect updateRect = rect & m_image->bounds();
    if (updateRect.isEmpty() || !(m_initialized)) return info;

    /**
     * Why the rect is artificial? That's easy!
     * It does not represent any real piece of the image. It is
     * intentionally stretched to get through the overlappping
     * stripes of neutrality and poke neighbouring tiles.
     * Thanks to the rect we get the coordinates of all the tiles
     * involved into update process
     */

    QRect artificialRect = stretchRect(updateRect, m_texturesInfo.border);
    artificialRect &= m_image->bounds();

    int firstColumn = xToCol(artificialRect.left());
    int lastColumn = xToCol(artificialRect.right());
    int firstRow = yToRow(artificialRect.top());
    int lastRow = yToRow(artificialRect.bottom());

    QBitArray channelFlags; // empty by default

    if (m_channelFlags.size() != m_image->projection()->colorSpace()->channels().size()) {
        setChannelFlags(QBitArray());
    }
    if (!m_useOcio) { // Ocio does its own channel flipping
        if (!m_allChannelsSelected) { // and we do it only if necessary
            channelFlags = m_channelFlags;
        }
    }

    qint32 numItems = (lastColumn - firstColumn + 1) * (lastRow - firstRow + 1);
    info->tileList.reserve(numItems);

    const QRect bounds = m_image->bounds();
    const int levelOfDetail = m_image->currentLevelOfDetail();

    QRect alignedUpdateRect = updateRect;
    QRect alignedBounds = bounds;

    if (levelOfDetail) {
        alignedUpdateRect = KisLodTransform::alignedRect(alignedUpdateRect, levelOfDetail);
        alignedBounds = KisLodTransform::alignedRect(alignedBounds, levelOfDetail);
    }

    for (int col = firstColumn; col <= lastColumn; col++) {
        for (int row = firstRow; row <= lastRow; row++) {

            const QRect tileRect = calculateTileRect(col, row);
            const QRect tileTextureRect = stretchRect(tileRect, m_texturesInfo.border);

            QRect alignedTileTextureRect = levelOfDetail ?
                KisLodTransform::alignedRect(tileTextureRect, levelOfDetail) :
                tileTextureRect;

            KisTextureTileUpdateInfoSP tileInfo(
                new KisTextureTileUpdateInfo(col, row,
                                             alignedTileTextureRect,
                                             alignedUpdateRect,
                                             alignedBounds,
                                             levelOfDetail));
            // Don't update empty tiles
            if (tileInfo->valid()) {
                tileInfo->retrieveData(m_image, channelFlags, m_onlyOneChannelSelected, m_selectedChannelIndex);

                if (convertColorSpace) {
                    tileInfo->convertTo(dstCS, m_renderingIntent, m_conversionFlags);
                }

                info->tileList.append(tileInfo);
            }
            else {
                kWarning() << "Trying to create an empty tileinfo record" << col << row << tileTextureRect << updateRect << m_image->bounds();
            }
        }
    }

    info->assignDirtyImageRect(rect);
    return info;
}

void KisOpenGLImageTextures::recalculateCache(KisUpdateInfoSP info)
{
    if (!m_initialized) {
        qDebug() << "OpenGL: Tried to edit image texture cache before it was initialized.";
        return;
    }

    KisOpenGLUpdateInfoSP glInfo = dynamic_cast<KisOpenGLUpdateInfo*>(info.data());
    if(!glInfo) return;

    KisTextureTileUpdateInfoSP tileInfo;
    foreach(tileInfo, glInfo->tileList) {
        KisTextureTile *tile = getTextureTileCR(tileInfo->tileCol(), tileInfo->tileRow());
        KIS_ASSERT_RECOVER_RETURN(tile);

        tile->update(*tileInfo);
    }
}

void KisOpenGLImageTextures::generateCheckerTexture(const QImage &checkImage)
{

    if (m_glFuncs) {
        qDebug() << "Attaching checker texture" << checkerTexture();
        m_glFuncs->glBindTexture(GL_TEXTURE_2D, checkerTexture());

        m_glFuncs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        m_glFuncs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        m_glFuncs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        m_glFuncs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        m_glFuncs->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        QImage img = checkImage;
        if (checkImage.width() != BACKGROUND_TEXTURE_SIZE || checkImage.height() != BACKGROUND_TEXTURE_SIZE) {
            img = checkImage.scaled(BACKGROUND_TEXTURE_SIZE, BACKGROUND_TEXTURE_SIZE);
        }
        m_glFuncs->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, BACKGROUND_TEXTURE_SIZE, BACKGROUND_TEXTURE_SIZE,
    #if QT_VERSION >= 0x040700
                    0, GL_BGRA, GL_UNSIGNED_BYTE, img.constBits());
    #else
                    0, GL_BGRA, GL_UNSIGNED_BYTE, img.bits());
    #endif
    }
    else {
        qDebug() << "OpenGL: Tried to generate checker texture before OpenGL was initialized.";
    }

}

GLuint KisOpenGLImageTextures::checkerTexture()
{
    if (m_glFuncs) {
        if (m_checkerTexture == 0) {
            m_glFuncs->glGenTextures(1, &m_checkerTexture);
        }
        return m_checkerTexture;
    }
    else {
        return 0;
    }
}

void KisOpenGLImageTextures::updateConfig(bool useBuffer, int NumMipmapLevels)
{
    if(m_textureTiles.isEmpty()) return;

    foreach(KisTextureTile *tile, m_textureTiles) {
        tile->setUseBuffer(useBuffer);
        tile->setNumMipmapLevels(NumMipmapLevels);
    }
}

void KisOpenGLImageTextures::slotImageSizeChanged(qint32 /*w*/, qint32 /*h*/)
{
    createImageTextureTiles();
}

void KisOpenGLImageTextures::setMonitorProfile(const KoColorProfile *monitorProfile, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags)
{
    //qDebug() << "Setting monitor profile to" << monitorProfile->name() << renderingIntent << conversionFlags;
    m_monitorProfile = monitorProfile;
    m_renderingIntent = renderingIntent;
    m_conversionFlags = conversionFlags;

    createImageTextureTiles();
}

void KisOpenGLImageTextures::setChannelFlags(const QBitArray &channelFlags)
{
    m_channelFlags = channelFlags;
    int selectedChannels = 0;
    const KoColorSpace *projectionCs = m_image->projection()->colorSpace();
    QList<KoChannelInfo*> channelInfo = projectionCs->channels();

    if (m_channelFlags.size() != channelInfo.size()) {
        m_channelFlags = QBitArray();
    }

    for (int i = 0; i < m_channelFlags.size(); ++i) {
        if (m_channelFlags.testBit(i) && channelInfo[i]->channelType() == KoChannelInfo::COLOR) {
            selectedChannels++;
            m_selectedChannelIndex = i;
        }
    }
    m_allChannelsSelected = (selectedChannels == m_channelFlags.size());
    m_onlyOneChannelSelected = (selectedChannels == 1);
}

void KisOpenGLImageTextures::getTextureSize(KisGLTexturesInfo *texturesInfo)
{
    KisConfig cfg;

    const GLint preferredTextureSize = cfg.openGLTextureSize();

    GLint maxTextureSize;
    if (m_glFuncs) {
        m_glFuncs->glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    }
    else {
        qDebug() << "OpenGL: Tried to read texture size before OpenGL was initialized.";
        maxTextureSize = GL_MAX_TEXTURE_SIZE;
    }

    texturesInfo->width = qMin(preferredTextureSize, maxTextureSize);
    texturesInfo->height = qMin(preferredTextureSize, maxTextureSize);

    texturesInfo->border = cfg.textureOverlapBorder();

    texturesInfo->effectiveWidth = texturesInfo->width - 2 * texturesInfo->border;
    texturesInfo->effectiveHeight = texturesInfo->height - 2 * texturesInfo->border;
}

bool KisOpenGLImageTextures::internalColorManagementActive() const
{
    return m_internalColorManagementActive;
}

bool KisOpenGLImageTextures::setInternalColorManagementActive(bool value)
{
    bool needsFinalRegeneration = m_internalColorManagementActive != value;


    if (needsFinalRegeneration) {
        m_internalColorManagementActive = value;
        createImageTextureTiles();

        // at this point the value of m_internalColorManagementActive might
        // have been forcely reverted to 'false' in case of some problems
    }

    return needsFinalRegeneration;
}

void KisOpenGLImageTextures::updateTextureFormat()
{
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (!(m_image && ctx)) return;

    m_texturesInfo.internalFormat = GL_RGBA8;
    m_texturesInfo.type = GL_UNSIGNED_BYTE;
    m_texturesInfo.format = GL_BGRA;

    KoID colorModelId = m_image->colorSpace()->colorModelId();
    KoID colorDepthId = m_image->colorSpace()->colorDepthId();

    KoID destinationColorModelId = RGBAColorModelID;
    KoID destinationColorDepthId = Integer8BitsColorDepthID;

    dbgUI << "Choosing texture format:";

    if (colorModelId == RGBAColorModelID) {
        if (colorDepthId == Float16BitsColorDepthID) {

            if (ctx->hasExtension("GL_ARB_texture_float")) {
                m_texturesInfo.internalFormat = GL_RGBA16F_ARB;
                dbgUI << "Using ARB half";
            }
            else if (ctx->hasExtension("GL_ATI_texture_float")) {
                m_texturesInfo.internalFormat = GL_RGBA_FLOAT16_ATI;
                dbgUI << "Using ATI half";
            }

            bool haveBuiltInOpenExr = false;
#ifdef HAVE_OPENEXR
            haveBuiltInOpenExr = true;
#endif

            if (haveBuiltInOpenExr && ctx->hasExtension("GL_ARB_half_float_pixel")) {
                m_texturesInfo.type = GL_HALF_FLOAT_ARB;
                destinationColorDepthId = Float16BitsColorDepthID;
                dbgUI << "Pixel type half";
            } else {
                m_texturesInfo.type = GL_FLOAT;
                destinationColorDepthId = Float32BitsColorDepthID;
                dbgUI << "Pixel type float";
            }
            m_texturesInfo.format = GL_RGBA;
        }
        else if (colorDepthId == Float32BitsColorDepthID) {
            if (ctx->hasExtension("GL_ARB_texture_float")) {
                m_texturesInfo.internalFormat = GL_RGBA32F_ARB;
                dbgUI << "Using ARB float";
            } else if (ctx->hasExtension("GL_ATI_texture_float")) {
                m_texturesInfo.internalFormat = GL_RGBA_FLOAT32_ATI;
                dbgUI << "Using ATI float";
            }

            m_texturesInfo.type = GL_FLOAT;
            m_texturesInfo.format = GL_RGBA;
            destinationColorDepthId = Float32BitsColorDepthID;
        }
        else if (colorDepthId == Integer16BitsColorDepthID) {
            m_texturesInfo.internalFormat = GL_RGBA16;
            m_texturesInfo.type = GL_UNSIGNED_SHORT;
            m_texturesInfo.format = GL_BGRA;
            destinationColorDepthId = Integer16BitsColorDepthID;
            dbgUI << "Using 16 bits rgba";
        }
    }
    else {
        // We will convert the colorspace to 16 bits rgba, instead of 8 bits
        if (colorDepthId == Integer16BitsColorDepthID) {
            m_texturesInfo.internalFormat = GL_RGBA16;
            m_texturesInfo.type = GL_UNSIGNED_SHORT;
            m_texturesInfo.format = GL_BGRA;
            destinationColorDepthId = Integer16BitsColorDepthID;
            dbgUI << "Using conversion to 16 bits rgba";
        }
    }

    if (!m_internalColorManagementActive &&
        colorModelId != destinationColorModelId) {

        KisConfig cfg;
        KisConfig::OcioColorManagementMode cm = cfg.ocioColorManagementMode();

        if (cm != KisConfig::INTERNAL) {
            QMessageBox::critical(0,
                                  i18nc("@title:window", "Krita"),
                                  i18n("You enabled OpenColorIO based color management, but your image is not an RGB image.\n"
                                       "OpenColorIO-based color management only works with RGB images.\n"
                                       "Please check the settings in the LUT docker."
                                       "OpenColorIO will now be deactivated."));
        }

        qWarning() << "WARNING: Internal color management was forcely enabled";
        qWarning() << "Color Management Mode: " << cm;
        qWarning() << ppVar(m_image->colorSpace());
        qWarning() << ppVar(destinationColorModelId);
        qWarning() << ppVar(destinationColorDepthId);

        cfg.setOcioColorManagementMode(KisConfig::INTERNAL);
        m_internalColorManagementActive = true;
    }

    const KoColorProfile *profile =
        m_internalColorManagementActive ||
        colorModelId != destinationColorModelId ?
        m_monitorProfile : m_image->colorSpace()->profile();

    /**
     * TODO: add an optimization so that the tile->convertTo() method
     *       would not be called when not needed (DK)
     */

    m_tilesDestinationColorSpace =
        KoColorSpaceRegistry::instance()->colorSpace(destinationColorModelId.id(),
                                                     destinationColorDepthId.id(),
                                                     profile);
}


#endif // HAVE_OPENGL

