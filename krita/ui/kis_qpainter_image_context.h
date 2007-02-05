/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_QPAINTER_IMAGE_CONTEXT_H_
#define KIS_QPAINTER_IMAGE_CONTEXT_H_

#include <config-krita.h>

#include <map>

#include <QObject>
#include <q3valuevector.h>

#include <krita_export.h>

#include "kis_shared.h"
#include "kis_types.h"

class QRegion;
class KoColorSpace;
class QBrush;

class KisQPainterImageContext;
typedef KisSharedPtr<KisQPainterImageContext> KisQPainterImageContextSP;

class KRITAUI_EXPORT KisQPainterImageContext : public QObject , public KisShared {

    Q_OBJECT

public:
    static KisQPainterImageContextSP getImageContext(KisImageSP image, KoColorProfile *monitorProfile);

    KisQPainterImageContext();
    virtual ~KisQPainterImageContext();

public:

    void setMonitorProfile(KoColorProfile *profile);
    void setHDRExposure(float exposure);

    QBrush * backgroundTexture() const;

    // Get the image tile containing the point (pixelX, pixelY).
    QImage * imageTextureTile(int pixelX, int pixelY) const;

    int imageTextureTileWidth() const;
    int imageTextureTileHeight() const;

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
     * Clients using the KisQPainterImageContext should connect to the
     * following signals rather than to the KisImage's own equivalent
     * signals. This ensures that the image textures are always up to date
     * when used.
     */

    /**
     * Emitted whenever an action has caused the image to be recomposited.
     *
     * @param region  The region that has been recomposited.
     */
    void sigImageUpdated(QRegion region);

    /**
     * Emitted whenever the image size changes.
     *
     * @param width  New image width
     * @param height New image height
     */
    void sigSizeChanged(qint32 width, qint32 height);

protected:
    KisQPainterImageContext(KisImageSP image, KoColorProfile *monitorProfile);

    void generateBackgroundTexture();
    void createImageTextureTiles();
    void destroyImageTextureTiles();
    int imageTextureTileIndex(int x, int y) const;
    void updateImageTextureTiles(const QRect& rect);

    static KoColorSpace* textureColorSpaceForImageColorSpace(KoColorSpace *imageColorSpace);
    static bool imageCanShareImageContext(KisImageSP image);

protected slots:
    void slotImageUpdated(QRect r);
    void slotImageSizeChanged(qint32 w, qint32 h);

private:
    KisImageSP m_image;
    KoColorProfile *m_monitorProfile;
    float m_exposure;
    bool m_displaySelection;

    QBrush m_backgroundTexture;

    static const int PREFERRED_IMAGE_TEXTURE_WIDTH;
    static const int PREFERRED_IMAGE_TEXTURE_HEIGHT;

    Q3ValueVector<QImage*> m_imageTextureTiles;
    int m_imageTextureTileWidth;
    int m_imageTextureTileHeight;
    int m_numImageTextureTileColumns;

    // There is one image context per image, so multiple views don't
    // duplicate the cached qimages.
    typedef std::map<KisImageSP, KisQPainterImageContext*> ImageContextMap;

    static ImageContextMap imageContextMap;
};

#endif // KIS_QPAINTER_IMAGE_CONTEXT_H_

