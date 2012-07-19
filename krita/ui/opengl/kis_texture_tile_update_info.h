/*
 *  Copyright (c) 2010, Dmitry Kazakov <dimula73@gmail.com>
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
#ifndef KIS_TEXTURE_TILE_UPDATE_INFO_H_
#define KIS_TEXTURE_TILE_UPDATE_INFO_H_

#ifdef HAVE_OPENGL

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include "kis_image.h"
#include "kis_paint_device.h"

#include <KoColorConversionTransformation.h>


class KisTextureTileUpdateInfo;
typedef QVector<KisTextureTileUpdateInfo> KisTextureTileUpdateInfoList;

class KisTextureTileUpdateInfo
{
public:
    KisTextureTileUpdateInfo() {
        m_patchPixels = 0;
    }

    KisTextureTileUpdateInfo(qint32 col, qint32 row, QRect tileRect, QRect updateRect, QRect currentImageRect) {
        m_tileCol = col;
        m_tileRow = row;
        m_tileRect = tileRect;
        m_patchRect = m_tileRect & updateRect;
        m_currentImageRect = currentImageRect;
        m_patchPixels = 0;
    }

    ~KisTextureTileUpdateInfo() {
    }

    void destroy() {
        delete[] m_patchPixels;
    }

    void retrieveData(KisImageWSP image) {
        m_patchColorSpace = image->projection()->colorSpace();
        m_patchPixels = m_patchColorSpace->allocPixelBuffer(m_patchRect.width() * m_patchRect.height());
        image->projection()->readBytes(m_patchPixels,
                                       m_patchRect.x(), m_patchRect.y(),
                                       m_patchRect.width(), m_patchRect.height());
    }

    void convertTo(const KoColorSpace* dstCS,
                   KoColorConversionTransformation::Intent renderingIntent,
                   KoColorConversionTransformation::ConversionFlags conversionFlags) {
        const qint32 numPixels = m_patchRect.width() * m_patchRect.height();
        /**
         * FIXME: is it possible to do an in-place conversion?
         */
        quint8* dstBuffer = dstCS->allocPixelBuffer(numPixels);
        // FIXME: rendering intent
        Q_ASSERT(dstBuffer && m_patchPixels);
        m_patchColorSpace->convertPixelsTo(m_patchPixels, dstBuffer, dstCS, numPixels, renderingIntent, conversionFlags);
        delete[] m_patchPixels;
        m_patchColorSpace = dstCS;
        m_patchPixels = dstBuffer;
    }

    inline quint8* data() const {
        return m_patchPixels;
    }

    inline bool isEntireTileUpdated() const {
        return m_patchRect == m_tileRect;
    }

    inline QPoint patchOffset() const {
        return QPoint(m_patchRect.x() - m_tileRect.x(),
                      m_patchRect.y() - m_tileRect.y());
    }

    inline QSize patchSize() const {
        return m_patchRect.size();
    }

    inline QRect tileRect() const {
        return m_tileRect;
    }

    inline QRect imageRect() const {
        return m_currentImageRect;
    }

    inline qint32 tileCol() const {
        return m_tileCol;
    }

    inline qint32 tileRow() const {
        return m_tileRow;
    }

    inline quint32 pixelSize() const {
        return m_patchColorSpace->pixelSize();
    }

private:
    qint32 m_tileCol;
    qint32 m_tileRow;
    QRect m_currentImageRect;
    QRect m_tileRect;
    QRect m_patchRect;
    const KoColorSpace* m_patchColorSpace;
    quint8 *m_patchPixels;
};


#endif /* HAVE_OPENGL */

#endif /* KIS_TEXTURE_TILE_UPDATE_INFO_H_ */

