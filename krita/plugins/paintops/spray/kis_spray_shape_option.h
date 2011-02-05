/*
 *  Copyright (c) 2008,2009,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef KIS_SPRAY_SHAPE_OPTION_H
#define KIS_SPRAY_SHAPE_OPTION_H

#include <kis_paintop_option.h>

const QString SPRAYSHAPE_ENABLED = "SprayShape/enabled";
const QString SPRAYSHAPE_SHAPE = "SprayShape/shape";
const QString SPRAYSHAPE_PROPORTIONAL = "SprayShape/proportional";
const QString SPRAYSHAPE_WIDTH = "SprayShape/width";
const QString SPRAYSHAPE_HEIGHT = "SprayShape/height";
const QString SPRAYSHAPE_IMAGE_URL = "SprayShape/imageUrl";
const QString SPRAYSHAPE_USE_ASPECT = "SprayShape/useAspect";


class KisShapeOptionsWidget;

class KisSprayShapeOption : public KisPaintOpOption
{
    Q_OBJECT
public:
    KisSprayShapeOption();
    ~KisSprayShapeOption();

    /// 0 - ellipse, 1 - rectangle, 2 - anti-aliased pixel, 2 - pixel
    int shape() const;
    
    void writeOptionSetting(KisPropertiesConfiguration* setting) const;
    void readOptionSetting(const KisPropertiesConfiguration* setting);

private:
    KisShapeOptionsWidget * m_options;
    bool m_useAspect;
    qreal m_aspect;
    int m_maxSize;
    
private:
    void setupBrushPreviewSignals();
    void computeAspect();

private slots:
    void prepareImage();
    void aspectToggled(bool toggled);
    void updateHeight(int value);
    void updateWidth(int value);
    
    void changeSizeUI(bool proportionalSize);
};

#include <QImage>

class KisShapeProperties{
public:
    // particle type size
    quint8 shape;
    quint16 width;
    quint16 height;
    bool enabled;        
    bool proportional;
    // rotation
    QImage image;
    
public:
    
    void loadSettings(const KisPropertiesConfiguration* settings, qreal proportionalWidth, qreal proportionalHeight){
        enabled = settings->getBool(SPRAYSHAPE_ENABLED,true);
        
        width = settings->getInt(SPRAYSHAPE_WIDTH);
        height = settings->getInt(SPRAYSHAPE_HEIGHT);

        proportional = settings->getBool(SPRAYSHAPE_PROPORTIONAL);
        
        if (proportional)
        {   
            width = (width / 100.0) * proportionalWidth;
            height = (height / 100.0) * proportionalHeight;
        }
        // particle type size
        shape = settings->getInt(SPRAYSHAPE_SHAPE);
        // you have to check if the image is null in client
        image = QImage(settings->getString(SPRAYSHAPE_IMAGE_URL));
    }
};

#endif // KIS_SPRAY_SHAPE_OPTION_H

