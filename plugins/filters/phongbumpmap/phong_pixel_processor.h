/*
*  Copyright (c) 2010-2011 José Luis Vergara <pentalis@gmail.com>
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
#ifndef PHONG_PIXEL_PROCESSOR_H
#define PHONG_PIXEL_PROCESSOR_H

#include <QVector3D>
#include <QTime>
#include <QImage>
#include <QColor>
#include <QList>
#include <QPair>
#include <QMap>

#include "phong_bumpmap_constants.h"
#include "kis_properties_configuration.h"

struct Illuminant
{
    QList<qreal> RGBvalue;
    QVector3D lightVector;
};

class PhongPixelProcessor
{
    
public:
    PhongPixelProcessor(quint32 pixelArea, const KisPropertiesConfiguration* config);
    ~PhongPixelProcessor();
    
    void initialize(const KisPropertiesConfiguration* config);
    void normalizeHeightmap();
    
    QVector3D reflection_vector;
    QVector3D normal_vector;
    QVector3D x_vector;
    QVector3D y_vector;
    QVector3D light_vector;
    QVector3D vision_vector;
    
    QVector<double> realheightmap;
    
    ///Ambient light coefficient
    qreal Ka;
    
    ///Diffuse light coefficient
    qreal Kd;
    
    ///Specular light coefficient
    qreal Ks;
    
    ///Shinyness exponent
    qreal shiny_exp;
    
    ///Total ambient light
    qreal Ia;
    
    ///Total diffuse light
    qreal Id;
    
    ///Total specular light
    qreal Is;
    
    QVector<quint16> IlluminatePixelFromHeightmap(quint32 posup, quint32 posdown, quint32 posleft, quint32 posright);
    QVector<quint16> IlluminatePixel();
    QVector<quint16> IlluminatePixelFromNormalmap(qreal r, qreal g, qreal b);
    
    void setLightVector(QVector3D light_vector);
    
    ///Light sources to use (those disabled in the GUI are not present here)
    QList<Illuminant> lightSources;
    
    ///Size of this stuff
    quint8 size;
    
    Illuminant fastLight;
    Illuminant fastLight2;
    
    bool diffuseLightIsEnabled;
    bool specularLightIsEnabled;

private:
    quint32 m_pixelArea;
};


#endif
