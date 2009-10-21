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

#include "brush.h"
#include "brush_shape.h"
#include "trajectory.h"

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorTransformation.h>

#include <QVariant>
#include <QHash>
#include <QList>

#include "kis_random_accessor.h"
//#include "widgets/kis_curve_widget.h"

#include <cmath>
#include <ctime>

const float radToDeg = 57.29578f;

#if defined(_WIN32) || defined(_WIN64)
#define srand48 srand
inline double drand48()
{
    return double(rand()) / RAND_MAX;
}
#endif

Brush::Brush(const BrushShape &initialShape, const KoColor &inkColor)
{
    setBrushShape(initialShape);
    setInkColor(inkColor);
    initDefaultValues();
    srand48(time(0));
}

Brush::Brush()
{
    initDefaultValues();

    //TODO clean up this code
    BrushShape bs;
    m_radius = 20;
    m_sigma = 20.f;
    //bs.fromGaussian( m_radius, m_sigma );
    bs.fromLine(m_radius, m_sigma);
    setBrushShape(bs);
}

void Brush::initDefaultValues()
{
    m_counter = 0;
    m_lastAngle = 0.0;
    m_oldPressure = 0.0f;

    m_useSaturation = false;
    m_useOpacity = true;
    m_useWeights = false;

    // equally set weights
    m_bristleLengthWeight = 1.0 / 4.0;
    m_bristleInkAmountWeight = 1.0 / 4.0;
    m_pressureWeight = 1.0 / 4.0;
    m_inkDepletionWeight = 1.0 / 4.0;
}

void Brush::setBrushShape(BrushShape brushShape)
{
    m_initialShape = brushShape;
    m_bristles = brushShape.getBristles();
}

void Brush::enableMousePressure(bool enable)
{
    m_mousePressureEnabled = enable;
}

void Brush::setInkColor(const KoColor &color)
{
    for (int i = 0; i < m_bristles.size(); i++) {
        m_bristles[i].setColor(color);
    }
    m_inkColor = color;
}


void Brush::setInkDepletion(const QList<float>& curveData)
{
    int count = curveData.size();

    for (int i = 0; i < count ; i++) {
        m_inkDepletion.append(curveData.at(i));
    }
}

void Brush::paint(KisPaintDeviceSP dev, const KisPaintInformation &info)
{
    // TODO lets paint footprint of the brush here
    Q_UNUSED(dev)
    Q_UNUSED(info)
}


void Brush::paintLine(KisPaintDeviceSP dev, KisPaintDeviceSP layer, const KisPaintInformation &pi1, const KisPaintInformation &pi2)
{

    m_counter++;

    qreal dx = pi2.pos().x() - pi1.pos().x();
    qreal dy = pi2.pos().y() - pi1.pos().y();

    qreal x1 = pi1.pos().x();
    qreal y1 = pi1.pos().y();

    qreal x2 = pi2.pos().x();
    qreal y2 = pi2.pos().y();

    qreal angle = atan2(dy, dx);


    qreal distance = sqrt(dx * dx + dy * dy);

    qreal pressure = pi2.pressure();
    if (m_mousePressureEnabled && pi2.pressure() == 0.5) { // it is mouse and want pressure from mouse movement
        pressure = 1.0 - computeMousePressure(distance);
    } else if (pi2.pressure() == 0.5) { // if it is mouse
        pressure = 1.0;
    }

    Bristle *bristle = 0;
    KoColor brColor;

    KisRandomAccessor accessor = dev->createRandomAccessor((int)x1, (int)y1);
    m_pixelSize = dev->colorSpace()->pixelSize();
    m_dabAccessor = &accessor;
    m_dev = dev;


    KisRandomAccessor laccessor = layer->createRandomAccessor((int)x1, (int)y1);
    m_layerAccessor = &laccessor;
    m_layerPixelSize = layer->colorSpace()->pixelSize();
    m_layer = layer;

    qreal inkDeplation;

    QHash<QString, QVariant> params;
    params["h"] = 0.0;
    params["s"] = 0.0;
    params["v"] = 0.0;

    QString saturation("s");
    QVariant saturationVariant;

    KoColorTransformation* transfo;
    transfo = m_dev->colorSpace()->createColorTransformation("hsv_adjustment", params);

    rotateBristles(angle + 1.57);
    int ix1, iy1, ix2, iy2;

    int size = m_bristles.size();
    Trajectory trajectory; // used for interpolation the path of bristles
    QVector<QPointF> bristlePath; // path for single bristle
    for (int i = 0; i < size; i++) {
        /*            if (m_bristles[i].distanceCenter() > m_radius || drand48() <0.5){
                      continue;
                      }*/
        bristle = &m_bristles[i];

        qreal fx1, fy1, fx2, fy2;
        qreal rndFactor = m_randomFactor;
        qreal scaleFactor = m_scaleFactor;
        qreal shearFactor = m_shearFactor;

        qreal randomX = drand48();
        qreal randomY = drand48();
        randomX -= 0.5;
        randomY -= 0.5;
        randomX *= rndFactor;
        randomY *= rndFactor;

        qreal scale = pressure * scaleFactor;
        qreal shear = pressure * shearFactor;

        m_transform.reset();
        m_transform.scale(scale, scale);
        m_transform.translate(randomX, randomY);

        m_transform.shear(shear, shear);

        // transform start dab
        m_transform.map(bristle->x(), bristle->y(), &fx1, &fy1);
        // transform end dab
        m_transform.map(bristle->x(), bristle->y(), &fx2, &fy2);

        // all coords relative to device position
        fx1 += x1;
        fy1 += y1;

        fx2 += x2;
        fy2 += y2;

        ix1 = (int)fx1;
        iy1 = (int)fy1;
        ix2 = (int)fx2;
        iy2 = (int)fy2;

        // paint between first and last dab
        bristlePath = trajectory.getLinearTrajectory(QPointF(fx1, fy1), QPointF(fx2, fy2), 1.0);

        brColor = bristle->color();
        int bristleCounter = 0;
        int brpathSize = bristlePath.size();
        int inkDepletionSize = m_inkDepletion.size();


        for (int i = 0; i < brpathSize ; i++) {
            bristleCounter = bristle->increment();
            if (bristleCounter >= inkDepletionSize - 1) {
                inkDeplation = m_inkDepletion[inkDepletionSize - 1];
            } else {
                inkDeplation = m_inkDepletion[bristleCounter];
            }

            // saturation transformation of the bristle ink color
            // add check for hsv transformation
            if (m_useSaturation && transfo != 0) {
                if (m_useWeights) {

                    // new weighted way (experiment)
                    saturationVariant = (
                                            (pressure * m_pressureWeight) +
                                            (bristle->length() * m_bristleLengthWeight) +
                                            (bristle->inkAmount() * m_bristleInkAmountWeight) +
                                            ((1.0 - inkDeplation) * m_inkDepletionWeight)) - 1.0;
                } else {
                    // old way of computing saturation
                    saturationVariant = (
                                            pressure *
                                            bristle->length() *
                                            bristle->inkAmount() *
                                            (1.0 - inkDeplation)) - 1.0;

                }
                transfo->setParameter(saturation, saturationVariant);
                transfo->transform(m_inkColor.data(), brColor.data() , 1);
            }

            // opacity transformation of the bristle color
            if (m_useOpacity) {
                qreal opacity = 255.0;
                if (m_useWeights) {
                    opacity = (
                                  (pressure * m_pressureWeight) +
                                  (bristle->length() * m_bristleLengthWeight) +
                                  (bristle->inkAmount() * m_bristleInkAmountWeight) +
                                  ((1.0 - inkDeplation) * m_inkDepletionWeight)) * 255.0;

                } else {
                    opacity =
                        255.0 *
                        /* pressure * */
                        bristle->length() *
                        bristle->inkAmount() *
                        (1.0 - inkDeplation);
                }
                brColor.setOpacity(static_cast<int>(opacity));
            }

            QPointF *bristlePos = &bristlePath[i];
            addBristleInk(bristle, bristlePos->x(), bristlePos->y(), brColor);

#if 0
            // some kind of nice weighted bidirectional painting
            // FIXME: 8-bit specific
            QColor qcolor; // Creating a qcolor in a loop is very slow
            brColor.toQColor(&qcolor);
            // instead of magic constant use pressure
            //mixCMY(bristlePos->x(), bristlePos->y(), qcolor.cyan(), qcolor.magenta(), qcolor.yellow(), 0.20);
            mixCMY(bristlePos->x(), bristlePos->y(), qcolor.cyan(), qcolor.magenta(), qcolor.yellow(), pressure*0.30);
#endif
            bristle->setInkAmount(1.0 - inkDeplation);
        }

    }
    rotateBristles(-(angle + 1.57));
    //repositionBristles(angle,slope);
    m_dev = 0;
    m_dabAccessor = 0;
}


void Brush::rotateBristles(double angle)
{
    qreal tx, ty, x, y;

    m_transform.reset();
    m_transform.rotateRadians(angle);

    for (int i = 0; i < m_bristles.size(); i++) {
        x = m_bristles[i].x();
        y = m_bristles[i].y();
        m_transform.map(x, y, &tx, &ty);
        //      tx = cos(angle)*x - sin(angle)*y;
        //      ty = sin(angle)*x + cos(angle)*y;
        m_bristles[i].setX(tx);
        m_bristles[i].setY(ty);
    }
    m_lastAngle = angle;
}

void Brush::repositionBristles(double angle, double slope)
{
    // setX
    srand48((int)slope);
    for (int i = 0; i < m_bristles.size(); i++) {
        float x = m_bristles[i].x();
        m_bristles[i].setX(x + drand48());
    }

    // setY
    srand48((int)angle);
    for (int i = 0; i < m_bristles.size(); i++) {
        float y = m_bristles[i].y();
        m_bristles[i].setY(y + drand48());
    }
}

Brush::~Brush()
{
    /*    if (!m_dabAccessor){
          delete m_dabAccessor;
          }*/
}

inline void Brush::addBristleInk(Bristle *bristle, float wx, float wy, const KoColor &color)
{
    int ix = (int)wx;
    int iy = (int)wy;
    m_dabAccessor->moveTo(ix, iy);
    if (m_layer->colorSpace()->alpha(m_dabAccessor->rawData()) < color.opacity()) {
        memcpy(m_dabAccessor->rawData(), color.data(), m_pixelSize);
    }
    bristle->upIncrement();
}

void Brush::oldAddBristleInk(Bristle *bristle, float wx, float wy, const KoColor &color)
{
    KoMixColorsOp * mixOp = m_dev->colorSpace()->mixColorsOp();
    m_dabAccessor->moveTo((int)wx, (int)wy);
    const quint8 *colors[2];
    colors[0] = color.data();
    colors[1] = m_dabAccessor->rawData(); // this is always (0,0,0) in RGB

    qint16 colorWeights[2];

    colorWeights[0] = static_cast<quint8>(color.opacity());
    colorWeights[1] = static_cast<quint8>(255 - color.opacity());
    mixOp->mixColors(colors, colorWeights, 2, m_dabAccessor->rawData());

    //memcpy ( m_dabAccessor->rawData(), color.data(), m_pixelSize );
    // bristle delivered some ink
    bristle->upIncrement();
}


// FIXME : 8 bit specific
// Description: this reads pixel from layer and mix CMY color with weight
// Here is the question : is this for paintOp or for compositeOp?
void Brush::mixCMY(double x, double y, int cyan, int magenta, int yellow, double weight)
{
    int MAX_CHANNEL_VALUE = 256;
    int nred, ngreen, nblue;

    int ix = (int)x;
    int nextX = ix + 1;
    int iy = (int)y;
    int nextY = iy + 1;

    double nextXdist = (double)nextX - x;
    double xDist = 1.0f - nextXdist;

    double nextYdist = (double)nextY - y;
    double yDist = 1.0f - nextYdist;

    QColor layerQColor;
    QColor result;
    KoColor kcolor(m_dev->colorSpace());

    // ============
    double brightness = MAX_CHANNEL_VALUE * nextXdist * nextYdist * weight;
    m_layerAccessor->moveTo(ix, iy);
    m_layer->colorSpace()->toQColor(m_layerAccessor->rawData(), &layerQColor);

    nred = (int)(layerQColor.red() * MAX_CHANNEL_VALUE - brightness * cyan);
    if (nred < 0)
        nred = 0;

    ngreen = (int)(layerQColor.green() * MAX_CHANNEL_VALUE - brightness * magenta);
    if (ngreen < 0)
        ngreen = 0;

    nblue = (int)(layerQColor.blue() * MAX_CHANNEL_VALUE - brightness * yellow);
    if (nblue < 0)
        nblue = 0;

    result.setRgb(
        nred / MAX_CHANNEL_VALUE,
        ngreen / MAX_CHANNEL_VALUE,
        nblue / MAX_CHANNEL_VALUE,
        MAX_CHANNEL_VALUE - 1);

    kcolor.fromQColor(result);
    m_dabAccessor->moveTo(ix, iy);
    memcpy(m_dabAccessor->rawData(), kcolor.data(), m_pixelSize);


    // ============
    brightness = MAX_CHANNEL_VALUE * xDist * nextYdist * weight;
    m_layerAccessor->moveTo(nextX, iy);
    m_layer->colorSpace()->toQColor(m_layerAccessor->rawData(), &layerQColor);

    nred = (int)(layerQColor.red() * MAX_CHANNEL_VALUE - brightness * cyan);
    if (nred < 0)
        nred = 0;

    ngreen = (int)(layerQColor.green() * MAX_CHANNEL_VALUE - brightness * magenta);
    if (ngreen < 0)
        ngreen = 0;

    nblue = (int)(layerQColor.blue() * MAX_CHANNEL_VALUE - brightness * yellow);
    if (nblue < 0)
        nblue = 0;

    result.setRgb(
        nred / MAX_CHANNEL_VALUE,
        ngreen / MAX_CHANNEL_VALUE,
        nblue / MAX_CHANNEL_VALUE,
        MAX_CHANNEL_VALUE - 1);

    kcolor.fromQColor(result);
    m_dabAccessor->moveTo(nextX, iy);
    memcpy(m_dabAccessor->rawData(), kcolor.data(), m_pixelSize);


    // ============
    brightness = MAX_CHANNEL_VALUE * nextXdist * yDist * weight;
    m_layerAccessor->moveTo(ix, nextY);
    m_layer->colorSpace()->toQColor(m_layerAccessor->rawData(), &layerQColor);

    nred = (int)(layerQColor.red() * MAX_CHANNEL_VALUE - brightness * cyan);
    if (nred < 0) {
        nred = 0;
    }

    ngreen = (int)(layerQColor.green() * MAX_CHANNEL_VALUE - brightness * magenta);
    if (ngreen < 0) {
        ngreen = 0;
    }

    nblue = (int)(layerQColor.blue() * MAX_CHANNEL_VALUE - brightness * yellow);
    if (nblue < 0) {
        nblue = 0;
    }

    result.setRgb(
        nred / MAX_CHANNEL_VALUE,
        ngreen / MAX_CHANNEL_VALUE,
        nblue / MAX_CHANNEL_VALUE,
        MAX_CHANNEL_VALUE - 1);

    kcolor.fromQColor(result);
    m_dabAccessor->moveTo(ix, nextY);
    memcpy(m_dabAccessor->rawData(), kcolor.data(), m_pixelSize);

    // ============
    brightness = MAX_CHANNEL_VALUE * xDist * yDist * weight;
    m_layerAccessor->moveTo(nextX, nextY);
    m_layer->colorSpace()->toQColor(m_layerAccessor->rawData(), &layerQColor);

    nred = (int)(layerQColor.red() * MAX_CHANNEL_VALUE - brightness * cyan);
    if (nred < 0) {
        nred = 0;
    }

    ngreen = (int)(layerQColor.green() * MAX_CHANNEL_VALUE - brightness * magenta);
    if (ngreen < 0) {
        ngreen = 0;
    }

    nblue = (int)(layerQColor.blue() * MAX_CHANNEL_VALUE - brightness * yellow);
    if (nblue < 0) {
        nblue = 0;
    }

    result.setRgb(
        nred / MAX_CHANNEL_VALUE,
        ngreen / MAX_CHANNEL_VALUE,
        nblue / MAX_CHANNEL_VALUE,
        MAX_CHANNEL_VALUE - 1);

    kcolor.fromQColor(result);
    m_dabAccessor->moveTo(nextX, nextY);
    memcpy(m_dabAccessor->rawData(), kcolor.data(), m_pixelSize);
}


void Brush::putBristle(Bristle *bristle, float wx, float wy, const KoColor &color)
{
    m_dabAccessor->moveTo((int)wx, (int)wy);
    memcpy(m_dabAccessor->rawData(), color.data(), m_pixelSize);
    // bristle delivered some ink
    bristle->upIncrement();

// Wu particles..useless, the result is not better
//     int x = int(wx);
//     int y = int(wy);
//     float fx = wx - x;
//     float fy = wy - y;
//
//     float MAX_OPACITY = mycolor.opacity();
//
//     int btl = (1-fx) * (1-fy) * MAX_OPACITY;
//     int btr =  (fx)  * (1-fy) * MAX_OPACITY;
//     int bbl = (1-fx) *  (fy)  * MAX_OPACITY;
//     int bbr =  (fx)  *  (fy)  * MAX_OPACITY;
//
//     const int MIN_OPACITY = 10;
//
//     if (btl>MIN_OPACITY)
//     {
//         m_dabAccessor->moveTo(x,   y);
//         mycolor.setOpacity(btl);
//         memcpy ( m_dabAccessor->rawData(), mycolor.data(), m_pixelSize );
//     }
//
//     if (btr>MIN_OPACITY){
//         m_dabAccessor->moveTo(x+1, y);
//         mycolor.setOpacity(btr);
//         memcpy ( m_dabAccessor->rawData(), mycolor.data(), m_pixelSize );
//     }
//
//     if (bbl>MIN_OPACITY){
//         m_dabAccessor->moveTo(x, y+1);
//         mycolor.setOpacity(bbl);
//         memcpy ( m_dabAccessor->rawData(), mycolor.data(), m_pixelSize );
//     }
//
//     if (bbr>MIN_OPACITY){
//         m_dabAccessor->moveTo(x+1, y+1);
//         mycolor.setOpacity(bbr);
//         memcpy ( m_dabAccessor->rawData(), mycolor.data(), m_pixelSize );
//     }
}

void Brush::setRadius(int radius)
{
    m_radius = radius;
}

void Brush::setSigma(double sigma)
{
    m_sigma = sigma;
}

double Brush::computeMousePressure(double distance)
{
    double scale = 20.0;
    double minPressure = 0.02;
    double oldPressure = m_oldPressure;

    double factor = 1.0 - distance / scale;
    if (factor < 0.0) factor = 0.0;

    double result = ((4.0 * oldPressure) + minPressure + factor) / 5.0;
    m_oldPressure = result;
    return result;
}

