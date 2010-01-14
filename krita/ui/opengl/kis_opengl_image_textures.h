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

#ifdef HAVE_OPENGL

#include <map>

#include <QGLWidget>
#include <QImage>
#include <QObject>
#include <QVector>

#include "krita_export.h"

#include "kis_shared.h"
#include "kis_types.h"


#ifdef HAVE_GLEW
class KisOpenGLHDRExposureProgram;
#endif

class KisOpenGLImageTextures;
typedef KisSharedPtr<KisOpenGLImageTextures> KisOpenGLImageTexturesSP;

class KoColorSpace;
class KoColorProfile;

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
    static KisOpenGLImageTexturesSP getImageTextures(KisImageWSP image, KoColorProfile *monitorProfile);

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
    void setMonitorProfile(KoColorProfile *profile);

    /**
     * Set the exposure level used to display high dynamic range images. Typical values
     * are between -10 and 10.
     * @param exposure The exposure level
     */
    void setHDRExposure(float exposure);

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
     * Get the image texture containing the point (pixelX, pixelY).
     * @param pixelX The x coordinate of the point to check
     * @param pixelY The y coordinate of the point to check
     */
    GLuint imageTextureTile(int pixelX, int pixelY) const;

    /**
     * The width of the image textures.
     */
    int imageTextureTileWidth() const;

    /**
     * The height of the image textures.
     */
    int imageTextureTileHeight() const;

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

    /**
     * Select selection visualization rendering.
     *
     * @param enable Set to true to enable selection visualization rendering.
     */
    void setSelectionDisplayEnabled(bool enable);

    /**
     * Update the image textures for the given image rectangle.
     *
     * @param imageRect The rectangle to update in image coordinates.
     */
    void update(const QRect& imageRect);

signals:
    /**
     * Emitted whenever an action has caused the image to be recomposited.
     *
     * @param r  The rectangle that has been recomposited.
     */
    void sigImageUpdated(const QRect &r);

    /**
     * Emitted whenever the image size changes.
     *
     * @param width  New image width
     * @param height New image height
     */
    void sigSizeChanged(qint32 width, qint32 height);

protected:
    KisOpenGLImageTextures(KisImageWSP image, KoColorProfile *monitorProfile);

    void createImageTextureTiles();
    void destroyImageTextureTiles();
    int imageTextureTileIndex(int x, int y) const;
    void updateImageTextureTiles(const QRect& rect);

    void setImageTextureFormat();

    static void createHDRExposureProgramIfCan();
    static bool imageCanUseHDRExposureProgram(KisImageWSP image);
    static bool imageCanShareTextures(KisImageWSP image);
    static bool haveHDRTextureFormat(const KoColorSpace *colorSpace);

protected slots:
    void slotImageUpdated(const QRect &);
    void slotImageSizeChanged(qint32 w, qint32 h);

private:
    KisImageWSP m_image;
    KoColorProfile *m_monitorProfile;
    float m_exposure;
    bool m_displaySelection;

    GLuint m_backgroundTexture;

    static const int PREFERRED_IMAGE_TEXTURE_WIDTH;
    static const int PREFERRED_IMAGE_TEXTURE_HEIGHT;

    QVector<GLuint> m_imageTextureTiles;
    int m_imageTextureTileWidth;
    int m_imageTextureTileHeight;
    int m_numImageTextureTileColumns;

    GLint m_imageTextureInternalFormat;
    GLenum m_imageTextureType;

    typedef std::map<KisImageWSP, KisOpenGLImageTextures*> ImageTexturesMap;

    static ImageTexturesMap imageTexturesMap;

#ifdef HAVE_GLEW
    bool m_usingHDRExposureProgram;
    static KisOpenGLHDRExposureProgram *HDRExposureProgram;
#endif
};

#endif // HAVE_OPENGL

#endif // KIS_OPENGL_IMAGE_TEXTURES_H_

