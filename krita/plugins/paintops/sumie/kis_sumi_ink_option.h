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

#ifndef KIS_SUMI_INK_OPTION_H
#define KIS_SUMI_INK_OPTION_H

#include <kis_paintop_option.h>
#include <krita_export.h>

class KisInkOptionsWidget;

class KisSumiInkOption : public KisPaintOpOption
{
public:
    KisSumiInkOption();
    ~KisSumiInkOption();

    int inkAmount() const;
    QList<float> curve() const;

    bool useSaturation() const;
    bool useOpacity() const;
    bool useWeights() const;

    int pressureWeight() const;
    int bristleLengthWeight() const;
    int bristleInkAmountWeight() const;
    int inkDepletionWeight() const;

    int m_curveSamples;
    QList<QPointF> m_curveData;

    void writeOptionSetting(KisPropertiesConfiguration* config) const;
    void readOptionSetting(const KisPropertiesConfiguration* config);
private:
    KisInkOptionsWidget * m_options;
};

#endif // KIS_SUMI_SHAPE_OPTION_H

