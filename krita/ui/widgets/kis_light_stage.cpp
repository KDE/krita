/*
 *  Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
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

#include "kis_light_stage.h"


KisLightStage::KisLightStage(QWidget *parent) :
    QWidget(parent)
{
    diffuseLightIsEnabled = true;
    specularLightIsEnabled = true;
    relativeZIlum = 2 / qreal(width());  // This number is what makes the heightmap look like a perfect hemisphere
}

KisLightStage::~KisLightStage()
{

}


void KisLightStage::paintEvent(QPaintEvent* event)
{
    QImage heightmapQImage(width(), height(), QImage::Format_RGB32);
        qDebug() << "Etto 1";
    QPainter painter(&heightmapQImage);
    painter.setRenderHint(QPainter::Antialiasing, 0x01);
    QPen pen;
    pen.setWidth(1);
    pen.setColor(QColor(0,0,0));
    QBrush brush(semiSphericalGradient());
    painter.setBrush(brush);
    painter.setPen(pen);
    painter.drawEllipse(0, 0, this->width(), this->height());
    painter.combinedMatrix();
    QPainter painter2(this);
    qDebug() << "Etto 2";
    //painter2.setCompositionMode(QPainter::CompositionMode_SourceOver);
/*
    QRgb* tempcolorline;
    for (int y = 0; y < heightmapQImage.height(); ++y) {
        for (int x = 0; x < heightmapQImage.width(); ++x) {
            tempcolorline = (QRgb *) heightmapQImage.scanLine(y);
            heightmap << QColor(tempcolorline[x]).red();
        }
    }
*/
    heightmap = semiSphericalHeightmap();

    // IF NO LIGHT SOURCES --> DO NOTHING HERE
    lightSources = findChildren<KisLightSource *>();

    qDebug() << lightSources;
    painter2.drawImage(1, 1, drawPhongBumpmap(), 0, 0, width()-2, height()-2);
    //painter2.drawImage(0, 0, heightmapQImage, 0, 0, width(), height());

    QPointF center(0, 0);
    center.setX(qreal(this->width()) / 2.0);
    center.setY(qreal(this->height()) / 2.0);
    qreal radius = center.x();
    QPointF position(0, 0);
}

QRadialGradient KisLightStage::semiSphericalGradient()
{
    qint16 counter = 0xFF;
    qreal m, z = 0;
    //m = distance from center
    //z = height, this gradient will be used as a heightmap
    QColor stopColor(0, 0, 0);
    QPointF center(qreal(width() / 2), qreal(height() / 2));
    QRadialGradient semiSphere(center, qreal(width() / 2));
    while (counter > 0) {
        z = counter;
        z /= 0xFF;
        counter -= 2;
        stopColor.setRgbF(z, z, z, 1);
        m = sqrt(1 - z * z);
        semiSphere.setColorAt(m, stopColor);
    }
    return semiSphere;
}

QVector<qreal> KisLightStage::semiSphericalHeightmap()
{
    qreal rx, ry;  // [-1, 1]
    QVector<qreal> foreverAlone(width() * height());
    qreal renderAzimuth;
    qreal rm;
    qreal inclination;
    qreal rz;
    for (int y = 0; y < height(); ++y) {
        for (int x = 0; x < width(); ++x) {
            //Obtain relative x and y
            rx = qreal(x - qreal(width() / 2));
            ry = qreal(y - qreal(height() / 2));
            //Normalize
            rx /= qreal(width() / 2);
            ry /= qreal(height() / 2);

            if (!(rx == 0 && ry == 0)) {
                renderAzimuth = atan2(rx, ry);
            } else {
                renderAzimuth = 0;
            }

            //Pitagoras
            rm = sqrt(pow(rx, 2) + pow(ry, 2));

            inclination = acos(rm);

            rz = sin(inclination);
            if (rz != rz) {
                foreverAlone << 0;
            } else {
                //foreverAlone << rz;
                foreverAlone[y * width() + x] = rz;
            }
            //foreverAlone[y * width() + x] = sin(inclination);
        }
    }
    return foreverAlone;
}

QImage KisLightStage::drawPhongBumpmap()
{
    const quint16 HEIGHT_MINUS_1 = height() - 1;
    const quint16 WIDTH_MINUS_1 = width() - 1;
    const quint8 OUTPUT_OFFSET = 1;

    quint32 posup;
    quint32 posdown;
    quint32 posleft;
    quint32 posright;

    QSize inputArea(width(), height());

    QImage bumpmap(width()-2, height()-2, QImage::Format_RGB32);
    bumpmap.fill(0x000000);

    QRgb** bumpmapByteLines = new QRgb*[bumpmap.height()];

    for (int yIndex = 0; yIndex < bumpmap.height(); yIndex++)
        bumpmapByteLines[yIndex] = (QRgb *) bumpmap.scanLine(yIndex);

    // Foreach inner pixel in the heightmap do...
    for (int y = 1; y < HEIGHT_MINUS_1; ++y) {
        for (int x = 1; x < WIDTH_MINUS_1; ++x) {
            posup   = (y + 1) * inputArea.width() + x;
            posdown = (y - 1) * inputArea.width() + x;
            posleft  = y * inputArea.width() + x - 1;
            posright = y * inputArea.width() + x + 1;

            bumpmapByteLines[y - OUTPUT_OFFSET][x - OUTPUT_OFFSET] =
            illuminatePixel(posup, posdown, posleft, posright);
        }
    }

//    delete [] bumpmapByteLines;
    return bumpmap;
}

QRgb KisLightStage::illuminatePixel(quint32 posup, quint32 posdown, quint32 posleft, quint32 posright)
{
    qreal temp;
    quint8 channel = 0;
    const quint8 totalChannels = 3;
    qreal computation[] = {0, 0, 0};
    QColor pixelColor(0, 0, 0);
    QVector3D normal_vector(0, 0, 1);
    QVector3D light_vector(0, 0, 1);
    QVector3D vision_vector(0, 0, 1);
    QVector3D reflection_vector(0, 0, 0);
    QColor fastLight(0, 0, 0);
    qreal Id, Kd, Ia, Ka, Is, Ks = 0;
    Kd = 0.5;
    Ka = 0.0;
    Ks = 0.0;
    qreal shiny_exp = 1;


    // Algorithm begins, Phong Illumination Model

    normal_vector.setX(- heightmap[posright] + heightmap[posleft]);
    normal_vector.setY(- heightmap[posup] + heightmap[posdown]);
    normal_vector.setZ(relativeZIlum);
    //normal_vector.setZ(0.005);

/*
    QVector3D X (1, 0, heightmap[posright] - heightmap[posleft]);
    QVector3D Y (0, 1, heightmap[posup] - heightmap[posdown]);
    normal_vector = QVector3D::crossProduct(X, Y);
    qDebug() << normal_vector;
*/
    normal_vector.normalize();

    // PREPARE ALGORITHM HERE

    for (int i = 0; i < lightSources.count(); i++) {
        light_vector = lightSources.at(i)->lightVector;

        for (channel = 0; channel < totalChannels; channel++) {
            Ia = lightSources.at(i)->RGBvalue.at(channel) * Ka;
            computation[channel] += Ia;
        }
        if (diffuseLightIsEnabled) {
            //temp = Kd * QVector3D::dotProduct(normal_vector, light_vector);
            temp = Kd * QVector3D::dotProduct(light_vector, normal_vector);
            for (channel = 0; channel < totalChannels; channel++) {
                Id = lightSources.at(i)->RGBvalue.at(channel) * temp;
                if (Id < 0)     Id = 0;
                if (Id > 1)     Id = 1;
                computation[channel] += Id;
            }
        }

        if (specularLightIsEnabled) {
            reflection_vector = 2 * QVector3D::dotProduct(normal_vector, light_vector) * normal_vector - light_vector;
            temp = Ks * pow(QVector3D::dotProduct(reflection_vector, vision_vector), shiny_exp);
            for (channel = 0; channel < totalChannels; channel++) {
                Is = lightSources.at(i)->RGBvalue.at(channel) * temp;
                if (Is < 0)     Is = 0;
                if (Is > 1)     Is = 1;
                computation[channel] += Is;
            }
        }
    }

    for (channel = 0; channel < totalChannels; channel++) {
        if (computation[channel] > 1)
            computation[channel] = 1;
        if (computation[channel] < 0)
            computation[channel] = 0;
    }

    pixelColor.setRedF(computation[0]);
    pixelColor.setGreenF(computation[1]);
    pixelColor.setBlueF(computation[2]);

    return pixelColor.rgb();
}
