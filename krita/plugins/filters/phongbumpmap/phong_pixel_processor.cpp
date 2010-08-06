/*
*  Copyright (c) 2010 José Luis Vergara <pentalis@gmail.com>
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

#include "phong_pixel_processor.h"
#include <cmath>

PhongPixelProcessor::PhongPixelProcessor()
{
    initialize();
}

PhongPixelProcessor::PhongPixelProcessor(quint8 hmap[])
{
    heightmap = hmap;
    initialize();
}

PhongPixelProcessor::PhongPixelProcessor(quint8* hmap[])
{
    fastHeightmap = hmap;
    initialize();
}

void PhongPixelProcessor::initialize()
{
    reflection_vector = QVector3D(0, 0, 0);
    normal_vector = QVector3D(0, 0, 1);
    x_vector = QVector3D(1, 0, 0);
    y_vector = QVector3D(0, 1, 0);
    vision_vector = QVector3D(0, 0, 1);
    setLightVector(QVector3D(-8, 8, 2));
    
    Illuminant light1;
    Illuminant light2;
    light1.RGBvalue << 0 << 0 << 1;
    light2.RGBvalue << 1 << 0 << 0;
    light1.lightVector = QVector3D(6, 6, 3);
    light1.lightVector.normalize();
    light2.lightVector = QVector3D(-8, 8, 5);
    light2.lightVector.normalize();
    lightSources.append(light1);
    lightSources.append(light2);
    
    fastLight = light1;
    fastLight2 = light2;
    //Ka, Kd and Ks must be between 0 and 1 or grave errors will happen
    Ka = 0.0;
    Kd = 1;
    Ks = 1;
    shiny_exp = 40;
    
    Ia = Id = Is = 0;
    
}
PhongPixelProcessor::~PhongPixelProcessor()
{

}


void PhongPixelProcessor::setLightVector(QVector3D lightVector)
{
    lightVector.normalize();
    light_vector = lightVector;
}

QRgb PhongPixelProcessor::reallyFastIlluminatePixel(quint32 posup, quint32 posdown, quint32 posleft, quint32 posright)
{
    qreal temp;
    qreal Il;
    const int totalChannels = 3;
    qreal computation[] = {0, 0, 0};
    QColor pixelColor(0, 0, 0);
    
    normal_vector.setX(- heightmap[posright] + heightmap[posleft]);
    normal_vector.setY(- heightmap[posup] + heightmap[posdown]);
    normal_vector.setZ(8);
    normal_vector.normalize();
    
    /*
    foreach (Illuminant illuminant, lightSources) {
        temp = Kd * QVector3D::dotProduct(normal_vector, illuminant.lightVector);
        for (int channel = 0; channel < totalChannels; channel++) {
            //I = each RGB value
            Id = illuminant.RGBvalue[channel] * temp;
            if (Id < 0)     Id = 0;
            if (Id > 1)     Id = 1;
            computation[channel] += Id;
        }
    }
    */
    
    temp = QVector3D::dotProduct(normal_vector, fastLight.lightVector);
    for (int channel = 0; channel < totalChannels; channel++) {
        //I = each RGB value
        Id = fastLight.RGBvalue[channel] * temp;
        if (Id < 0)     Id = 0;
        if (Id > 1)     Id = 1;
        computation[channel] += Id;
    }
    
    temp = QVector3D::dotProduct(normal_vector, fastLight2.lightVector);
    for (int channel = 0; channel < totalChannels; channel++) {
        //I = each RGB value
        Id = fastLight2.RGBvalue[channel] * temp;
        if (Id < 0)     Id = 0;
        if (Id > 1)     Id = 1;
        computation[channel] += Id;
    }
    
    pixelColor.setRedF(computation[0]);
    pixelColor.setGreenF(computation[1]);
    pixelColor.setBlueF(computation[2]);
    
    return pixelColor.rgb();
}

/*
QRgb PhongPixelProcessor::fastIlluminatePixel(QPoint posup, QPoint posdown, QPoint posleft, QPoint posright)
{
    qreal I;
    qreal Il;
    int totalChannels = 3;
    qreal computation[] = {0, 0, 0};
    QColor pixelColor;
    
    normal_vector.setX(- fastHeightmap[posright.y()][posright.x()] + fastHeightmap[posleft.y()][posleft.x()]);
    normal_vector.setY(- fastHeightmap[posup.y()][posup.x()] + fastHeightmap[posdown.y()][posdown.x()]);
    normal_vector.setZ(8);
    normal_vector.normalize();
    reflection_vector = (2 * pow(QVector3D::dotProduct(normal_vector, light_vector), shiny_exp)) * normal_vector - light_vector;
    
    foreach (Illuminant illuminant, lightSources) {
        for (int channel = 0; channel < totalChannels; channel++) {
            //I = each RGB value
            Il = illuminant.RGBvalue[channel];
            Ia = Ka * Il;
            Id = Kd * Il * QVector3D::dotProduct(normal_vector, illuminant.lightVector);
            if (Id < 0)     Id = 0;
            Is = Ks * Il * QVector3D::dotProduct(vision_vector, reflection_vector);
            if (Is < 0)     Is = 0;
            I = Ia + Id + Is;
            if (I  > 1)     I  = 1;
            computation[channel] += I;
        }
    }
    
    for (int channel = 0; channel < totalChannels; channel++) {
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
*/

QColor PhongPixelProcessor::illuminatePixel(quint32 posup, quint32 posdown, quint32 posleft, quint32 posright)
{
    qreal I;
    qreal Il;
    int totalChannels = 3;
    qreal computation[] = {0, 0, 0};
    QColor pixelColor;

    normal_vector.setX(- heightmap[posright] + heightmap[posleft]);
    normal_vector.setY(- heightmap[posup] + heightmap[posdown]);
    normal_vector.setZ(8);
    normal_vector.normalize();
    reflection_vector = (2 * pow(QVector3D::dotProduct(normal_vector, light_vector), shiny_exp)) * normal_vector - light_vector;

    foreach (Illuminant illuminant, lightSources) {
        for (int channel = 0; channel < totalChannels; channel++) {
            //I = each RGB value
            Il = illuminant.RGBvalue[channel];
            Ia = Ka * Il;
            Id = Kd * Il * QVector3D::dotProduct(normal_vector, illuminant.lightVector);
            if (Id < 0)     Id = 0;
            Is = Ks * Il * QVector3D::dotProduct(vision_vector, reflection_vector);
            if (Is < 0)     Is = 0;
            I = Ia + Id + Is;
            if (I  > 1)     I  = 1;
            computation[channel] += I;
        }
    }
    
    for (int channel = 0; channel < totalChannels; channel++) {
        if (computation[channel] > 1)
            computation[channel] = 1;
        if (computation[channel] < 0)
            computation[channel] = 0;
    }
    
    pixelColor.setRedF(computation[0]);
    pixelColor.setGreenF(computation[1]);
    pixelColor.setBlueF(computation[2]);
    
    return pixelColor;
}