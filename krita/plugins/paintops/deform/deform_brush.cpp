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

#include "deform_brush.h"

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorTransformation.h>

#include <QVariant>
#include <QHash>
#include <QList>

#include <kis_iterator.h>
#include <kis_random_accessor.h>
#include <kis_random_sub_accessor.h>

#include <cmath>
#include <ctime>
#include <QTime>

const qreal radToDeg = 57.29578;
const qreal degToRad = M_PI / 180.0;

#if defined(_WIN32) || defined(_WIN64)
#define srand48 srand
inline double drand48()
{
    return double(rand()) / RAND_MAX;
}
#endif

DeformBrush::DeformBrush()
{
    m_firstPaint = false;
}

DeformBrush::~DeformBrush()
{
    if (m_distanceTable != 0) {
        delete[] m_distanceTable;
    }
}


/// this method uses my code for bilinear interpolation
inline void DeformBrush::myMovePixel(qreal newX, qreal newY, quint8 *dst)
{
    if (m_useBilinear) {
        if (m_useOldData) {
            bilinear_interpolation_old(newX, newY, dst);
        } else {
            bilinear_interpolation(newX, newY, dst);
        }

    } else if (point_interpolation(&newX, &newY, m_image)) {
        m_readAccessor->moveTo(newX, newY);

        if (m_useOldData) {
            memcpy(dst, m_readAccessor->oldRawData(), m_pixelSize);
        } else {
            memcpy(dst, m_readAccessor->rawData(), m_pixelSize);
        }
    }
}

/// this method uses KisSubPixelAccessor
inline void DeformBrush::movePixel(qreal newX, qreal newY, quint8 *dst)
{
    if (m_useBilinear) {
        m_srcAcc->moveTo(newX, newY);

        if (m_useOldData) {
            m_srcAcc->sampledOldRawData(dst);
        } else {
            m_srcAcc->sampledRawData(dst);
        }
    } else {
        if (point_interpolation(&newX, &newY, m_image)) {
            m_readAccessor->moveTo(newX, newY);
            if (m_useOldData) {
                memcpy(dst , m_readAccessor->oldRawData(), m_pixelSize);
            } else {
                memcpy(dst , m_readAccessor->rawData(), m_pixelSize);
            }
        }
    }
}

/***
* methods with fast prefix (like this one called fastScale) uses KisRectIterator,
* they are faster just a little bit (according my tests 120 miliseconds faster with big radius, slower with small radius)
**/
void DeformBrush::fastScale(qreal cursorX, qreal cursorY, qreal factor)
{
    int curXi = static_cast<int>(cursorX + 0.5);
    int curYi = static_cast<int>(cursorY + 0.5);

    qreal newX, newY;
    qreal distance;
    qreal scaleFactor;

    qint32 x;
    qint32 y;

    int left = curXi - m_radius;
    int top = curYi - m_radius;
    int w = m_radius * 2 + 1;
    int h = w;

    KisRectIterator m_srcIt = m_dab->createRectIterator(left, top, w , h);
    for (; !m_srcIt.isDone(); ++m_srcIt) {

        x = m_srcIt.x();
        y = m_srcIt.y();

        newX = x - curXi;
        newY = y - curYi;

        distance = distanceFromCenter(abs(newX), abs(newY));
        if (distance > 1.0) continue;

        scaleFactor = (1.0 - distance) * factor + distance;

        newX /= scaleFactor;
        newY /= scaleFactor;

        newX += curXi;
        newY += curYi;

        myMovePixel(newX, newY, m_srcIt.rawData());
    }
}


void DeformBrush::fastLensDistortion(qreal cursorX, qreal cursorY, qreal k1, qreal k2)
{

    int curXi = static_cast<int>(cursorX + 0.5);
    int curYi = static_cast<int>(cursorY + 0.5);

    qreal newX, newY;
    qreal distance;

    qint32 x;
    qint32 y;

    int left = curXi - m_radius;
    int top = curYi - m_radius;
    int w = m_radius * 2 + 1;
    int h = w;

    KisRectIterator m_srcIt = m_dab->createRectIterator(left, top, w , h);
    for (; !m_srcIt.isDone(); ++m_srcIt) {

        x = m_srcIt.x();
        y = m_srcIt.y();

        newX = (x - curXi);
        newY = (y - curYi);

        // normalized distance
        distance = distanceFromCenter(abs(newX), abs(newY));
        if (distance > 1.0) continue;

        //normalize
        newX /= m_maxdist;
        newY /= m_maxdist;

        qreal radius_2 = newX * newX + newY * newY;
        qreal radius_4 = radius_2 * radius_2;

        if (m_action == 7) {
            newX = newX * (1.0 + k1 * radius_2 + k2 * radius_4);
            newY = newY * (1.0 + k1 * radius_2 + k2 * radius_4);
        } else {
            newX = newX / (1.0 + k1 * radius_2 + k2 * radius_4);
            newY = newY / (1.0 + k1 * radius_2 + k2 * radius_4);
        }

        newX = m_maxdist * newX;
        newY = m_maxdist * newY;

        newX += curXi;
        newY += curYi;

        myMovePixel(newX, newY, m_srcIt.rawData());
    }
}


void DeformBrush::fastMove(qreal cursorX, qreal cursorY, qreal dx, qreal dy)
{
    int curXi = static_cast<int>(cursorX + 0.5);
    int curYi = static_cast<int>(cursorY + 0.5);

    qreal x, y;
    qreal newX, newY;
    qreal distance;

    int left = curXi - m_radius;
    int top = curYi - m_radius;
    int w = m_radius * 2 + 1;
    int h = w;

    KisRectIterator m_srcIt = m_dab->createRectIterator(left, top, w , h);
    for (; !m_srcIt.isDone(); ++m_srcIt) {
        x = m_srcIt.x();
        y = m_srcIt.y();

        newX = x - curXi;
        newY = y - curYi;

        distance = distanceFromCenter(abs(newX), abs(newY));
        if (distance > 1.0) continue;

        newX -= dx * m_amount * (1.0 - distance);
        newY -= dy * m_amount * (1.0 - distance);

        newX += curXi;
        newY += curYi;

        myMovePixel(newX, newY, m_srcIt.rawData());
    }
}


void DeformBrush::fastDeformColor(qreal cursorX, qreal cursorY, qreal amount)
{
    int curXi = static_cast<int>(cursorX + 0.5);
    int curYi = static_cast<int>(cursorY + 0.5);

    qreal x, y;
    qreal newX, newY;
    qreal distance;
    qreal randomX, randomY;

    int left = curXi - m_radius;
    int top = curYi - m_radius;
    int w = m_radius * 2 + 1;
    int h = w;

    KisRectIterator m_srcIt = m_dab->createRectIterator(left, top, w , h);
    for (; !m_srcIt.isDone(); ++m_srcIt) {
        x = m_srcIt.x();
        y = m_srcIt.y();

        distance = distanceFromCenter(abs(x - curXi), abs(y - curYi));
        if (distance > 1.0) continue;

        randomX = drand48();
        randomY = drand48();

        randomX = (randomX * 2.0) - 1.0;
        randomY = (randomY * 2.0) - 1.0;

        newX = x + (amount * randomX);
        newY = y + (amount * randomY);

        myMovePixel(newX, newY, m_srcIt.rawData());
    }

}


void DeformBrush::fastSwirl(qreal cursorX, qreal cursorY, qreal alpha)
{
    int curXi = static_cast<int>(cursorX + 0.5);
    int curYi = static_cast<int>(cursorY + 0.5);

    qreal x, y;
    qreal newX, newY;
    qreal rotX, rotY;
    qreal distance;

    int left = curXi - m_radius;
    int top = curYi - m_radius;
    int w = m_radius * 2 + 1;
    int h = w;

    KisRectIterator m_srcIt = m_dab->createRectIterator(left, top, w , h);
    for (; !m_srcIt.isDone(); ++m_srcIt) {
        x = m_srcIt.x();
        y = m_srcIt.y();

        newX = x - curXi;
        newY = y - curYi;

        distance = distanceFromCenter(abs(newX), abs(newY));
        if (distance > 1.0) continue;

        distance = 1.0 - distance;
        rotX = cos(-alpha * distance) * newX - sin(-alpha * distance) * newY;
        rotY = sin(-alpha * distance) * newX + cos(-alpha * distance) * newY;

        newX = rotX;
        newY = rotY;

        newX += curXi;
        newY += curYi;

        myMovePixel(newX, newY, m_srcIt.rawData());
    }
}


/***
* Methods without fast prefix uses KisRandomAccessor (m_writeAccesor created on temporary device)
**/
void DeformBrush::lensDistortion(qreal cursorX, qreal cursorY, qreal k1, qreal k2)
{
    int curXi = static_cast<int>(cursorX + 0.5);
    int curYi = static_cast<int>(cursorY + 0.5);

    qreal newX, newY;
    qreal distance;

    for (int x = curXi - m_radius; x < curXi + m_radius; x++) {
        for (int y = curYi - m_radius; y < curYi + m_radius; y++) {
            newX = (x - curXi);
            newY = (y - curYi);

            // normalized distance
            distance = distanceFromCenter(abs(newX), abs(newY));
            if (distance > 1.0) continue;

            //normalize
            newX /= m_maxdist;
            newY /= m_maxdist;

            qreal radius_2 = newX * newX + newY * newY;
            qreal radius_4 = radius_2 * radius_2;

            if (m_action == 7) {
                newX = newX * (1.0 + k1 * radius_2 + k2 * radius_4);
                newY = newY * (1.0 + k1 * radius_2 + k2 * radius_4);
            } else {
                newX = newX / (1.0 + k1 * radius_2 + k2 * radius_4);
                newY = newY / (1.0 + k1 * radius_2 + k2 * radius_4);
            }

            newX = m_maxdist * newX;
            newY = m_maxdist * newY;

            newX += curXi;
            newY += curYi;

            m_writeAccessor->moveTo(x, y);
            movePixel(newX, newY, m_writeAccessor->rawData());
        }
    }
}


void DeformBrush::move(qreal cursorX, qreal cursorY, qreal dx, qreal dy)
{
    int curXi = static_cast<int>(cursorX + 0.5);
    int curYi = static_cast<int>(cursorY + 0.5);
    //KoColor kcolor( m_dev->colorSpace() );

    qreal newX, newY;
    qreal distance;

    for (int x = curXi - m_radius; x < curXi + m_radius; x++) {
        for (int y = curYi - m_radius; y < curYi + m_radius; y++) {
            newX = x - curXi;
            newY = y - curYi;

            // normalized distance
            distance = distanceFromCenter(abs(newX), abs(newY));

            // we want circle
            if (distance > 1.0) continue;

            newX -= dx * m_amount * (1.0 - distance);
            newY -= dy * m_amount * (1.0 - distance);

            newX += curXi;
            newY += curYi;

            m_writeAccessor->moveTo(x, y);
            movePixel(newX, newY, m_writeAccessor->rawData());
        }
    }
}


void DeformBrush::scale(qreal cursorX, qreal cursorY, qreal factor)
{
    int curXi = static_cast<int>(cursorX + 0.5);
    int curYi = static_cast<int>(cursorY + 0.5);
    //KoColor kcolor( m_dev->colorSpace() );

    qreal newX, newY;
    qreal distance;
    qreal scaleFactor;

    for (int x = curXi - m_radius; x < curXi + m_radius; x++) {
        for (int y = curYi - m_radius; y < curYi + m_radius; y++) {
            newX = x - curXi;
            newY = y - curYi;

            // normalized distance
            distance = distanceFromCenter(abs(newX), abs(newY));

            // we want circle
            if (distance > 1.0) continue;
            scaleFactor = (1.0 - distance) * factor + distance;

            newX /= scaleFactor;
            newY /= scaleFactor;

            newX += curXi;
            newY += curYi;

            m_writeAccessor->moveTo(x, y);
            movePixel(newX, newY, m_writeAccessor->rawData());
        }
    }
}


void DeformBrush::deformColor(qreal cursorX, qreal cursorY, qreal amount)
{
    int curXi = static_cast<int>(cursorX + 0.5);
    int curYi = static_cast<int>(cursorY + 0.5);

    qreal newX, newY;
    qreal randomX, randomY;
    qreal distance;

    srand48(time(0));
    for (int x = curXi - m_radius; x <= curXi + m_radius; x++) {
        for (int y = curYi - m_radius; y <= curYi + m_radius; y++) {

            distance = distanceFromCenter(abs(x - curXi), abs(y - curYi));
            if (distance > 1.0) continue;

            randomX = drand48();
            randomY = drand48();

            randomX = (randomX * 2.0) - 1.0;
            randomY = (randomY * 2.0) - 1.0;

            newX = x + (amount * randomX);
            newY = y + (amount * randomY);

            m_writeAccessor->moveTo(x, y);
            movePixel(newX, newY, m_writeAccessor->rawData());
        }
    }
}



void DeformBrush::swirl(qreal cursorX, qreal cursorY, qreal alpha)
{
    int curXi = static_cast<int>(cursorX + 0.5);
    int curYi = static_cast<int>(cursorY + 0.5);

    qreal newX, newY;
    qreal rotX, rotY;
    qreal distance;

    for (int x = curXi - m_radius; x <= curXi + m_radius; x++) {
        for (int y = curYi - m_radius; y <= curYi + m_radius; y++) {

            newX = x - curXi;
            newY = y - curYi;

            distance = distanceFromCenter(abs(newX), abs(newY));
            if (distance > 1.0) continue;

            distance = 1.0 - distance;
            rotX = cos(-alpha * distance) * newX - sin(-alpha * distance) * newY;
            rotY = sin(-alpha * distance) * newX + cos(-alpha * distance) * newY;

            newX = rotX;
            newY = rotY;

            newX += curXi;
            newY += curYi;

            m_writeAccessor->moveTo(x, y);
            movePixel(newX, newY, m_writeAccessor->rawData());
        }
    }
}

void DeformBrush::paint(KisPaintDeviceSP dev, KisPaintDeviceSP layer, const KisPaintInformation &info)
{
    qreal x1 = info.pos().x();
    qreal y1 = info.pos().y();

    m_dev = layer;
    m_dab = dev;
    m_pixelSize = dev->colorSpace()->pixelSize();

    KisRandomAccessor accessor = dev->createRandomAccessor((int)x1, (int)y1);
    m_writeAccessor = &accessor;

    KisRandomConstAccessor accessor2 = layer->createRandomConstAccessor((int)x1, (int)y1);
    m_readAccessor = &accessor2;

    KisRandomSubAccessorPixel srcAcc = layer->createRandomSubAccessor();
    m_srcAcc = &srcAcc;

#if 1
    if (m_action == 1) {
        // grow
        if (m_useCounter) {
            fastScale(x1, y1, 1.0 + m_counter*m_counter / 100.0);
        } else {
            fastScale(x1, y1, 1.0 + m_amount);
        }
    } else

        if (m_action == 2) {
            // shrink
            if (m_useCounter) {
                fastScale(x1, y1, 1.0 - m_counter*m_counter / 100.0);
            } else {
                fastScale(x1, y1, 1.0 - m_amount);
            }
        } else

            if (m_action == 3) {
                // CW
                if (m_useCounter) {
                    fastSwirl(x1, y1, (m_counter) *  degToRad);
                    //fastSwirl(x1,y1, (1.0/360*m_counter) *  radToDeg); // crazy fast swirl
                } else {
                    fastSwirl(x1, y1, (360 * m_amount * 0.5) *  degToRad);
                }

            } else

                if (m_action == 4) {
                    // CCW
                    if (m_useCounter) {
                        fastSwirl(x1, y1, (m_counter) *  degToRad);
                        //fastSwirl(x1,y1, (1.0/360*m_counter) * -radToDeg); // crazy fast swirl ccw
                    } else {
                        fastSwirl(x1, y1, (360 * m_amount * 0.5) *  -degToRad);
                    }
                } else

                    if (m_action == 5) {
                        if (m_firstPaint == false) {
                            m_prevX = x1;
                            m_prevY = y1;
                            m_firstPaint = true;
                        } else {
                            fastMove(x1, y1, x1 - m_prevX, y1 - m_prevY);
                            m_prevX = x1;
                            m_prevY = y1;
                        }
                    } else

                        if (m_action == 6 || m_action == 7) {
                            fastLensDistortion(x1, y1, m_amount, 0);
                        } else if (m_action == 8) {
                            fastDeformColor(x1, y1, m_amount);
                        }
#endif

#if 0
    if (m_action == 1) {
        // grow
        if (m_useCounter) {
            scale(x1, y1, 1.0 + m_counter*m_counter / 100.0);
        } else {
            scale(x1, y1, 1.0 + m_amount);
        }
    } else

        if (m_action == 2) {
            // shrink
            if (m_useCounter) {
                scale(x1, y1, 1.0 - m_counter*m_counter / 100.0);
            } else {
                scale(x1, y1, 1.0 - m_amount);
            }
        } else

            if (m_action == 3) {
                // CW
                swirl(x1, y1, (1.0 / 360*m_counter) *  radToDeg);
            } else

                if (m_action == 4) {
                    // CCW
                    swirl(x1, y1, (1.0 / 360*m_counter) * -radToDeg);
                } else

                    if (m_action == 5) {
                        if (m_firstPaint == false) {
                            m_prevX = x1;
                            m_prevY = y1;
                            m_firstPaint = true;
                        } else {
                            move(x1, y1, x1 - m_prevX, y1 - m_prevY);
                        }
                    } else

                        if (m_action == 6 || m_action == 7) {
                            lensDistortion(x1, y1, m_amount, 0);
                        } else if (m_action == 8) {
                            deformColor(x1, y1, m_amount);
                        }

#endif

    m_counter++;
}


bool DeformBrush::point_interpolation(qreal* x, qreal* y, KisImageWSP image)
{
    if (*x >= 0 && *x < image->width() - 1 && *y >= 0 && *y < image->height() - 1) {
        *x = *x + 0.5; // prepare for typing to int
        *y = *y + 0.5;
        return true;
    }
    return false;
}

void DeformBrush::debugColor(const quint8* data)
{
    QColor rgbcolor;
    m_dev->colorSpace()->toQColor(data, &rgbcolor);
    dbgPlugins << "RGBA: ("
    << rgbcolor.red()
    << ", " << rgbcolor.green()
    << ", " << rgbcolor.blue()
    << ", " << rgbcolor.alpha() << ")";
}

void DeformBrush::precomputeDistances(int radius)
{
    int size = (radius + 1) * (radius + 1);
    m_distanceTable = new qreal[size];
    int pos = 0;

    for (int y = 0; y <= radius; y++)
        for (int x = 0; x <= radius; x++, pos++) {
            m_distanceTable[pos] = sqrt(x * x + y * y) / m_maxdist;
        }
}


void DeformBrush::bilinear_interpolation(double x, double y, quint8 *dst)
{
    KoMixColorsOp * mixOp = m_dev->colorSpace()->mixColorsOp();

    int ix = (int)floor(x);
    int iy = (int)floor(y);

    if (ix >= 0 &&
            ix <= m_image->width() - 2 &&
            iy >= 0 &&
            iy <= m_image->height() - 2) {
        const quint8 *colors[4];
        m_readAccessor->moveTo(ix, iy);
        colors[0] = m_readAccessor->rawData(); //11

        m_readAccessor->moveTo(ix + 1, iy);
        colors[1] = m_readAccessor->rawData(); //12

        m_readAccessor->moveTo(ix, iy + 1);
        colors[2] = m_readAccessor->rawData(); //21

        m_readAccessor->moveTo(ix + 1, iy + 1);
        colors[3] = m_readAccessor->rawData();  //22

        double x_frac = x - (double)ix;
        double y_frac = y - (double)iy;

        qint16 colorWeights[4];
        int MAX_16BIT = 255;

        colorWeights[0] = static_cast<quint16>((1.0 - y_frac) * (1.0 - x_frac) * MAX_16BIT);
        colorWeights[1] = static_cast<quint16>((1.0 - y_frac) *  x_frac * MAX_16BIT);
        colorWeights[2] = static_cast<quint16>(y_frac * (1.0 - x_frac) * MAX_16BIT);
        colorWeights[3] = static_cast<quint16>(y_frac * x_frac * MAX_16BIT);

        mixOp->mixColors(colors, colorWeights, 4, dst);
    }
}


void DeformBrush::bilinear_interpolation_old(double x, double y , quint8 *dst)
{
    KoMixColorsOp * mixOp = m_dev->colorSpace()->mixColorsOp();

    int ix = (int)floor(x);
    int iy = (int)floor(y);

    if (ix >= 0 &&
            ix <= m_image->width() - 2 &&
            iy >= 0 &&
            iy <= m_image->height() - 2) {
        const quint8 *colors[4];
        m_readAccessor->moveTo(ix, iy);
        colors[0] = m_readAccessor->oldRawData(); //11

        m_readAccessor->moveTo(ix + 1, iy);
        colors[1] = m_readAccessor->oldRawData(); //12

        m_readAccessor->moveTo(ix, iy + 1);
        colors[2] = m_readAccessor->oldRawData(); //21

        m_readAccessor->moveTo(ix + 1, iy + 1);
        colors[3] = m_readAccessor->oldRawData();  //22

        double x_frac = x - (double)ix;
        double y_frac = y - (double)iy;

        qint16 colorWeights[4];
        int MAX_16BIT = 255;

        colorWeights[0] = static_cast<quint16>((1.0 - y_frac) * (1.0 - x_frac) * MAX_16BIT);
        colorWeights[1] = static_cast<quint16>((1.0 - y_frac) *  x_frac * MAX_16BIT);
        colorWeights[2] = static_cast<quint16>(y_frac * (1.0 - x_frac) * MAX_16BIT);
        colorWeights[3] = static_cast<quint16>(y_frac * x_frac * MAX_16BIT);
        mixOp->mixColors(colors, colorWeights, 4, dst);
    }
}
