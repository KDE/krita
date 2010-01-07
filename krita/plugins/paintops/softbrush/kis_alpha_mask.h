/*
 *  Copyright (c) 2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#ifndef KIS_ALPHA_MASK_H
#define KIS_ALPHA_MASK_H

#include <QtGlobal>
#include <cmath>

class QPoint;
class QImage;

class KisCircleAlphaMask {

public:

        /* Class for storing qreal alpha masks 0.0...1.0 for various computational tasks in paintops*/
        KisCircleAlphaMask(int radius);
        ~KisCircleAlphaMask();
        
        void generateCircleDistanceMap(bool invert/*QPoint pos*/);
        void generateGaussMap(bool invert);
        
        void resize(int radius);
        void setSigma(qreal sigma);

        /// starts at 0,0
        inline qreal valueAt(int x, int y){ return m_data[qAbs(y) * m_width + qAbs(x)]; }
        
        QImage toQImage();
        
        void smooth(qreal edge0, qreal edge1);
        int radius() { return m_radius; }
        
private:
    qreal smoothstep (qreal edge0, qreal edge1, qreal x);
    qreal * m_data;
    int m_radius;
    int m_width;
    int m_size;
    
    qreal m_sigma;
    double m_sigmaSquared;
    double m_sigmaConst;

    inline qreal gaussAt(qreal x, qreal y){ return exp((x*x + y*y) / m_sigmaSquared) * m_sigmaConst; }
    inline qreal optGaussAt(qreal xx, qreal yy){ return exp((xx + yy) / m_sigmaSquared) * m_sigmaConst; }
};

#endif