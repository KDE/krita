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

#include <opengl/kis_opengl.h>
#include "canvas/kis_update_info.h"
#include "opengl/kis_texture_tile_update_info.h"
#include "opengl/kis_texture_tile.h"


#ifdef HAVE_OPENGL

#include <QObject>
#include <QVector>
#include <QMap>

#include "krita_export.h"

#include "kis_shared.h"


#ifdef HAVE_GLEW
class KisOpenGLHDRExposureProgram;
#endif

class KisOpenGLImageTextures;
typedef KisSharedPtr<KisOpenGLImageTextures> KisOpenGLImageTexturesSP;

class KoColorSpace;
class KoColorProfile;
class KisDisplayFilter;

/**
 * A set of OpenGL textures that contains the projection of a KisImage.
 */
class KRITAUI_EXPORT KisOpenGLImageTextures : public QObject, public KisShared
{
    Q_OBJECT

public:
    /**
     * Obtain a KisOpenGLImageTextures object for the given image.
     * @param image The image
     * @param monitorProfile The profile of the display device
     */
    static KisOpenGLImageTexturesSP getImageTextures(KisImageWSP image,
                                                     KoColorProfile *monitorProfile, KoColorConversionTransformation::Intent renderingIntent,
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
     * Set the color profile of the display device.
     * @param profile The color profile of the display device
     */
    void setMonitorProfile(const KoColorProfile *monitorProfile,
                           KoColorConversionTransformation::Intent renderingIntent,
                           KoColorConversionTransformation::ConversionFlags conversionFlags);

    /**
     * set the (ocio) display filter.
     */
    void setDisplayFilter(KisDisplayFilter *displayFilter);

    /**
     * Generate a background texture from the given QImage. This is used for the checker
     * pattern on which the image is rendered.
     */
    void generateBackgroundTexture(const QImage & checkImage);

    /**
     * The background texture.
     */
    GLuint backgroundTexture() const;

    static const int BACKGROUND_TEXTURE_CHECK_SIZE = 32;
    static const int BACKGROUND_TEXTURE_SIZE = BACKGROUND_TEXTURE_CHECK_SIZE * 2;

    /**
     * Activate the high dynamic range image program. Call this before rendering
     * the image textures if the image has high dynamic range.
     */
    void activateHDRExposureProgram();

    /**
     * Detivate the high dynamic range image program.
     */
    void deactivateHDRExposureProgram();

    /**
     * Returns true if the textures are to be rendered using the high dynamic
     * range image program.
     */
    bool usingHDRExposureProgram() const;

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
        return m_textureTiles[row * m_numCols + col];
    }

    inline KisTextureTile* getTextureTile(int x, int y) {
        return getTextureTileCR(xToCol(x), yToRow(y));;
    }

public slots:
    KisOpenGLUpdateInfoSP updateCache(const QRect& rect);
    void recalculateCache(KisUpdateInfoSP info);

    void slotImageSizeChanged(qint32 w, qint32 h);

protected:
    KisOpenGLImageTextures(KisImageWSP image, KoColorProfile *monitorProfile,
                           KoColorConversionTransformation::Intent renderingIntent,
                           KoColorConversionTransformation::ConversionFlags conversionFlags);

    void createImageTextureTiles();
    void destroyImageTextureTiles();

    static bool imageCanUseHDRExposureProgram(KisImageWSP image);
    static bool imageCanShareTextures(KisImageWSP image);

private:
    QRect calculateTileRect(int col, int row) const;
    static void getTextureSize(KisGLTexturesInfo *texturesInfo);
    void updateTextureFormat();

private:
    KisImageWSP m_image;
    QRect m_storedImageBounds;
    const KoColorProfile *m_monitorProfile;
    KoColorConversionTransformation::Intent m_renderingIntent;
    KoColorConversionTransformation::ConversionFlags m_conversionFlags;
    GLuint m_backgroundTexture;

    KisGLTexturesInfo m_texturesInfo;
    int m_numCols;
    QVector<KisTextureTile*> m_textureTiles;

#ifdef HAVE_GLEW
    bool m_usingHDRExposureProgram;
#endif

    KisDisplayFilter *m_displayFilter;

private:
    typedef QMap<KisImageWSP, KisOpenGLImageTextures*> ImageTexturesMap;
    static ImageTexturesMap imageTexturesMap;
};

#endif // HAVE_OPENGL

#endif // KIS_OPENGL_IMAGE_TEXTURES_H_

