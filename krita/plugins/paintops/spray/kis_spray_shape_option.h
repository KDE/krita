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
#include <QImage>

const QString SPRAYSHAPE_SHAPE = "SprayShape/shape";
const QString SPRAYSHAPE_PROPORTIONAL = "SprayShape/proportional";
const QString SPRAYSHAPE_WIDTH = "SprayShape/width";
const QString SPRAYSHAPE_HEIGHT = "SprayShape/height";
const QString SPRAYSHAPE_RANDOM_SIZE = "SprayShape/randomSize";
const QString SPRAYSHAPE_FIXED_ROTATION = "SprayShape/fixedRotation";
const QString SPRAYSHAPE_FIXED_ANGEL = "SprayShape/fixedAngle";
const QString SPRAYSHAPE_RANDOM_ROTATION = "SprayShape/randomRotation";
const QString SPRAYSHAPE_RANDOM_ROTATION_WEIGHT = "SprayShape/randomRotationWeight";
const QString SPRAYSHAPE_FOLLOW_CURSOR = "SprayShape/followCursor";
const QString SPRAYSHAPE_FOLLOW_CURSOR_WEIGHT = "SprayShape/followCursorWeigth";
const QString SPRAYSHAPE_DRAWING_ANGLE = "SprayShape/followDrawingAngle";
const QString SPRAYSHAPE_DRAWING_ANGLE_WEIGHT = "SprayShape/followDrawingAngleWeigth";
const QString SPRAYSHAPE_IMAGE_URL = "SprayShape/imageUrl";



class KisShapeOptionsWidget;

class KisSprayShapeOption : public KisPaintOpOption
{
    Q_OBJECT
public:
    KisSprayShapeOption();
    ~KisSprayShapeOption();

    /// 0 - ellipse, 1 - rectangle, 2 - anti-aliased pixel, 2 - pixel
    int shape() const;
    
    /// size settings
    QImage image() const { return m_image; }

    void writeOptionSetting(KisPropertiesConfiguration* setting) const;
    void readOptionSetting(const KisPropertiesConfiguration* setting);

private:
    KisShapeOptionsWidget * m_options;
    QImage m_image;
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

#endif // KIS_SPRAY_SHAPE_OPTION_H

