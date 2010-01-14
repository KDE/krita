/*
 *  Copyright (c) 2009,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_alpha_mask.h"
#include "kis_debug.h"

#include <cstdio>
#include <cmath>
#include <QImage>

KisCircleAlphaMask::KisCircleAlphaMask(int radius){
    m_sigma = 1.0;
    m_data = 0;
    resize(radius);
}



KisCircleAlphaMask::~KisCircleAlphaMask()
{
    delete [] m_data;
    m_data = 0;
}

inline void KisCircleAlphaMask::initSigma(qreal sigma)
{
    m_sigma = sigma;
    m_sigmaSquared = - 2.0 * m_sigma * m_sigma;
}

void KisCircleAlphaMask::setSigma(qreal sigma, qreal sigmaConst)
{
    initSigma(sigma);
    m_sigmaConst = sigmaConst;
}


void KisCircleAlphaMask::setSigma(qreal sigma)
{
    initSigma(sigma);
    m_sigmaConst = 1.0 / (2.0 * M_PI * m_sigma * m_sigma);
}



// TODO QPoint pos , we will see
void KisCircleAlphaMask::generateCircleDistanceMap(bool invert)
{
    int pos = 0;
    int yy = 0; // power of y
    qreal value;
    for (int y = 0; y <= m_radius; y++){
        yy = y*y;
        for (int x = 0; x <= m_radius; x++, pos++){
            value = sqrt(x*x + yy)/(m_radius);
            (value > 1.0) ? m_data[pos] = 0.0 : m_data[pos] = (invert ? 1.0 - value : value);
        }
    }
}

void KisCircleAlphaMask::generateGaussMap ( bool invert )
{
    Q_UNUSED(invert);
    //qreal factor = 1.0;

    // determine the "clever" radius
    int ix = 0;
    bool run = true;

    while (run){
        quint8 alpha = qRound(255 * gaussAt(ix, 0) );
        if (alpha > 0)
        {
            ix++;
        }else{
            run = false;
        }
    }
    // TODO: clean up after sucessful debugging
    int radius = ix;
    qreal step = radius / (qreal)m_radius;
    kDebug() << "Radius: " << m_radius << " | Computed radius: " << radius << "| Sigma: " << m_sigma << " |Step: " << step;;
    if (radius == 0) return;

    int pos = 0;
    qreal px = 0.0;
    qreal py = 0.0;
    qreal pyy = 0.0;
    for (int y = 0; y <= m_radius; y++) {
        for (int x = 0; x <= m_radius; x++,pos++) {
            m_data[pos] = gaussAt(px, py);
            px += step;
        }
        px = 0.0;
        py += step;
        pyy = py*py;
    }

    qreal minLen = m_data[m_size-1];
    qreal maxLen = m_data[0];
    //qreal dist = maxLen - minLen;

// normalize?
//     pos = 0;
//     while (pos < m_size){
//     //           m_data[pos] = (m_data[pos] - minLen) / dist;
//         m_data[pos] = m_data[pos] * 1.0/maxLen;
//         pos++;
//     }

#if 0
     pos = 0;
     for (int y = 0; y <= m_radius; y++) {
          for (int x = 0; x <= m_radius; x++,pos++) {
                 printf("%.3f ",m_data[pos]);
         }
         printf("\n");
     }
#endif
}




QImage KisCircleAlphaMask::toQImage()
{
    QImage img = QImage(m_width, m_width, QImage::Format_ARGB32);

    int pos = 0;
    int alpha;
    for (int y=0; y < m_width; y++){
        QRgb *pixel = reinterpret_cast<QRgb *>(img.scanLine(y));
        for (int x=0; x < m_width; x++, pos++){
            alpha = qRound(255 * m_data[pos]);
            pixel[y] = qRgba(alpha, alpha, alpha, 255);
        }
    }

    return img;
}




void KisCircleAlphaMask::resize(int radius)
{
    m_radius = radius;

    m_width = m_radius + 1;

    m_size = m_width * m_width;
    m_data = new qreal[m_size];
    memset(m_data, 0 , m_size * sizeof(qreal));
}



void KisCircleAlphaMask::smooth(qreal edge0, qreal edge1)
{
    int pos = 0;
    while (pos < m_size){
        m_data[pos] = smoothstep(edge0, edge1, m_data[pos]);
        pos++;
    }
}


inline qreal KisCircleAlphaMask::smoothstep(qreal edge0, qreal edge1, qreal x)
{
    if (x < edge0)      return 0.0;
    if (x >= edge1)     return 1.0;
    x = (x - edge0) / (edge1 - edge0);
    return x*x*(3 - 2 * x);
}


