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

#ifndef KIS_SUMI_SHAPE_OPTION_H
#define KIS_SUMI_SHAPE_OPTION_H

#include <kis_paintop_option.h>
#include <krita_export.h>

class KisShapeOptionsWidget;

class KisSumiShapeOption : public KisPaintOpOption
{
public:
    KisSumiShapeOption();
    ~KisSumiShapeOption();

    void setRadius(int radius) const;
    void setScaleFactor(qreal scale) const;
    
    int radius() const;
    double sigma() const;
    int brushDimension() const;
    bool mousePressure() const;

    double scaleFactor() const;
    double shearFactor() const;
    double randomFactor() const;

    void writeOptionSetting(KisPropertiesConfiguration* config) const;
    void readOptionSetting(const KisPropertiesConfiguration* config);
private:
    KisShapeOptionsWidget * m_options;
};

#endif // KIS_SUMI_SHAPE_OPTION_H

