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

#ifndef KIS_HAIRY_INK_OPTION_H
#define KIS_HAIRY_INK_OPTION_H

#include <kis_paintop_option.h>
#include <krita_export.h>

const QString HAIRY_INK_DEPLETION_ENABLED = "HairyInk/enabled";
const QString HAIRY_INK_AMOUNT = "HairyInk/inkAmount";
const QString HAIRY_INK_USE_SATURATION = "HairyInk/useSaturation";
const QString HAIRY_INK_USE_OPACITY = "HairyInk/useOpacity";
const QString HAIRY_INK_USE_WEIGHTS = "HairyInk/useWeights";
const QString HAIRY_INK_PRESSURE_WEIGHT = "HairyInk/pressureWeights";
const QString HAIRY_INK_BRISTLE_LENGTH_WEIGHT = "HairyInk/bristleLengthWeights";
const QString HAIRY_INK_BRISTLE_INK_AMOUNT_WEIGHT = "HairyInk/bristleInkAmountWeight";
const QString HAIRY_INK_DEPLETION_WEIGHT = "HairyInk/inkDepletionWeight";
const QString HAIRY_INK_DEPLETION_CURVE = "HairyInk/inkDepletionCurve";
const QString HAIRY_INK_SOAK = "HairyInk/soak";

class KisInkOptionsWidget;

class KisHairyInkOption : public KisPaintOpOption
{
public:
    KisHairyInkOption();
    ~KisHairyInkOption();

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

    void writeOptionSetting(KisPropertiesConfiguration* config) const;
    void readOptionSetting(const KisPropertiesConfiguration* config);
private:
    KisInkOptionsWidget * m_options;
};

#endif // KIS_HAIRY_SHAPE_OPTION_H

