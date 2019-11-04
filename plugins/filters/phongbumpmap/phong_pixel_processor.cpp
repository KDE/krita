/*
*  Copyright (c) 2010-2012 José Luis Vergara <pentalis@gmail.com>
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
#include <iostream>
#include <KoChannelInfo.h>

PhongPixelProcessor::PhongPixelProcessor(quint32 pixelArea, const KisPropertiesConfigurationSP config)
{
    m_pixelArea = pixelArea;
    initialize(config);
}

void PhongPixelProcessor::initialize(const KisPropertiesConfigurationSP config)
{
    // Basic, fundamental
    normal_vector = QVector3D(0, 0, 1);
    vision_vector = QVector3D(0, 0, 1);
    x_vector = QVector3D(1, 0, 0);
    y_vector = QVector3D(0, 1, 0);

    // Mutable
    light_vector = QVector3D(0, 0, 0);
    reflection_vector = QVector3D(0, 0, 0);

    //setLightVector(QVector3D(-8, 8, 2));

    Illuminant light[PHONG_TOTAL_ILLUMINANTS];
    QVariant guiLight[PHONG_TOTAL_ILLUMINANTS];

    qint32 azimuth;
    qint32 inclination;

    for (int i = 0; i < PHONG_TOTAL_ILLUMINANTS; i++) {
        if (config->getBool(PHONG_ILLUMINANT_IS_ENABLED[i])) {
            if (config->getProperty(PHONG_ILLUMINANT_COLOR[i], guiLight[i])) {
                light[i].RGBvalue << guiLight[i].value<QColor>().redF();
                light[i].RGBvalue << guiLight[i].value<QColor>().greenF();
                light[i].RGBvalue << guiLight[i].value<QColor>().blueF();

                azimuth = config->getInt(PHONG_ILLUMINANT_AZIMUTH[i]) - 90;
                inclination = config->getInt(PHONG_ILLUMINANT_INCLINATION[i]);

                qreal m; //2D vector magnitude
                light[i].lightVector.setZ( sin( inclination * M_PI / 180 ) );
                m = cos( inclination * M_PI / 180);

                light[i].lightVector.setX( cos( azimuth * M_PI / 180 ) * m  );
                light[i].lightVector.setY( sin( azimuth * M_PI / 180 ) * m  );
                //Pay close attention to this, indexes will move in this line
                lightSources.append(light[i]);
            }
        }
    }

    size = lightSources.size();

    //Code that exists only to swiftly switch to the other algorithm (reallyFastIlluminatePixel) to test
    if (size > 0) {
        fastLight = light[0];
        fastLight2 = light[0];
    }

    //Ka, Kd and Ks must be between 0 and 1 or grave errors will happen
    Ka = config->getDouble(PHONG_AMBIENT_REFLECTIVITY);
    Kd = config->getDouble(PHONG_DIFFUSE_REFLECTIVITY);
    Ks = config->getDouble(PHONG_SPECULAR_REFLECTIVITY);
    shiny_exp = config->getInt(PHONG_SHINYNESS_EXPONENT);

    Ia = Id = Is = 0;

    diffuseLightIsEnabled = config->getBool(PHONG_DIFFUSE_REFLECTIVITY_IS_ENABLED);
    specularLightIsEnabled = config->getBool(PHONG_SPECULAR_REFLECTIVITY_IS_ENABLED);
    
    realheightmap = QVector<double>(m_pixelArea, 0);
}


PhongPixelProcessor::~PhongPixelProcessor()
{

}


void PhongPixelProcessor::setLightVector(QVector3D lightVector)
{
    lightVector.normalize();
    light_vector = lightVector;
}

QVector<quint16> PhongPixelProcessor::IlluminatePixelFromHeightmap(quint32 posup, quint32 posdown, quint32 posleft, quint32 posright)
{
    QVector<quint16> finalPixel(4, 0xFFFF);

    if (lightSources.size() == 0)
        return finalPixel;

    // Algorithm begins, Phong Illumination Model
    normal_vector.setX(- realheightmap[posright] + realheightmap[posleft]);
    normal_vector.setY(- realheightmap[posup] + realheightmap[posdown]);
    normal_vector.setZ(8);
    normal_vector.normalize();

    // PREPARE ALGORITHM HERE

    finalPixel = IlluminatePixel();
    
    return finalPixel;
}

QVector<quint16> PhongPixelProcessor::IlluminatePixel()
{
    qreal temp;
    quint8 channel = 0;
    const quint8 totalChannels = 3; // The 4th is alpha and we'll fill it with a nice 0xFFFF
    qreal computation[] = {0, 0, 0};
    QVector<quint16> finalPixel(4, 0xFFFF);

    if (lightSources.size() == 0)
        return finalPixel;

    // PREPARE ALGORITHM HERE

    for (int i = 0; i < size; i++) {
        light_vector = lightSources.at(i).lightVector;

        for (channel = 0; channel < totalChannels; channel++) {
            Ia = lightSources.at(i).RGBvalue.at(channel) * Ka;
            computation[channel] += Ia;
        }
        if (diffuseLightIsEnabled) {
            temp = Kd * QVector3D::dotProduct(normal_vector, light_vector);
            for (channel = 0; channel < totalChannels; channel++) {
                Id = lightSources.at(i).RGBvalue.at(channel) * temp;
                if (Id < 0)     Id = 0;
                if (Id > 1)     Id = 1;
                computation[channel] += Id;
            }
        }

        if (specularLightIsEnabled) {
            reflection_vector = (2 * pow(QVector3D::dotProduct(normal_vector, light_vector), shiny_exp)) * normal_vector - light_vector;
            temp = Ks * QVector3D::dotProduct(vision_vector, reflection_vector);
            for (channel = 0; channel < totalChannels; channel++) {
                Is = lightSources.at(i).RGBvalue.at(channel) * temp;
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

    //RGBA actually uses the BGRA order of channels, hence the disorder
    finalPixel[2] = quint16(computation[0] * 0xFFFF);
    finalPixel[1] = quint16(computation[1] * 0xFFFF);
    finalPixel[0] = quint16(computation[2] * 0xFFFF);
    
    return finalPixel;
}

QVector<quint16> PhongPixelProcessor::IlluminatePixelFromNormalmap(qreal r, qreal g, qreal b)
{
    QVector<quint16> finalPixel(4, 0xFFFF);

    if (lightSources.size() == 0)
        return finalPixel;
    
    //  if ()
    // Algorithm begins, Phong Illumination Model
    normal_vector.setX(r*2-1.0);
    normal_vector.setY(-(g*2-1.0));
    normal_vector.setZ(b*2-1.0);
    //normal_vector.normalize();

    // PREPARE ALGORITHM HERE

    finalPixel = IlluminatePixel();
    
    return finalPixel;
}
