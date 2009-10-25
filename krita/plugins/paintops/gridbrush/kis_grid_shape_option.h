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

#ifndef KIS_GRID_SHAPE_OPTION_H
#define KIS_GRID_SHAPE_OPTION_H

#include <kis_paintop_option.h>

class KisShapeOptionsWidget;

class KisGridShapeOption : public KisPaintOpOption
{
public:
    KisGridShapeOption();
    ~KisGridShapeOption();

    /// Ellipse, rectangle, line, pixel, anti-aliased pixel
    int shape() const;

    /// TODO
    void writeOptionSetting(KisPropertiesConfiguration* setting) const;
    /// TODO
    void readOptionSetting(const KisPropertiesConfiguration* setting);
private:
   KisShapeOptionsWidget * m_options;
};

#endif // KIS_GRID_SHAPE_OPTION_H

