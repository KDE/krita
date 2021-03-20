/*
 * SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_COLOR_OPTION_H
#define KIS_COLOR_OPTION_H

#include <kis_paintop_option.h>
#include <kritapaintop_export.h>

const QString COLOROP_HUE = "ColorOption/hue";
const QString COLOROP_SATURATION = "ColorOption/saturation";
const QString COLOROP_VALUE = "ColorOption/value";

const QString COLOROP_USE_RANDOM_HSV = "ColorOption/useRandomHSV";
const QString COLOROP_USE_RANDOM_OPACITY = "ColorOption/useRandomOpacity";
const QString COLOROP_SAMPLE_COLOR = "ColorOption/sampleInputColor";

const QString COLOROP_FILL_BG = "ColorOption/fillBackground";
const QString COLOROP_COLOR_PER_PARTICLE = "ColorOption/colorPerParticle";
const QString COLOROP_MIX_BG_COLOR = "ColorOption/mixBgColor";

class KisColorOptionsWidget;

class PAINTOP_EXPORT KisColorOption : public KisPaintOpOption
{
    Q_OBJECT
public:
    KisColorOption();
    ~KisColorOption() override;

    bool useRandomHSV() const;
    bool useRandomOpacity() const;
    bool sampleInputColor() const;

    bool fillBackground() const;
    bool colorPerParticle() const;
    bool mixBgColor() const;

    // TODO: these should be intervals like 20..180
    int hue() const;
    int saturation() const;
    int value() const;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private Q_SLOTS:

    void setHSVEnabled(bool);

private:


    KisColorOptionsWidget * m_options;
};

class PAINTOP_EXPORT KisColorProperties
{
public:
    bool useRandomHSV;
    bool useRandomOpacity;
    bool sampleInputColor;

    bool fillBackground;
    bool colorPerParticle;
    bool mixBgColor;

    int hue;
    int saturation;
    int value;
public:
    /// fill the class members with related properties
    void fillProperties(const KisPropertiesConfigurationSP setting);
};

#endif // KIS_COLOR_OPTION_H

