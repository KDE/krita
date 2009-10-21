/*
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
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

#include "brush_shape.h"
#include <QVector>
#include <cmath>

const float PI = 3.141592f;

BrushShape::BrushShape()
{

}

BrushShape::~BrushShape()
{

}


void BrushShape::fromGaussian(int radius, float maxLength, float sigma)
{
    m_width = m_height = radius * 2 + 1;
    int gaussLength = (int)(m_width * m_width);
    //int center = (edgeSize - 1) / 2;

    float sigmaSquare = - 2.0 * sigma * sigma;
    float sigmaConst = 1.0 / (2.0 * PI * sigma * sigma);

    float total = 0;
    float length = 0;
    int p = 0;


    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            length = (std::exp((float)(x * x + y * y) / sigmaSquare) * sigmaConst);
            total += length;
            Bristle b(x, y, length*maxLength);
            b.setInkAmount(1.0f);
            m_bristles.append(b);
            p++;
        }
    }

    // normalise
    for (int i = 0; i < gaussLength; i++) {
        m_bristles[i].setLength(m_bristles[i].length() / total);
    }
}

QVector<Bristle> BrushShape::getBristles()
{
    return m_bristles;
}

int BrushShape::width()
{
    return m_width;
}

int BrushShape::height()
{
    return m_height;
}
