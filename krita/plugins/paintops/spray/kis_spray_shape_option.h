/*
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KIS_SPRAY_SHAPE_OPTION_H
#define KIS_SPRAY_SHAPE_OPTION_H

#include <kis_paintop_option.h>

class KisShapeOptionsWidget;

class KisSprayShapeOption : public KisPaintOpOption
{

    Q_OBJECT

public:
    KisSprayShapeOption();
    ~KisSprayShapeOption();

    /// 0 - ellipse, 1 - rectangle, 2 - anti-aliased pixel, 2 - pixel
    int shape() const;

    /// distribution settings
    bool gaussian() const;
    
    /// size settings
    bool proportional() const;
    qreal widthPerc() const;
    qreal heightPerc() const;
    int width() const;
    int height() const;
    /// random size 
    bool jitterShapeSize() const;

    bool fixedRotation() const;
    int fixedAngle() const;
   
    bool randomRotation() const;
    qreal randomRotationWeight() const;
    
    bool followCursor() const;
    qreal followCursorWeigth() const;
    
    QString path() const;
    
    /// TODO
    void writeOptionSetting(KisPropertiesConfiguration* setting) const;
    /// TODO
    void readOptionSetting(const KisPropertiesConfiguration* setting);
private:
    KisShapeOptionsWidget * m_options;

private slots:
            void randomValueChanged(int value);
            void followValueChanged(int value);
};

#endif // KIS_SPRAY_SHAPE_OPTION_H

