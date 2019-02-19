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
#ifndef KIS_OPENGL_IMAGE_TEXTURES_H_
#define KIS_OPENGL_IMAGE_TEXTURES_H_

#include <QVector>
#include <QMap>
#include <QOpenGLFunctions>

#include "kritaui_export.h"

#include "kis_shared.h"

#include "canvas/kis_update_info.h"
#include "opengl/kis_texture_tile.h"
#include "KisOpenGLUpdateInfoBuilder.h"

class KisOpenGLImageTextures;
typedef KisSharedPtr<KisOpenGLImageTextures> KisOpenGLImageTexturesSP;

class KoColorProfile;
class KisTextureTileUpdateInfoPoolCollection;
typedef QSharedPointer<KisTextureTileInfoPool> KisTextureTileInfoPoolSP;

class KisProofingConfiguration;
typedef QSharedPointer<KisProofingConfiguration> KisProofingConfigurationSP;

/**
 * A set of OpenGL textures that contains the projection of a KisImage.
 */
class KRITAUI_EXPORT KisOpenGLImageTextures : public KisShared
{
public:
    /**
     * Obtain a KisOpenGLImageTextures object for the given image.
     * @param image The image
     * @param monitorProfile The profile of the display device
     * @param renderingIntent The rendering intent
     * @param conversionFlags The color conversion flags
     */
    static KisOpenGLImageTexturesSP getImageTextures(KisImageWSP image,
                                                     const KoColorProfile *monitorProfile, KoColorConversionTransformation::Intent renderingIntent,
                                                     KoColorConversionTransformation::ConversionFlags conversionFlags);

    /**
     * Default constructor.
     */
    KisOpenGLImageTextures();

    /**
     * Destructor.
     */
    virtual ~KisOpenGLImageTextures();

    /**
     * \return the image associated with the textures
     */
    KisImageSP image() const;

    /**
     * Set the color profile of the display device.
     * @param monitorProfile The color profile of the display device
     * @param renderingIntent The rendering intent
     * @param conversionFlags The color conversion flags
     */
    void setMonitorProfile(const KoColorProfile *monitorProfile,
                           KoColorConversionTransformation::Intent renderingIntent,
                           KoColorConversionTransformation::ConversionFlags conversionFlags);

    /**
     * Update the textures when the color space of the image changes.
     * @return true when a full data refetch should be initiated by the caller
     */
    bool setImageColorSpace(const KoColorSpace *cs);

    /**
     * Complete initialization can only happen once an OpenGL context has been created.
     * @param f Pointer to OpenGL functions. They must already be initialized.
     */
    void initGL(QOpenGLFunctions *f);

    void setChannelFlags(const QBitArray &channelFlags);
    void setProofingConfig(KisProofingConfigurationSP);

    bool internalColorManagementActive() const;
    bool setInternalColorManagementActive(bool value);

    /**
     * The background checkers texture.
     */
    static const int BACKGROUND_TEXTURE_CHECK_SIZE = 32;
    static const int BACKGROUND_TEXTURE_SIZE = BACKGROUND_TEXTURE_CHECK_SIZE * 2;

    /**
     * Generate a background texture from the given QImage. This is used for the checker
     * pattern on which the image is rendered.
     */
    void generateCheckerTexture(const QImage & checkImage);
    GLuint checkerTexture();

    void updateConfig(bool useBuffer, int NumMipmapLevels);

public:
    inline QRect storedImageBounds() {
        return m_storedImageBounds;
    }

    inline int xToCol(int x) {
        return x / m_texturesInfo.effectiveWidth;
    }

    inline int yToRow(int y) {
        return y / m_texturesInfo.effectiveHeight;
    }

    inline KisTextureTile* getTextureTileCR(int col, int row) {
        if (m_initialized) {
            int tile = row * m_numCols + col;
            KIS_ASSERT_RECOVER_RETURN_VALUE(m_textureTiles.size() > tile, 0);
            return m_textureTiles[tile];
        }
        return 0;
    }

    inline qreal texelSize() const {
        Q_ASSERT(m_texturesInfo.width == m_texturesInfo.height);
        return 1.0 / m_texturesInfo.width;
    }

    KisOpenGLUpdateInfoSP updateCache(const QRect& rect, KisImageSP srcImage);
    KisOpenGLUpdateInfoSP updateCacheNoConversion(const QRect& rect);

    void recalculateCache(KisUpdateInfoSP info, bool blockMipmapRegeneration);

    void slotImageSizeChanged(qint32 w, qint32 h);

    KisOpenGLUpdateInfoBuilder& updateInfoBuilder();

protected:

    KisOpenGLImageTextures(KisImageWSP image, const KoColorProfile *monitorProfile,
                           KoColorConversionTransformation::Intent renderingIntent,
                           KoColorConversionTransformation::ConversionFlags conversionFlags);

    void recreateImageTextureTiles();

    void destroyImageTextureTiles();

    static bool imageCanShareTextures();

private:

    void getTextureSize(KisGLTexturesInfo *texturesInfo);

    void updateTextureFormat();
    KisOpenGLUpdateInfoSP updateCacheImpl(const QRect& rect, KisImageSP srcImage, bool convertColorSpace);

private:
    KisImageWSP m_image;
    QRect m_storedImageBounds;
    const KoColorProfile *m_monitorProfile;
    KoColorConversionTransformation::Intent m_renderingIntent;
    KoColorConversionTransformation::ConversionFlags m_conversionFlags;

    /**
     * Shows whether the internal color management should be enabled or not.
     * Please note that if you disable color management, *but* your image color
     * space will not be supported (non-RGB), then it will be enabled anyway.
     * And this valiable will hold the real state of affairs!
     */
    bool m_internalColorManagementActive;

    GLuint m_checkerTexture;

    KisGLTexturesInfo m_texturesInfo;
    int m_numCols;
    QVector<KisTextureTile*> m_textureTiles;

    QOpenGLFunctions *m_glFuncs;

    bool m_useOcio;
    bool m_initialized;

    KisOpenGLUpdateInfoBuilder m_updateInfoBuilder;

private:
    typedef QMap<KisImageWSP, KisOpenGLImageTextures*> ImageTexturesMap;
    static ImageTexturesMap imageTexturesMap;
};

#endif // KIS_OPENGL_IMAGE_TEXTURES_H_

