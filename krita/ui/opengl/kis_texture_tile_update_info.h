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

#include "opengl/kis_opengl.h"

#ifdef HAVE_OPENGL

#include <QMessageBox>
#include <QScopedArrayPointer>

#include <KoColorSpace.h>
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_config.h"
#include <KoColorConversionTransformation.h>
#include <KoChannelInfo.h>

class KisTextureTileUpdateInfo;
typedef QSharedPointer<KisTextureTileUpdateInfo> KisTextureTileUpdateInfoSP;
typedef QVector<KisTextureTileUpdateInfoSP> KisTextureTileUpdateInfoSPList;


class KisTextureTileUpdateInfo
{
public:
    KisTextureTileUpdateInfo()
        : m_patchPixels(0)
        , m_patchPixelsLength(0)
    {
    }

    KisTextureTileUpdateInfo(qint32 col, qint32 row, QRect tileRect, QRect updateRect, QRect currentImageRect)
        : m_patchPixels(0)
        , m_patchPixelsLength(0)

    {
        m_tileCol = col;
        m_tileRow = row;
        m_tileRect = tileRect;
        m_patchRect = m_tileRect & updateRect;
        m_currentImageRect = currentImageRect;
        m_numPixels = m_patchRect.width() * m_patchRect.height();
    }

    ~KisTextureTileUpdateInfo() {
    }

    void retrieveData(KisImageWSP image, QBitArray m_channelFlags, bool onlyOneChannelSelected, int selectedChannelIndex)
    {
        m_patchColorSpace = image->projection()->colorSpace();

        try {
            m_patchPixels.reset(new quint8[m_patchColorSpace->pixelSize() * m_patchRect.width() * m_patchRect.height()]);
        }
        catch (std::bad_alloc) {
            QMessageBox::critical(0, i18n("Fatal Error"), i18n("Krita has run out of memory and has to close."));
            qFatal("KisTextureTileUpdate::retrieveData: Could not allocate enough memory (1).");
        }

        image->projection()->readBytes(m_patchPixels.data(),
                                       m_patchRect.x(), m_patchRect.y(),
                                       m_patchRect.width(), m_patchRect.height());

        // XXX: if the paint colorspace is rgb, we should do the channel swizzling in
        //      the display shader
        if (!m_channelFlags.isEmpty()) {

            quint32 numPixels = m_patchRect.width() * m_patchRect.height();

            QScopedArrayPointer<quint8> channelProjectionCache;
            try {
                channelProjectionCache.reset(new quint8[m_patchColorSpace->pixelSize() * numPixels]);
            }
            catch (std::bad_alloc) {
                QMessageBox::critical(0, i18n("Fatal Error"), i18n("Krita has run out of memory and has to close."));
                qFatal("KisTextureTileUpdate::retrieveData: Could not allocate enough memory (1).");
            }

            QList<KoChannelInfo*> channelInfo = m_patchColorSpace->channels();
            int channelSize = channelInfo[selectedChannelIndex]->size();
            int pixelSize = m_patchColorSpace->pixelSize();

            KisConfig cfg;

            if (onlyOneChannelSelected && !cfg.showSingleChannelAsColor()) {
                int selectedChannelPos = channelInfo[selectedChannelIndex]->pos();
                for (uint pixelIndex = 0; pixelIndex < numPixels; ++pixelIndex) {
                    for (uint channelIndex = 0; channelIndex < m_patchColorSpace->channelCount(); ++channelIndex) {

                        if (channelInfo[channelIndex]->channelType() == KoChannelInfo::COLOR) {
                            memcpy(channelProjectionCache.data() + (pixelIndex * pixelSize) + (channelIndex * channelSize),
                                   m_patchPixels.data() + (pixelIndex * pixelSize) + selectedChannelPos,
                                   channelSize);
                        }
                        else if (channelInfo[channelIndex]->channelType() == KoChannelInfo::ALPHA) {
                            memcpy(channelProjectionCache.data() + (pixelIndex * pixelSize) + (channelIndex * channelSize),
                                   m_patchPixels.data() + (pixelIndex * pixelSize) + (channelIndex * channelSize),
                                   channelSize);
                        }
                    }
                }
            }
            else {
                for (uint pixelIndex = 0; pixelIndex < numPixels; ++pixelIndex) {
                    for (uint channelIndex = 0; channelIndex < m_patchColorSpace->channelCount(); ++channelIndex) {
                        if (m_channelFlags.testBit(channelIndex)) {
                            memcpy(channelProjectionCache.data() + (pixelIndex * pixelSize) + (channelIndex * channelSize),
                                   m_patchPixels.data() + (pixelIndex * pixelSize) + (channelIndex * channelSize),
                                   channelSize);
                        }
                        else {
                            memset(channelProjectionCache.data() + (pixelIndex * pixelSize) + (channelIndex * channelSize), 0, channelSize);
                        }
                    }
                }

            }

            m_patchPixels.swap(channelProjectionCache);

        }

    }

    void convertTo(const KoColorSpace* dstCS,
                   KoColorConversionTransformation::Intent renderingIntent,
                   KoColorConversionTransformation::ConversionFlags conversionFlags)
    {

        if (dstCS == m_patchColorSpace && conversionFlags == KoColorConversionTransformation::Empty) return;

        if (m_numPixels > 0) {

            const qint32 numPixels = m_patchRect.width() * m_patchRect.height();
            const quint32 conversionCacheLength = numPixels * dstCS->pixelSize();

            QScopedArrayPointer<quint8> conversionCache;
            try {
                conversionCache.reset(new quint8[conversionCacheLength]);
            }
            catch (std::bad_alloc) {
                QMessageBox::critical(0, i18n("Fatal Error"), i18n("Krita has run out of memory and has to close."));
                qFatal("KisTextureTileUpdate::convertTo. Could not allocate enough memory.");
            }

            m_patchColorSpace->convertPixelsTo(m_patchPixels.data(), conversionCache.data(), dstCS, numPixels, renderingIntent, conversionFlags);

            m_patchColorSpace = dstCS;
            m_patchPixels.swap(conversionCache);
            m_patchPixelsLength = conversionCacheLength;
        }
    }

    inline quint8* data() const {
        return m_patchPixels.data();
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

    inline quint32 patchPixelsLength() const {
        return m_patchPixelsLength;
    }

    inline bool valid() const {
        return m_numPixels > 0;
    }

private:
    Q_DISABLE_COPY(KisTextureTileUpdateInfo);

private:
    qint32 m_tileCol;
    qint32 m_tileRow;
    QRect m_currentImageRect;
    QRect m_tileRect;
    QRect m_patchRect;
    const KoColorSpace* m_patchColorSpace;
    QScopedArrayPointer<quint8> m_patchPixels;
    quint32 m_patchPixelsLength;

    quint32 m_numPixels;
};


#endif /* HAVE_OPENGL */

#endif /* KIS_TEXTURE_TILE_UPDATE_INFO_H_ */

