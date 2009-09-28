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
public:
    KisSprayShapeOption();
    ~KisSprayShapeOption();

    int width() const;
    int height() const;

    /// 0 - shape, 1 - particle, 2 - pixel
    int object() const; 

    /// 0 - ellipse, 1 - rectangle, 2 - metaball
    int shape() const;

    bool jitterShapeSize() const;

    bool highRendering() const;
    bool proportional() const;
    bool gaussian() const;
    
    qreal widthPerc() const;
    qreal heightPerc() const;

    qreal minTresh() const;
    qreal maxTresh() const;

    /// TODO
    void writeOptionSetting(KisPropertiesConfiguration* setting) const;
    /// TODO
    void readOptionSetting(const KisPropertiesConfiguration* setting);
private:
   KisShapeOptionsWidget * m_options;
};

#endif // KIS_SPRAY_SHAPE_OPTION_H

