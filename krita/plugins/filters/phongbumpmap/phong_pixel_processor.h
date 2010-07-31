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

#include <QVector3D>
#include <QTime>
#include <QImage>
#include <QColor>
#include <QList>
#include <QPair>
#include <QMap>

struct Illuminant
{
    QList<qreal> RGBvalue;
    QVector3D lightVector;
};

class PhongPixelProcessor
{
    
public:
    PhongPixelProcessor(quint8* hmap[]);
    PhongPixelProcessor(quint8 hmap[]);
    ~PhongPixelProcessor();
    
    void initialize();
    
    QVector3D reflection_vector;
    QVector3D normal_vector;
    QVector3D x_vector;
    QVector3D y_vector;
    QVector3D light_vector;
    QVector3D vision_vector;
    
    quint8** fastHeightmap;
    quint8* heightmap;
    
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
    
    QRgb reallyFastIlluminatePixel(quint16 upx, quint16 upy, quint16 downx, quint16 downy, quint16 leftx, quint16 lefty, quint16 rightx, quint16 righty);
    QRgb fastIlluminatePixel(QPoint posup, QPoint posdown, QPoint posleft, QPoint posright);
    QColor illuminatePixel(quint32 posup, quint32 posdown, quint32 posleft, quint32 posright);
    
    void setLightVector(QVector3D light_vector);
    
    QList<Illuminant> lightSources;
    Illuminant fastLight;
    Illuminant fastLight2;
};


/*
struct Material
{
    ///Ambient light coefficient
    qreal Ka;
    
    ///Diffuse light coefficient
    qreal Kd;
    
    ///Specular light coefficient
    qreal Ks;
    
    ///Shinyness exponent
    qreal shiny_exp;
};

struct pseudo3DPixel
{
    QVector3D reflection_vector;
    QVector3D normal_vector;
    QVector3D x_vector;
    QVector3D y_vector;
    QVector3D light_vector;
    QVector3D vision_vector;
    
    quint8* heightmap;
};
*/
