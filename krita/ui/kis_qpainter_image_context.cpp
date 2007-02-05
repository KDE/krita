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

#include <config-krita.h>

#include <kdebug.h>
#include <ksharedptr.h>

#include "kis_global.h"
#include "kis_meta_registry.h"
#include "KoColorSpaceRegistry.h"
#include <KoIntegerMaths.h>
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_selection.h"
#include "kis_background.h"
#include "kis_qpainter_image_context.h"


#define PATTERN_WIDTH 64
#define PATTERN_HEIGHT 64

using namespace std;

KisQpainterImageContext::ImageContextMap KisQpainterImageContext::imageContextMap;

const int KisQpainterImageContext::PREFERRED_IMAGE_TEXTURE_WIDTH = 256;
const int KisQpainterImageContext::PREFERRED_IMAGE_TEXTURE_HEIGHT = 256;

KisQpainterImageContext::KisQpainterImageContext()
{
    m_image = 0;
    m_monitorProfile = 0;
    m_exposure = 0;
}

KisQpainterImageContext::~KisQpainterImageContext()
{
    kDebug(41001) << "Destroyed KisQpainterImageContext\n";

    --SharedContextWidgetRefCount;
    kDebug(41001) << "Shared context widget ref count now " << SharedContextWidgetRefCount << endl;

    if (SharedContextWidgetRefCount == 0) {

        kDebug(41001) << "Deleting shared context widget\n";
        delete SharedContextWidget;
        SharedContextWidget = 0;
    }

    imageContextMap.erase(m_image);
}

KisQpainterImageContext::KisQpainterImageContext(KisImageSP image, KoColorProfile *monitorProfile)
{
    kDebug(41001) << "Created KisQpainterImageContext\n";

    m_image = image;
    m_monitorProfile = monitorProfile;
    m_exposure = 0;
    m_displaySelection = true;

    generateBackgroundTexture();

    GLint max_texture_size;

    m_imageTextureTileWidth = PREFERRED_IMAGE_TEXTURE_WIDTH;
    m_imageTextureTileHeight = PREFERRED_IMAGE_TEXTURE_HEIGHT;

    createImageTextureTiles();

    connect(m_image.data(), SIGNAL(sigImageUpdated(QRect)), SLOT(slotImageUpdated(QRect)));
    connect(m_image.data(), SIGNAL(sigSizeChanged(qint32, qint32)), SLOT(slotImageSizeChanged(qint32, qint32)));

    updateImageTextureTiles(m_image->bounds());
}

KisQpainterImageContextSP KisQpainterImageContext::getImageContext(KisImageSP image, KoColorProfile *monitorProfile)
{
    if (imageCanShareImageContext(image)) {
        ImageContextMap::iterator it = imageContextMap.find(image);

        if (it != imageContextMap.end()) {

            kDebug(41001) << "Sharing image context from map\n";

            KisQpainterImageContextSP context = KisQpainterImageContextSP((*it).second);
            context->setMonitorProfile(monitorProfile);

            return context;
        } else {
            KisQpainterImageContext *imageContext = new KisQpainterImageContext(image, monitorProfile);
            imageContextMap[image] = imageContext;

            kDebug(41001) << "Added shareable context to map\n";

            return KisQpainterImageContextSP(imageContext);
        }
    } else {
        kDebug(41001) << "Creating non-shareable image context\n";

        return KisQpainterImageContextSP(new KisQpainterImageContext(image, monitorProfile));
    }
}

bool KisQpainterImageContext::imageCanShareImageContext(KisImageSP image)
{
    if (image->colorSpace()->hasHighDynamicRange()) {
        //XXX: and we don't have shaders...
        return false;
    } else {
        return true;
    }
}

QGLWidget *KisQpainterImageContext::sharedContextWidget() const
{
    return SharedContextWidget;
}

void KisQpainterImageContext::updateImageTextureTiles(const QRect& rect)
{
    //kDebug(41007) << "updateImageTextureTiles " << rect << endl;

    QRect updateRect = rect & m_image->bounds();

    if (!updateRect.isEmpty()) {

        int firstColumn = updateRect.left() / m_imageTextureTileWidth;
        int lastColumn = updateRect.right() / m_imageTextureTileWidth;
        int firstRow = updateRect.top() / m_imageTextureTileHeight;
        int lastRow = updateRect.bottom() / m_imageTextureTileHeight;

        for (int column = firstColumn; column <= lastColumn; column++) {
            for (int row = firstRow; row <= lastRow; row++) {

                QRect tileRect(column * m_imageTextureTileWidth, row * m_imageTextureTileHeight,
                               m_imageTextureTileWidth, m_imageTextureTileHeight);

                QRect tileUpdateRect = tileRect & updateRect;

                QImage tileUpdateImage = m_image->convertToQImage(tileUpdateRect.left(), tileUpdateRect.top(),
                                                                  tileUpdateRect.right(), tileUpdateRect.bottom(),
                                                                  m_monitorProfile, m_exposure);

                if (m_displaySelection) {
                    if (!m_image->activeLayer().isNull()) {
                        m_image->activeLayer()->paint(tileUpdateImage,
                                                      tileUpdateRect.x(), tileUpdateRect.y(),
                                                      tileUpdateRect.width(), tileUpdateRect.height());
                    }
                }

                if (tileUpdateRect.width() == m_imageTextureTileWidth && tileUpdateRect.height() == m_imageTextureTileHeight) {

                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_imageTextureTileWidth, m_imageTextureTileHeight, 0,
                                 GL_BGRA, GL_UNSIGNED_BYTE, tileUpdateImage.bits());
                } else {
                    int xOffset = tileUpdateRect.x() - tileRect.x();
                    int yOffset = tileUpdateRect.y() - tileRect.y();

                    glTexSubImage2D(GL_TEXTURE_2D, 0, xOffset, yOffset, tileUpdateRect.width(), tileUpdateRect.height(),
                                    GL_BGRA, GL_UNSIGNED_BYTE, tileUpdateImage.bits());
                }

            }
        }
    }
}

KoColorSpace* KisQpainterImageContext::textureColorSpaceForImageColorSpace(KoColorSpace */*imageColorSpace*/)
{
    return KoColorSpaceRegistry::instance()->colorSpace("RGBA", 0);
}

void KisQpainterImageContext::setMonitorProfile(KoColorProfile *monitorProfile)
{
    if (monitorProfile != m_monitorProfile) {
        m_monitorProfile = monitorProfile;
        generateBackgroundTexture();
        updateImageTextureTiles(m_image->bounds());
    }
}

void KisQpainterImageContext::setHDRExposure(float exposure)
{
    if (exposure != m_exposure) {
        m_exposure = exposure;

        if (m_image->colorSpace()->hasHighDynamicRange()) {
            //XXX: and we are not using shaders...
            updateImageTextureTiles(m_image->bounds());
        }
    }
}

void KisQpainterImageContext::generateBackgroundTexture()
{
    QPixmap tile(PATTERN_WIDTH * 2, PATTERN_HEIGHT * 2);
    tile.fill(Qt::white);
    QPainter pt(&tile);
    QColor color(220, 220, 220);
    pt.fillRect(0, 0, PATTERN_WIDTH, PATTERN_HEIGHT, color);
    pt.fillRect(PATTERN_WIDTH, PATTERN_HEIGHT, PATTERN_WIDTH, PATTERN_HEIGHT, color);
    pt.end();
    QBrush b(tile);

    m_backgroundTexture = b;

}

QBrush * KisQpainterImageContext::backgroundTexture() const
{
    return &m_backgroundTexture;
}

int KisQpainterImageContext::imageTextureTileIndex(int x, int y) const
{
    int column = x / m_imageTextureTileWidth;
    int row = y / m_imageTextureTileHeight;

    return column + (row * m_numImageTextureTileColumns);
}

GLuint KisQpainterImageContext::imageTextureTile(int pixelX, int pixelY) const
{
    qint32 textureTileIndex = imageTextureTileIndex(pixelX, pixelY);

    textureTileIndex = CLAMP(textureTileIndex, 0, ((qint32)m_imageTextureTiles.count()) - 1);

    return m_imageTextureTiles[textureTileIndex];
}

int KisQpainterImageContext::imageTextureTileWidth() const
{
    return m_imageTextureTileWidth;
}

int KisQpainterImageContext::imageTextureTileHeight() const
{
    return m_imageTextureTileHeight;
}

void KisQpainterImageContext::createImageTextureTiles()
{
    SharedContextWidget->makeCurrent();

    destroyImageTextureTiles();

    m_numImageTextureTileColumns = (m_image->width() + m_imageTextureTileWidth - 1) / m_imageTextureTileWidth;
    int numImageTextureTileRows = (m_image->height() + m_imageTextureTileHeight - 1) / m_imageTextureTileHeight;
    int numImageTextureTiles = m_numImageTextureTileColumns * numImageTextureTileRows;

    m_imageTextureTiles.resize(numImageTextureTiles);
    glGenTextures(numImageTextureTiles, &(m_imageTextureTiles[0]));

#define RGBA_BYTES_PER_PIXEL 4

    QByteArray emptyTilePixelData(m_imageTextureTileWidth * m_imageTextureTileHeight * RGBA_BYTES_PER_PIXEL, 0);

    for (int tileIndex = 0; tileIndex < numImageTextureTiles; ++tileIndex) {

        glBindTexture(GL_TEXTURE_2D, m_imageTextureTiles[tileIndex]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_imageTextureTileWidth, m_imageTextureTileHeight, 0,
                     GL_BGRA, GL_UNSIGNED_BYTE, emptyTilePixelData.data());
    }
}

void KisQpainterImageContext::destroyImageTextureTiles()
{
    if (!m_imageTextureTiles.empty()) {
        SharedContextWidget->makeCurrent();
        glDeleteTextures(m_imageTextureTiles.count(), &(m_imageTextureTiles[0]));
        m_imageTextureTiles.clear();
    }
}

void KisQpainterImageContext::update(const QRect& imageRect)
{
    updateImageTextureTiles(imageRect);
}

void KisQpainterImageContext::setSelectionDisplayEnabled(bool enable)
{
    m_displaySelection = enable;
}

void KisQpainterImageContext::slotImageUpdated(QRect rc)
{
    QRect r = rc & m_image->bounds();

    updateImageTextureTiles(r);
    emit sigImageUpdated(r);
}

void KisQpainterImageContext::slotImageSizeChanged(qint32 w, qint32 h)
{
    createImageTextureTiles();
    updateImageTextureTiles(m_image->bounds());

    emit sigSizeChanged(w, h);
}

#include "kis_qpainter_image_context.moc"



