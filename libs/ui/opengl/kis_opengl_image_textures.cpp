/*
 *  SPDX-FileCopyrightText: 2005-2007 Adrian Page <adrian@pagenet.plus.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "opengl/kis_opengl_image_textures.h"

#ifdef QT_OPENGL_ES_2
#include <qopengl.h>
#endif

#ifndef QT_OPENGL_ES_2
#include <QOpenGLFunctions>
#endif
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
#include "KisOpenGLModeProber.h"
#include "kis_fixed_paint_device.h"

#ifdef HAVE_OPENEXR
#include <half.h>
#endif

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#ifndef GL_BGRA
#define GL_BGRA 0x80E1
#endif

// GL_EXT_texture_format_BGRA8888
#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT 0x80E1
#endif
#ifndef GL_BGRA8_EXT
#define GL_BGRA8_EXT 0x93A1
#endif


KisOpenGLImageTextures::ImageTexturesMap KisOpenGLImageTextures::imageTexturesMap;

KisOpenGLImageTextures::KisOpenGLImageTextures()
    : m_image(0)
    , m_monitorProfile(0)
    , m_internalColorManagementActive(true)
    , m_glFuncs(0)
    , m_useOcio(false)
    , m_initialized(false)
{
    KisConfig cfg(true);
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
    , m_internalColorManagementActive(true)
    , m_glFuncs(0)
    , m_useOcio(false)
    , m_initialized(false)
{
    Q_ASSERT(renderingIntent < 4);
}

void KisOpenGLImageTextures::initGL(QOpenGLFunctions *f)
{
    if (f) {
        m_glFuncs = f;
    } else {
        errUI << "Tried to create OpenGLImageTextures with uninitialized QOpenGLFunctions";
    }

    getTextureSize(&m_texturesInfo);

    // we use local static object for creating pools shared among
    // different images
    static KisTextureTileInfoPoolRegistry s_poolRegistry;
    m_updateInfoBuilder.setTextureInfoPool(s_poolRegistry.getPool(m_texturesInfo.width, m_texturesInfo.height));

    m_checkerTexture = GLuint();
    m_glFuncs->glGenTextures(1, &(*m_checkerTexture));
    recreateImageTextureTiles();

    KisOpenGLUpdateInfoSP info = updateCache(m_image->bounds(), m_image);
    recalculateCache(info, false);
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
    if (m_checkerTexture) {
        m_glFuncs->glDeleteTextures(1, &(*m_checkerTexture));
    }
}

KisImageSP KisOpenGLImageTextures::image() const
{
    return m_image;
}

bool KisOpenGLImageTextures::imageCanShareTextures()
{
    KisConfig cfg(true);
    if (cfg.useOcio()) return false;
    if (KisPart::instance()->mainwindowCount() == 1) return true;
    if (QGuiApplication::screens().count() == 1) return true;
    for (int i = 1; i < QGuiApplication::screens().count(); i++) {
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
    // Disabled until we figure out why we're deleting the shared textures on closing the second view on a single image
    if (false && imageCanShareTextures()) {
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

void KisOpenGLImageTextures::recreateImageTextureTiles()
{

    destroyImageTextureTiles();
    updateTextureFormat();

    const KoColorSpace *tilesDestinationColorSpace =
        m_updateInfoBuilder.destinationColorSpace();

    if (!tilesDestinationColorSpace) {
        qDebug() << "No destination colorspace!!!!";
        return;
    }


    m_storedImageBounds = m_image->bounds();
    const int lastCol = xToCol(m_image->width());
    const int lastRow = yToRow(m_image->height());

    m_numCols = lastCol + 1;

    // Default color is transparent black
    const int pixelSize = tilesDestinationColorSpace->pixelSize();
    QByteArray emptyTileData((m_texturesInfo.width) * (m_texturesInfo.height) * pixelSize, 0);

    KisConfig config(true);
    KisOpenGL::FilterMode mode = (KisOpenGL::FilterMode)config.openGLFilteringMode();

    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (ctx) {
        QOpenGLFunctions *f = ctx->functions();

        m_initialized = true;
        dbgUI  << "OpenGL: creating texture tiles of size" << m_texturesInfo.height << "x" << m_texturesInfo.width;

        m_textureTiles.reserve((lastRow+1)*m_numCols);
        for (int row = 0; row <= lastRow; row++) {
            for (int col = 0; col <= lastCol; col++) {
                QRect tileRect = m_updateInfoBuilder.calculateEffectiveTileRect(col, row, m_image->bounds());

                KisTextureTile *tile = new KisTextureTile(tileRect,
                                                          &m_texturesInfo,
                                                          emptyTileData,
                                                          mode,
                                                          config.useOpenGLTextureBuffer(),
                                                          config.numMipmapLevels(),
                                                          f);
                m_textureTiles.append(tile);
            }
        }
    }
    else {
        dbgUI << "Tried to init texture tiles without a current OpenGL Context.";
    }
}

void KisOpenGLImageTextures::destroyImageTextureTiles()
{
    if (m_textureTiles.isEmpty()) return;

    Q_FOREACH (KisTextureTile *tile, m_textureTiles) {
        delete tile;
    }
    m_textureTiles.clear();
    m_storedImageBounds = QRect();
}

KisOpenGLUpdateInfoSP KisOpenGLImageTextures::updateCache(const QRect& rect, KisImageSP srcImage)
{
    return updateCacheImpl(rect, srcImage, true);
}

KisOpenGLUpdateInfoSP KisOpenGLImageTextures::updateCacheNoConversion(const QRect& rect)
{
    return updateCacheImpl(rect, m_image, false);
}

// TODO: add sanity checks about the conformance of the passed srcImage!
KisOpenGLUpdateInfoSP KisOpenGLImageTextures::updateCacheImpl(const QRect& rect, KisImageSP srcImage, bool convertColorSpace)
{
    if (!m_initialized) return new KisOpenGLUpdateInfo();
    return m_updateInfoBuilder.buildUpdateInfo(rect, srcImage, convertColorSpace);
}

void KisOpenGLImageTextures::recalculateCache(KisUpdateInfoSP info, bool blockMipmapRegeneration)
{
    if (!m_initialized) {
        dbgUI << "OpenGL: Tried to edit image texture cache before it was initialized.";
        return;
    }

    KisOpenGLUpdateInfoSP glInfo = dynamic_cast<KisOpenGLUpdateInfo*>(info.data());
    if(!glInfo) return;

    KisTextureTileUpdateInfoSP tileInfo;
    Q_FOREACH (tileInfo, glInfo->tileList) {
        KisTextureTile *tile = getTextureTileCR(tileInfo->tileCol(), tileInfo->tileRow());
        KIS_ASSERT_RECOVER_RETURN(tile);

        tile->update(*tileInfo, blockMipmapRegeneration);
    }
}

void KisOpenGLImageTextures::generateCheckerTexture(const QImage &checkImage)
{
    if (!m_initialized) {
        return;
    }

    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (ctx) {
        QOpenGLFunctions *f = ctx->functions();
        dbgUI << "Attaching checker texture" << checkerTexture();
        f->glBindTexture(GL_TEXTURE_2D, checkerTexture());

        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        f->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        QImage img = checkImage;
        if (checkImage.width() != BACKGROUND_TEXTURE_SIZE || checkImage.height() != BACKGROUND_TEXTURE_SIZE) {
            img = checkImage.scaled(BACKGROUND_TEXTURE_SIZE, BACKGROUND_TEXTURE_SIZE);
        }

        // convert from sRGB to display format, potentially HDR
        const KoColorSpace *temporaryColorSpace = KoColorSpaceRegistry::instance()->rgb8();
        const KoColorSpace *finalColorSpace =
               KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                            m_updateInfoBuilder.destinationColorSpace()->colorDepthId().id(),
                                                            m_monitorProfile);

        KisFixedPaintDevice checkers(temporaryColorSpace);
        checkers.convertFromQImage(img, temporaryColorSpace->profile()->name());
        checkers.convertTo(finalColorSpace);

        KIS_ASSERT(checkers.bounds().width() == BACKGROUND_TEXTURE_SIZE);
        KIS_ASSERT(checkers.bounds().height() == BACKGROUND_TEXTURE_SIZE);

        GLint format = m_texturesInfo.format;
        GLint internalFormat = m_texturesInfo.internalFormat;
        GLint type = m_texturesInfo.type;

        f->glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, BACKGROUND_TEXTURE_SIZE, BACKGROUND_TEXTURE_SIZE,
                        0, format, type, checkers.data());
    }
    else {
        dbgUI << "OpenGL: Tried to generate checker texture before OpenGL was initialized.";
    }

}

GLuint KisOpenGLImageTextures::checkerTexture()
{
    if (m_glFuncs) {
        if (!m_checkerTexture) {
            m_checkerTexture = GLuint();
            m_glFuncs->glGenTextures(1, &(*m_checkerTexture));
        }
        return *m_checkerTexture;
    }
    else {
        dbgUI << "Tried to access checker texture before OpenGL was initialized";
        return 0;
    }
}

void KisOpenGLImageTextures::updateConfig(bool useBuffer, int NumMipmapLevels)
{
    if(m_textureTiles.isEmpty()) return;

    Q_FOREACH (KisTextureTile *tile, m_textureTiles) {
        tile->setUseBuffer(useBuffer);
        tile->setNumMipmapLevels(NumMipmapLevels);
    }
}

void KisOpenGLImageTextures::testingForceInitialized()
{
    m_initialized = true;
    m_updateInfoBuilder.setTextureInfoPool(toQShared(new KisTextureTileInfoPool(256, 256)));

    ConversionOptions options;
    options.m_destinationColorSpace = KoColorSpaceRegistry::instance()->rgb8();
    options.m_conversionFlags = KoColorConversionTransformation::internalConversionFlags();
    options.m_renderingIntent = KoColorConversionTransformation::internalRenderingIntent();
    options.m_needsConversion = false;
    m_updateInfoBuilder.setConversionOptions(options);

    m_updateInfoBuilder.setTextureBorder(4);
    m_updateInfoBuilder.setEffectiveTextureSize(QSize(248, 248));
}

void KisOpenGLImageTextures::slotImageSizeChanged(qint32 /*w*/, qint32 /*h*/)
{
    recreateImageTextureTiles();
}

KisOpenGLUpdateInfoBuilder &KisOpenGLImageTextures::updateInfoBuilder()
{
    return m_updateInfoBuilder;
}

const KoColorProfile *KisOpenGLImageTextures::monitorProfile()
{
    return m_monitorProfile;
}

void KisOpenGLImageTextures::setMonitorProfile(const KoColorProfile *monitorProfile, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags)
{
    //dbgUI << "Setting monitor profile to" << monitorProfile->name() << renderingIntent << conversionFlags;
    m_monitorProfile = monitorProfile;
    m_renderingIntent = renderingIntent;
    m_conversionFlags = conversionFlags;

    recreateImageTextureTiles();
}

bool KisOpenGLImageTextures::setImageColorSpace(const KoColorSpace *cs)
{
    Q_UNUSED(cs);
    // TODO: implement lazy update: do not re-upload textures when not needed

    recreateImageTextureTiles();
    return true;
}

void KisOpenGLImageTextures::setChannelFlags(const QBitArray &channelFlags)
{
    QBitArray targetChannelFlags = channelFlags;
    int selectedChannels = 0;
    const KoColorSpace *projectionCs = m_image->projection()->colorSpace();
    QList<KoChannelInfo*> channelInfo = projectionCs->channels();

    if (targetChannelFlags.size() != channelInfo.size()) {
        targetChannelFlags = QBitArray();
    }

    int selectedChannelIndex = -1;

    for (int i = 0; i < targetChannelFlags.size(); ++i) {
        if (targetChannelFlags.testBit(i) && channelInfo[i]->channelType() == KoChannelInfo::COLOR) {
            selectedChannels++;
            selectedChannelIndex = i;
        }
    }
    const bool allChannelsSelected = (selectedChannels == targetChannelFlags.size());
    const bool onlyOneChannelSelected = (selectedChannels == 1);

    // OCIO has its own channel swizzling
    if (allChannelsSelected || m_useOcio) {
        m_updateInfoBuilder.setChannelFlags(QBitArray(), false, -1);
    } else {
        m_updateInfoBuilder.setChannelFlags(targetChannelFlags, onlyOneChannelSelected, selectedChannelIndex);
    }
}

void KisOpenGLImageTextures::setProofingConfig(KisProofingConfigurationSP proofingConfig)
{
    m_updateInfoBuilder.setProofingConfig(proofingConfig);
}

void KisOpenGLImageTextures::getTextureSize(KisGLTexturesInfo *texturesInfo)
{
    KisConfig cfg(true);

    const GLint preferredTextureSize = cfg.openGLTextureSize();

    GLint maxTextureSize;
    if (m_glFuncs) {
        m_glFuncs->glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    }
    else {
        dbgUI << "OpenGL: Tried to read texture size before OpenGL was initialized.";
        maxTextureSize = GL_MAX_TEXTURE_SIZE;
    }

    texturesInfo->width = qMin(preferredTextureSize, maxTextureSize);
    texturesInfo->height = qMin(preferredTextureSize, maxTextureSize);

    texturesInfo->border = cfg.textureOverlapBorder();

    texturesInfo->effectiveWidth = texturesInfo->width - 2 * texturesInfo->border;
    texturesInfo->effectiveHeight = texturesInfo->height - 2 * texturesInfo->border;

    m_updateInfoBuilder.setTextureBorder(texturesInfo->border);
    m_updateInfoBuilder.setEffectiveTextureSize(
        QSize(texturesInfo->effectiveWidth, texturesInfo->effectiveHeight));
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
        recreateImageTextureTiles();

        // at this point the value of m_internalColorManagementActive might
        // have been forcely reverted to 'false' in case of some problems
    }

    return needsFinalRegeneration;
}

namespace {
void initializeRGBA16FTextures(QOpenGLContext *ctx, KisGLTexturesInfo &texturesInfo, KoID &destinationColorDepthId)
{
    if (KisOpenGL::hasOpenGLES() || KisOpenGL::hasOpenGL3()) {
        texturesInfo.internalFormat = GL_RGBA16F;
        dbgUI << "Using half (GLES or GL3)";
    } else if (ctx->hasExtension("GL_ARB_texture_float")) {
#ifndef QT_OPENGL_ES_2
        texturesInfo.internalFormat = GL_RGBA16F_ARB;
        dbgUI << "Using ARB half";
    }
    else if (ctx->hasExtension("GL_ATI_texture_float")) {
        texturesInfo.internalFormat = GL_RGBA_FLOAT16_ATI;
        dbgUI << "Using ATI half";
#else
        KIS_ASSERT_X(false, "initializeRGBA16FTextures",
                "Unexpected KisOpenGL::hasOpenGLES and \
                 KisOpenGL::hasOpenGL3 returned false");
#endif
    }

    bool haveBuiltInOpenExr = false;
#ifdef HAVE_OPENEXR
    haveBuiltInOpenExr = true;
#endif

    if (haveBuiltInOpenExr && (KisOpenGL::hasOpenGLES() || KisOpenGL::hasOpenGL3())) {
        texturesInfo.type = GL_HALF_FLOAT;
        destinationColorDepthId = Float16BitsColorDepthID;
        dbgUI << "Pixel type half (GLES or GL3)";
    } else if (haveBuiltInOpenExr && ctx->hasExtension("GL_ARB_half_float_pixel")) {
#ifndef QT_OPENGL_ES_2
        texturesInfo.type = GL_HALF_FLOAT_ARB;
        destinationColorDepthId = Float16BitsColorDepthID;
        dbgUI << "Pixel type half";
    } else {
        texturesInfo.type = GL_FLOAT;
        destinationColorDepthId = Float32BitsColorDepthID;
        dbgUI << "Pixel type float";
#else
        KIS_ASSERT_X(false, "KisOpenGLCanvas2::paintToolOutline",
                "Unexpected KisOpenGL::hasOpenGLES and \
                 KisOpenGL::hasOpenGL3 returned false");
#endif
    }
    texturesInfo.format = GL_RGBA;
}
}

void KisOpenGLImageTextures::updateTextureFormat()
{
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (!(m_image && ctx)) return;

    if (!KisOpenGL::hasOpenGLES()) {
#ifndef QT_OPENGL_ES_2
        m_texturesInfo.internalFormat = GL_RGBA8;
        m_texturesInfo.type = GL_UNSIGNED_BYTE;
        m_texturesInfo.format = GL_BGRA;
#else
        KIS_ASSERT_X(false, "KisOpenGLImageTextures::updateTextureFormat",
                "Unexpected KisOpenGL::hasOpenGLES returned false");
#endif
    } else {
#ifdef QT_OPENGL_ES_2
        m_texturesInfo.internalFormat = GL_RGBA8;
        m_texturesInfo.type = GL_UNSIGNED_BYTE;
        m_texturesInfo.format = GL_RGBA;
#else
        m_texturesInfo.internalFormat = GL_BGRA8_EXT;
        m_texturesInfo.type = GL_UNSIGNED_BYTE;
        m_texturesInfo.format = GL_BGRA_EXT;
        if(!ctx->hasExtension(QByteArrayLiteral("GL_EXT_texture_format_BGRA8888"))) {
            // The red and blue channels are swapped, but it will be re-swapped
            // by texture swizzle mask set in KisTextureTile::setTextureParameters
            m_texturesInfo.internalFormat = GL_RGBA8;
            m_texturesInfo.type = GL_UNSIGNED_BYTE;
            m_texturesInfo.format = GL_RGBA;
        }
#endif
    }

    const bool useHDRMode = KisOpenGLModeProber::instance()->useHDRMode();
    const KoID colorModelId = m_image->colorSpace()->colorModelId();
    const KoID colorDepthId = useHDRMode ? Float16BitsColorDepthID : m_image->colorSpace()->colorDepthId();

    KoID destinationColorModelId = RGBAColorModelID;
    KoID destinationColorDepthId = Integer8BitsColorDepthID;

    dbgUI << "Choosing texture format:";

    if (colorModelId == RGBAColorModelID) {
        if (colorDepthId == Float16BitsColorDepthID) {
            initializeRGBA16FTextures(ctx, m_texturesInfo, destinationColorDepthId);
        }
        else if (colorDepthId == Float32BitsColorDepthID) {
            if (KisOpenGL::hasOpenGLES() || KisOpenGL::hasOpenGL3()) {
                m_texturesInfo.internalFormat = GL_RGBA32F;
                dbgUI << "Using float (GLES or GL3)";
            } else if (ctx->hasExtension("GL_ARB_texture_float")) {
#ifndef QT_OPENGL_ES_2
                m_texturesInfo.internalFormat = GL_RGBA32F_ARB;
                dbgUI << "Using ARB float";
            } else if (ctx->hasExtension("GL_ATI_texture_float")) {
                m_texturesInfo.internalFormat = GL_RGBA_FLOAT32_ATI;
                dbgUI << "Using ATI float";
#else
        KIS_ASSERT_X(false, "KisOpenGLCanvas2::updateTextureFormat",
                "Unexpected KisOpenGL::hasOpenGLES and \
                 KisOpenGL::hasOpenGL3 returned false");
#endif
            }

            m_texturesInfo.type = GL_FLOAT;
            m_texturesInfo.format = GL_RGBA;
            destinationColorDepthId = Float32BitsColorDepthID;
        }
        else if (colorDepthId == Integer16BitsColorDepthID) {
#ifndef QT_OPENGL_ES_2
            if (!KisOpenGL::hasOpenGLES()) {
                m_texturesInfo.internalFormat = GL_RGBA16;
                m_texturesInfo.type = GL_UNSIGNED_SHORT;
                m_texturesInfo.format = GL_BGRA;
                destinationColorDepthId = Integer16BitsColorDepthID;
                dbgUI << "Using 16 bits rgba";
            }
#endif
            // TODO: for ANGLE, see if we can convert to 16f to support 10-bit display
        }
    }
    else {
        // We will convert the colorspace to 16 bits rgba, instead of 8 bits
        if (colorDepthId == Integer16BitsColorDepthID && !KisOpenGL::hasOpenGLES()) {
#ifndef QT_OPENGL_ES_2
            m_texturesInfo.internalFormat = GL_RGBA16;
            m_texturesInfo.type = GL_UNSIGNED_SHORT;
            m_texturesInfo.format = GL_BGRA;
            destinationColorDepthId = Integer16BitsColorDepthID;
            dbgUI << "Using conversion to 16 bits rgba";
#else
            KIS_ASSERT_X(false, "KisOpenGLCanvas2::updateTextureFormat",
                    "Unexpected KisOpenGL::hasOpenGLES returned false");
#endif
        } else if (colorDepthId == Float16BitsColorDepthID && KisOpenGL::hasOpenGLES()) {
            // TODO: try removing opengl es limit
            initializeRGBA16FTextures(ctx, m_texturesInfo, destinationColorDepthId);
        }
        // TODO: for ANGLE, see if we can convert to 16f to support 10-bit display
    }

    if (!m_internalColorManagementActive &&
            colorModelId != destinationColorModelId) {

        KisConfig cfg(false);
        KisConfig::OcioColorManagementMode cm = cfg.ocioColorManagementMode();

        if (cm != KisConfig::INTERNAL) {
            emit sigShowFloatingMessage(
                i18n("OpenColorIO is disabled: image color space is not supported"), 5000, true);
        }

        warnUI << "WARNING: Internal color management was forcibly enabled";
        warnUI << "Color Management Mode: " << cm;
        warnUI << ppVar(m_image->colorSpace());
        warnUI << ppVar(destinationColorModelId);
        warnUI << ppVar(destinationColorDepthId);

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

    const KoColorSpace *tilesDestinationColorSpace =
            KoColorSpaceRegistry::instance()->colorSpace(destinationColorModelId.id(),
                                                         destinationColorDepthId.id(),
                                                         profile);

    m_updateInfoBuilder.setConversionOptions(
        ConversionOptions(tilesDestinationColorSpace,
                          m_renderingIntent,
                          m_conversionFlags));
}

