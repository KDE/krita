/*
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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

#include <qstring.h>
#include <klocale.h>

#include "config.h"

#ifdef HAVE_OPENEXR
#include <half.h>
#endif

#include "kis_global.h"
#include "kis_basic_histogram_producers.h"
#include "kis_integer_maths.h"
#include "kis_channelinfo.h"
#include "kis_colorspace.h"
#include "kis_lab_colorspace.h"

KisLabColorSpace* KisGenericLabHistogramProducer::m_labCs = 0;


KisBasicHistogramProducer::KisBasicHistogramProducer(const KisID& id, int channels, int nrOfBins, KisColorSpace *cs)
    : m_channels(channels),
      m_nrOfBins(nrOfBins),
      m_colorSpace(cs),
      m_id(id)
{
    m_bins.resize(m_channels);
    for (int i = 0; i < m_channels; i++)
        m_bins.at(i).resize(m_nrOfBins);
    m_outLeft.resize(m_channels);
    m_outRight.resize(m_channels);
    m_count = 0;
    m_from = 0.0;
    m_width = 1.0;
}

void KisBasicHistogramProducer::clear() {
    m_count = 0;
    for (int i = 0; i < m_channels; i++) {
        for (int j = 0; j < m_nrOfBins; j++) {
            m_bins.at(i).at(j) = 0;
        }
        m_outRight.at(i) = 0;
        m_outLeft.at(i) = 0;
    }
}

void KisBasicHistogramProducer::makeExternalToInternal() {
    // This function assumes that the pixel is has no 'gaps'. That is to say: if we start
    // at byte 0, we can get to the end of the pixel by adding consecutive size()s of
    // the channels
    Q3ValueVector<KisChannelInfo *> c = channels();
    uint count = c.count();
    int currentPos = 0;

    for (uint i = 0; i < count; i++) {
        for (uint j = 0; j < count; j++) {
            if (c.at(j)->pos() == currentPos) {
                m_external.append(j);
                break;
            }
        }
        currentPos += c.at(m_external.at(m_external.count() - 1))->size();
    }
}

// ------------ U8 ---------------------

KisBasicU8HistogramProducer::KisBasicU8HistogramProducer(const KisID& id, KisColorSpace *cs)
    : KisBasicHistogramProducer(id, cs->nChannels(), 256, cs)
{
}

QString KisBasicU8HistogramProducer::positionToString(double pos) const {
    return QString("%1").arg(static_cast<quint8>(pos * UINT8_MAX));
}

void KisBasicU8HistogramProducer::addRegionToBin(quint8 * pixels, quint8 * selectionMask, quint32 nPixels, KisColorSpace *cs)
{
    qint32 pSize = cs->pixelSize();

    if ( selectionMask ) {
        while (nPixels > 0) {
            if ( ! (m_skipUnselected && *selectionMask == 0) || (m_skipTransparent && cs->getAlpha(pixels) == OPACITY_TRANSPARENT) ) {

                for (int i = 0; i < m_channels; i++) {
                    m_bins.at(i).at(pixels[i])++;
                }
                m_count++;

            }

            pixels += pSize;
            selectionMask++;
            nPixels--;
        }
    }
    else {
        while (nPixels > 0) {
            if ( ! (m_skipTransparent && cs->getAlpha(pixels) == OPACITY_TRANSPARENT) ) {

                for (int i = 0; i < m_channels; i++) {
                    m_bins.at(i).at(pixels[i])++;
                }
                m_count++;

            }

            pixels += pSize;
            selectionMask++;
            nPixels--;
        }
    }
}

// ------------ U16 ---------------------

KisBasicU16HistogramProducer::KisBasicU16HistogramProducer(const KisID& id, KisColorSpace *cs)
    : KisBasicHistogramProducer(id, cs->nChannels(), 256, cs)
{
}

QString KisBasicU16HistogramProducer::positionToString(double pos) const
{
    return QString("%1").arg(static_cast<quint8>(pos * UINT8_MAX));
}

double KisBasicU16HistogramProducer::maximalZoom() const
{
    return 1.0 / 255.0;
}

void KisBasicU16HistogramProducer::addRegionToBin(quint8 * pixels, quint8 * selectionMask, quint32 nPixels, KisColorSpace *cs)
{
    // The view
    quint16 from = static_cast<quint16>(m_from * UINT16_MAX);
    quint16 width = static_cast<quint16>(m_width * UINT16_MAX + 0.5); // We include the end
    quint16 to = from + width;
    double factor = 255.0 / width;

    qint32 pSize = cs->pixelSize();

    if ( selectionMask ) {
        quint16* pixel = reinterpret_cast<quint16*>(pixels);
        while (nPixels > 0) {
            if ( ! ((m_skipUnselected && *selectionMask == 0) || (m_skipTransparent && cs->getAlpha(pixels) == OPACITY_TRANSPARENT)) ) {
                for (int i = 0; i < m_channels; i++) {
                    quint16 value = pixel[i];
                    if (value > to)
                        m_outRight.at(i)++;
                    else if (value < from)
                        m_outLeft.at(i)++;
                    else
                        m_bins.at(i).at(static_cast<quint8>((value - from) * factor))++;
                }
                m_count++;
            }
            pixels += pSize;
            selectionMask++;
            nPixels--;
        }
    }
    else {
        while (nPixels > 0) {
            quint16* pixel = reinterpret_cast<quint16*>(pixels);

            if ( ! (m_skipTransparent && cs->getAlpha(pixels) == OPACITY_TRANSPARENT)) {
                for (int i = 0; i < m_channels; i++) {
                    quint16 value = pixel[i];
                    if (value > to)
                        m_outRight.at(i)++;
                    else if (value < from)
                        m_outLeft.at(i)++;
                    else
                        m_bins.at(i).at(static_cast<quint8>((value - from) * factor))++;
                }
                m_count++;
            }
            pixels += pSize;
            selectionMask++;
            nPixels--;

        }
    }
}

// ------------ Float32 ---------------------
KisBasicF32HistogramProducer::KisBasicF32HistogramProducer(const KisID& id, KisColorSpace *cs)
    : KisBasicHistogramProducer(id, cs->nChannels(), 256, cs)
{
}

QString KisBasicF32HistogramProducer::positionToString(double pos) const {
    return QString("%1").arg(static_cast<float>(pos)); // XXX I doubt this is correct!
}

double KisBasicF32HistogramProducer::maximalZoom() const {
    // XXX What _is_ the maximal zoom here? I don't think there is one with floats, so this seems a fine compromis for the moment
    return 1.0 / 255.0;
}

void KisBasicF32HistogramProducer::addRegionToBin(quint8 * pixels, quint8 * selectionMask, quint32 nPixels, KisColorSpace *cs) {
    // The view
    float from = static_cast<float>(m_from);
    float width = static_cast<float>(m_width);
    float to = from + width;
    float factor = 255.0 / width;

    qint32 pSize = cs->pixelSize();

    if ( selectionMask ) {
        while (nPixels > 0) {

            float* pixel = reinterpret_cast<float*>(pixels);
            if ( !((m_skipUnselected && *selectionMask == 0) || (m_skipTransparent && cs->getAlpha(pixels) == OPACITY_TRANSPARENT)) ) {
                for (int i = 0; i < m_channels; i++) {
                    float value = pixel[i];
                    if (value > to)
                        m_outRight.at(i)++;
                    else if (value < from)
                        m_outLeft.at(i)++;
                    else
                        m_bins.at(i).at(static_cast<quint8>((value - from) * factor))++;
                }
                m_count++;
            }

            pixels += pSize;
            selectionMask++;
            nPixels--;

        }
    }
    else {
        while (nPixels > 0) {

            float* pixel = reinterpret_cast<float*>(pixels);
            if ( !(m_skipTransparent && cs->getAlpha(pixels) == OPACITY_TRANSPARENT)) {
                for (int i = 0; i < m_channels; i++) {
                    float value = pixel[i];
                    if (value > to)
                        m_outRight.at(i)++;
                    else if (value < from)
                        m_outLeft.at(i)++;
                    else
                        m_bins.at(i).at(static_cast<quint8>((value - from) * factor))++;
                }
                m_count++;
            }

            pixels += pSize;
            selectionMask++;
            nPixels--;

        }
    }
}

#ifdef HAVE_OPENEXR
// ------------ Float16 Half ---------------------
KisBasicF16HalfHistogramProducer::KisBasicF16HalfHistogramProducer(const KisID& id,
                                                                   KisColorSpace *cs)
    : KisBasicHistogramProducer(id, cs->nChannels(), 256, cs) {
}

QString KisBasicF16HalfHistogramProducer::positionToString(double pos) const {
    return QString("%1").arg(static_cast<float>(pos)); // XXX I doubt this is correct!
}

double KisBasicF16HalfHistogramProducer::maximalZoom() const {
    // XXX What _is_ the maximal zoom here? I don't think there is one with floats, so this seems a fine compromis for the moment
    return 1.0 / 255.0;
}

void KisBasicF16HalfHistogramProducer::addRegionToBin(quint8 * pixels, quint8 * selectionMask, quint32 nPixels, KisColorSpace *cs) {
    // The view
    float from = static_cast<float>(m_from);
    float width = static_cast<float>(m_width);
    float to = from + width;
    float factor = 255.0 / width;

    qint32 pSize = cs->pixelSize();
    if ( selectionMask ) {
        while (nPixels > 0) {
            half* pixel = reinterpret_cast<half*>(pixels);
            if ( !((m_skipUnselected  && *selectionMask == 0) || (m_skipTransparent && cs->getAlpha(pixels) == OPACITY_TRANSPARENT)) ) {
                for (int i = 0; i < m_channels; i++) {
                    float value = pixel[i];
                    if (value > to)
                        m_outRight.at(i)++;
                    else if (value < from)
                        m_outLeft.at(i)++;
                    else
                        m_bins.at(i).at(static_cast<quint8>((value - from) * factor))++;
                }
                m_count++;
            }
            pixels += pSize;
            selectionMask++;
            nPixels--;
        }
    }
    else {
        while (nPixels > 0) {
            half* pixel = reinterpret_cast<half*>(pixels);
            if ( !(m_skipTransparent && cs->getAlpha(pixels) == OPACITY_TRANSPARENT)) {
                for (int i = 0; i < m_channels; i++) {
                    float value = pixel[i];
                    if (value > to)
                        m_outRight.at(i)++;
                    else if (value < from)
                        m_outLeft.at(i)++;
                    else
                        m_bins.at(i).at(static_cast<quint8>((value - from) * factor))++;
                }
                m_count++;
            }
            pixels += pSize;
            selectionMask++;
            nPixels--;
        }
    }
}
#endif

// ------------ Generic RGB ---------------------
KisGenericRGBHistogramProducer::KisGenericRGBHistogramProducer()
    : KisBasicHistogramProducer(KisID("GENRGBHISTO", i18n("Generic RGB Histogram")),
                                3, 256, 0) {
    /* we set 0 as colorspece, because we are not based on a specific colorspace. This
       is no problem for the superclass since we override channels() */
    m_channelsList.append(new KisChannelInfo(i18n("R"), 0, KisChannelInfo::COLOR, KisChannelInfo::UINT8, 1, QColor(255,0,0)));
    m_channelsList.append(new KisChannelInfo(i18n("G"), 1, KisChannelInfo::COLOR, KisChannelInfo::UINT8, 1, QColor(0,255,0)));
    m_channelsList.append(new KisChannelInfo(i18n("B"), 2, KisChannelInfo::COLOR, KisChannelInfo::UINT8, 1, QColor(0,0,255)));
}

Q3ValueVector<KisChannelInfo *> KisGenericRGBHistogramProducer::channels() {
    return m_channelsList;
}

QString KisGenericRGBHistogramProducer::positionToString(double pos) const {
    return QString("%1").arg(static_cast<quint8>(pos * UINT8_MAX));
}

double KisGenericRGBHistogramProducer::maximalZoom() const {
    return 1.0;
}


void KisGenericRGBHistogramProducer::addRegionToBin(quint8 * pixels, quint8 * selectionMask, quint32 nPixels, KisColorSpace *cs)
{
    for (int i = 0; i < m_channels; i++) {
        m_outRight.at(i) = 0;
        m_outLeft.at(i) = 0;
    }

    QColor c;
    qint32 pSize = cs->pixelSize();
    if (selectionMask) {
        while (nPixels > 0) {
            if ( !((m_skipUnselected  && *selectionMask == 0) || (m_skipTransparent && cs->getAlpha(pixels) == OPACITY_TRANSPARENT)) ) {
                cs->toQColor(pixels, &c);
                m_bins.at(0).at(c.Qt::red())++;
                m_bins.at(1).at(c.Qt::green())++;
                m_bins.at(2).at(c.Qt::blue())++;

                m_count++;
            }
            pixels += pSize;
            selectionMask++;
            nPixels--;
        }

    }
    else {
        while (nPixels > 0) {

            if ( !(m_skipTransparent && cs->getAlpha(pixels) == OPACITY_TRANSPARENT)) {
                cs->toQColor(pixels, &c);
                m_bins.at(0).at(c.Qt::red())++;
                m_bins.at(1).at(c.Qt::green())++;
                m_bins.at(2).at(c.Qt::blue())++;

                m_count++;
            }
            pixels += pSize;
            selectionMask++;
            nPixels--;
        }
    }
}

// ------------ Generic L*a*b* ---------------------
KisGenericLabHistogramProducer::KisGenericLabHistogramProducer()
    : KisBasicHistogramProducer(KisID("GENLABHISTO", i18n("L*a*b* Histogram")), 3, 256, 0) {
    /* we set 0 as colorspace, because we are not based on a specific colorspace. This
       is no problem for the superclass since we override channels() */
    m_channelsList.append(new KisChannelInfo(i18n("L*"), 0, KisChannelInfo::COLOR, KisChannelInfo::UINT8));
    m_channelsList.append(new KisChannelInfo(i18n("a*"), 1, KisChannelInfo::COLOR, KisChannelInfo::UINT8));
    m_channelsList.append(new KisChannelInfo(i18n("b*"), 2, KisChannelInfo::COLOR, KisChannelInfo::UINT8));

    if (!m_labCs) {
        KisProfile *labProfile = new KisProfile(cmsCreateLabProfile(NULL));
        m_labCs = new KisLabColorSpace(0, labProfile);
    }
    m_colorSpace = m_labCs;
}
KisGenericLabHistogramProducer::~KisGenericLabHistogramProducer()
{
    delete m_channelsList[0];
    delete m_channelsList[1];
    delete m_channelsList[2];
}

Q3ValueVector<KisChannelInfo *> KisGenericLabHistogramProducer::channels() {
    return m_channelsList;
}

QString KisGenericLabHistogramProducer::positionToString(double pos) const {
    return QString("%1").arg(static_cast<quint16>(pos * UINT16_MAX));
}

double KisGenericLabHistogramProducer::maximalZoom() const {
    return 1.0;
}


void KisGenericLabHistogramProducer::addRegionToBin(quint8 * pixels, quint8 * selectionMask, quint32 nPixels,  KisColorSpace *cs)
{
    for (int i = 0; i < m_channels; i++) {
        m_outRight.at(i) = 0;
        m_outLeft.at(i) = 0;
    }

    quint8 dst[8];
    qint32 pSize = cs->pixelSize();

    if (selectionMask) {
        while (nPixels > 0) {
            if ( !((m_skipUnselected  && *selectionMask == 0) || (m_skipTransparent && cs->getAlpha(pixels) == OPACITY_TRANSPARENT)) ) {
/*
  cs->toQColor(pixels, &c);
  m_bins.at(0).at(c.red())++;
*/
                m_count++;
            }
            pixels += pSize;
            selectionMask++;
            nPixels--;
        }
    }
    else {
        while (nPixels > 0) {
            if ( !(m_skipTransparent && cs->getAlpha(pixels) == OPACITY_TRANSPARENT))  {

  cs->convertPixelsTo(pixels, dst, m_colorSpace, 1);
  m_bins.at(0).at(m_colorSpace->scaleToU8(dst, 0))++;
  m_bins.at(1).at(m_colorSpace->scaleToU8(dst, 1))++;
  m_bins.at(2).at(m_colorSpace->scaleToU8(dst, 2))++;

                m_count++;
            }
            pixels += pSize;
            selectionMask++;
            nPixels--;
        }
    }
}

