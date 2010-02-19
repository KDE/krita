/*
 *  Copyright (c) 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef _BRUSH_SHAPE_H_
#define _BRUSH_SHAPE_H_

#include <QVector>
#include "bristle.h"

class QString;
class QImage;
class KoColorSpace;
class BrushShape
{

public:
    BrushShape();
    ~BrushShape();

    void fromDistance(int radius, float scale);
    void fromGaussian(int radius, float sigma);
    void fromLine(int radius, float sigma);
    // slow, optimize!
    void fromQImage(QImage image);
    void tresholdBristles(double treshold);

    QVector<Bristle*> getBristles();
    int width();
    int height();
    int radius();
    float sigma();

    bool hasColor(){ return m_hasColor; }
    void setHasColor(bool hasColor){ m_hasColor = hasColor; }
    void setColorSpace(KoColorSpace * cs){ m_colorSpace = cs;}
private:
    int m_width;
    int m_height;

    int m_radius;
    float m_sigma;

    bool m_hasColor;
    const KoColorSpace * m_colorSpace;
    QVector<Bristle*> m_bristles;
};

#endif
