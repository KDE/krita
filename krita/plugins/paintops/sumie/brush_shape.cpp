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
#include <QImage>

#include <cmath>
#include "kis_debug.h"

BrushShape::BrushShape()
{

}

BrushShape::~BrushShape()
{

}

void BrushShape::fromDistance(int radius, float scale)
{
    Q_UNUSED(scale);
    m_width = m_height = radius * 2 + 1;
    qreal distance = 0.0;
    qreal maxDist = sqrt(radius * radius);

    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if ((x*x + y*y) < radius*radius) {
                distance = sqrt(x * x + y * y);
                distance /= maxDist;
                Bristle b(x, y, 1.0 - distance);
                b.setInkAmount(1.0f);
                m_bristles.append(b);
            }
        }
    }
}

void BrushShape::fromGaussian(int radius, float sigma)
{
    m_radius = radius;
    m_sigma = sigma;

    m_width = m_height = radius * 2 + 1;
    int gaussLength = (int)(m_width * m_width);
    //int center = (edgeSize - 1) / 2;

    float sigmaSquare = - 2.0 * sigma * sigma;
    float sigmaConst = 1.0 / (2.0 * M_PI * sigma * sigma);

    float total = 0;
    float length = 0;
    int p = 0;



    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            length = (exp((x * x + y * y) / sigmaSquare) * sigmaConst);
            total += length;
            Bristle b(x, y, length);
            b.setInkAmount(1.0f);
            m_bristles.append(b);
            p++;
        }
    }

    float minLen = m_bristles[0].length();
    float maxLen = m_bristles[gaussLength/2].length();
    float dist = maxLen - minLen;

    // normalise lengths
    float result;
    int i = 0;

    for (int x = 0; x < m_width; x++) {
        for (int y = 0; y < m_height; y++, i++) {
            result = (m_bristles[i].length() - minLen) / dist;
            m_bristles[i].setLength(result);
        }
    }

}

void BrushShape::fromLine(int radius, float sigma)
{
    m_radius = radius;
    m_sigma = sigma;

    m_width = radius * 2 + 1;
    m_height = 1;

    int gaussLength = m_width;

    float sigmaSquare = - 2.0f * sigma * sigma;
    float sigmaConst = 1.0f / (sigma * 2.506628f); /* sqrt(2.0*pi) */

    float length;
    for (int x = -radius; x <= radius; x++) {
        length = exp(x * x / sigmaSquare) * sigmaConst;
        Bristle b(x , 0.0f , length);
        m_bristles.append(b);
    }

    float minLen = m_bristles[0].length();
    float maxLen = m_bristles[gaussLength/2].length();
    float dist = maxLen - minLen;

    // normalise lengths
    float result;

    for (int x = 0; x < m_width; x++) {
        result = (m_bristles[x].length() - minLen) / dist;
        m_bristles[x].setLength(result);
    }
}

void BrushShape::fromBitMap(const char* fileName)
{
    QImage image(fileName, 0);
    if (image.isNull()) {
        return;
    }

    m_radius = -1;
    m_sigma = -1;

    m_width = image.width();
    m_height = image.height();

    int x_radius = m_width / 2;
    int y_radius = m_height / 2;

    QColor pixelColor;
    for (int x = -x_radius; x < x_radius; x++) {
        for (int y = -y_radius; y < y_radius; y++) {
            pixelColor.setRgba(image.pixel(x + x_radius, y + y_radius));
            Bristle b(x, y , (float)pixelColor.value() / 255.0f); // using value from image as length of bristle
            m_bristles.append(b);
        }
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

int BrushShape::radius()
{
    return m_radius;
}

float BrushShape::sigma()
{
    return m_sigma;
}

void BrushShape::tresholdBristles(double treshold)
{
    for (int i = 0; i < m_bristles.size(); i++) {
        if (m_bristles[i].length() < treshold) {
            m_bristles.remove(i);
        }
    }
}
