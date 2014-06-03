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
    {
    }

    KisTextureTileUpdateInfo(qint32 col, qint32 row, QRect tileRect, QRect updateRect, QRect currentImageRect)
        : m_patchPixels(0)
    {
        m_tileCol = col;
        m_tileRow = row;
        m_tileRect = tileRect;
        m_patchRect = m_tileRect & updateRect;
        m_currentImageRect = currentImageRect;
        m_numPixels = m_patchRect.width() * m_patchRect.height();
    }

    ~KisTextureTileUpdateInfo() {
        destroy();
        KIS_ASSERT_RECOVER_NOOP(!m_patchPixels);
    }

    void destroy() {
        delete[] m_patchPixels;
        m_patchPixels = 0;
    }

    void retrieveData(KisImageWSP image, QBitArray m_channelFlags, bool onlyOneChannelSelected, int selectedChannelIndex)
    {
        m_patchColorSpace = image->projection()->colorSpace();
        if (!m_patchPixels) {
            try {
                m_patchPixels = new quint8[m_patchColorSpace->pixelSize() * m_patchRect.width() * m_patchRect.height()];
            }
            catch (std::bad_alloc) {
                QMessageBox::critical(0, i18n("Fatal Error"), i18n("Krita has run out of memory and has to close."));
                qFatal("KisTextureTileUpdate::retrieveData: Could not allocate enough memory (1).");
            }

        }

        image->projection()->readBytes(m_patchPixels,
                                       m_patchRect.x(), m_patchRect.y(),
                                       m_patchRect.width(), m_patchRect.height());

        // XXX: if the paint colorspace is rgb, we should do the channel swizzling in
        //      the display shader
        if (!m_channelFlags.isEmpty()) {

            quint32 numPixels = m_patchRect.width() * m_patchRect.height();

            quint8 *dst = 0;
            try {
                dst = new quint8[m_patchColorSpace->pixelSize() * numPixels];
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
                            memcpy(dst + (pixelIndex * pixelSize) + (channelIndex * channelSize),
                                   m_patchPixels + (pixelIndex * pixelSize) + selectedChannelPos,
                                   channelSize);
                        }
                        else if (channelInfo[channelIndex]->channelType() == KoChannelInfo::ALPHA) {
                            memcpy(dst + (pixelIndex * pixelSize) + (channelIndex * channelSize),
                                   m_patchPixels + (pixelIndex * pixelSize) + (channelIndex * channelSize),
                                   channelSize);
                        }
                    }
                }
            }
            else {
                for (uint pixelIndex = 0; pixelIndex < numPixels; ++pixelIndex) {
                    for (uint channelIndex = 0; channelIndex < m_patchColorSpace->channelCount(); ++channelIndex) {
                        if (m_channelFlags.testBit(channelIndex)) {
                            memcpy(dst + (pixelIndex * pixelSize) + (channelIndex * channelSize),
                                   m_patchPixels  + (pixelIndex * pixelSize) + (channelIndex * channelSize),
                                   channelSize);
                        }
                        else {
                            memset(dst + (pixelIndex * pixelSize) + (channelIndex * channelSize), 0, channelSize);
                        }
                    }
                }

            }
            delete[] m_patchPixels;
            m_patchPixels = dst;

        }

    }

    void convertTo(const KoColorSpace* dstCS,
                   KoColorConversionTransformation::Intent renderingIntent,
                   KoColorConversionTransformation::ConversionFlags conversionFlags)
    {

        if (m_numPixels > 0) {
            const qint32 numPixels = m_patchRect.width() * m_patchRect.height();
            quint8* dstBuffer = 0;
            try {
                 dstBuffer = new quint8[dstCS->pixelSize() * numPixels];
            }
            catch (std::bad_alloc) {
                QMessageBox::critical(0, i18n("Fatal Error"), i18n("Krita has run out of memory and has to close."));
                qFatal("KisTextureTileUpdate::convertTo. Could not allocate enough memory.");
            }

            // FIXME: rendering intent
            Q_ASSERT(dstBuffer && m_patchPixels);
            m_patchColorSpace->convertPixelsTo(m_patchPixels, dstBuffer, dstCS, numPixels, renderingIntent, conversionFlags);

            delete[] m_patchPixels;

            m_patchColorSpace = dstCS;
            m_patchPixels = dstBuffer;
            m_patchPixelsLength = numPixels * dstCS->pixelSize();
        }
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
    quint8 *m_patchPixels;
    quint32 m_patchPixelsLength;
    quint32 m_numPixels;
};


#endif /* HAVE_OPENGL */

#endif /* KIS_TEXTURE_TILE_UPDATE_INFO_H_ */

