/*
 * Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com
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

#include <QVector>

#ifndef __KIS_GMIC_SIMPLE_CONVERTOR_H
#define __KIS_GMIC_SIMPLE_CONVERTOR_H

#include <gmic.h>
#include <kis_paint_device.h>


class KisGmicSimpleConvertor
{
public:
    KisGmicSimpleConvertor();
    ~KisGmicSimpleConvertor();

public:
    static QImage convertToQImage(gmic_image<float>& gmicImage);
    static void convertFromQImage(const QImage &image, gmic_image<float>& gmicImage);

    // convert functions
    void convertToGmicImage(KisPaintDeviceSP dev, gmic_image<float>& gmicImage);
    KisPaintDeviceSP convertFromGmicImage(gmic_image<float>& gmicImage, bool &preserveAlpha);
    // re-align functions
    void grayscale2rgb(cimg_library::CImg< float >& gmicImage, QVector< quint8 * > &planes);
    void grayscaleAlpha2rgba(cimg_library::CImg< float >& gmicImage, QVector< quint8 * > &planes);
    void rgb2rgb(cimg_library::CImg< float >& gmicImage, QVector< quint8 * > &planes);
    void rgba2rgba(cimg_library::CImg< float >& gmicImage, QVector< quint8 * > &planes);

    void releaseAlphaChannel()
    {
        // alphaPos == 3
        delete m_planarBytes[3];
        m_planarBytes[3] = 0;
    }

    void deletePlanes()
    {
        qDeleteAll(m_planarBytes);
        m_planarBytes.clear();
    }

    void accumulate(unsigned int channelSize, bool alphaChannelEnabled = true)
    {
        setChannelSize(channelSize);
        m_planarBytes.resize(4);

        int channelCount = 4;
        if (!alphaChannelEnabled)
        {
            m_planarBytes[3] = 0;
            channelCount = 3;
        }

        for (int i=0;i<channelCount;i++)
        {
            m_planarBytes[i] = new quint8[channelSize * sizeof(float)];
        }
    }

    void setChannelSize(unsigned int channelSize) { m_channelSize = channelSize; }
    // count of float pixels per channel
    unsigned int channelSize() {return m_channelSize;}

private:
    unsigned int m_channelSize;
    QVector<quint8 *> m_planarBytes;

};

#endif
