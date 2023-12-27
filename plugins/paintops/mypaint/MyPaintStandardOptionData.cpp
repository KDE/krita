/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <MyPaintStandardOptionData.h>

namespace deprecated_remove_after_krita6 {

/**
 * Before Krita 5.2 MyPaint brushes saved into .kpp files used to have
 * separate properties for hardness, opacity and diameter. They are gone
 * with Krita 5.2, but to make sure new brush presets can be open with the
 * older versions of Krita, we should keep them for some time.
 */

const QString MYPAINT_HARDNESS = "MyPaint/hardness";
const QString MYPAINT_OPACITY = "MyPaint/opcity";
const QString MYPAINT_DIAMETER = "MyPaint/diameter";

class SensorPackOpacity : public MyPaintSensorPack
{
public:
    KisSensorPackInterface * clone() const override
    {
        return new SensorPackOpacity(*this);
    }

    void write(const KisCurveOptionDataCommon &data, KisPropertiesConfiguration *setting) const override
    {
        MyPaintSensorPack::write(data, setting);
        setting->setProperty(MYPAINT_OPACITY, data.strengthValue);
    }
};

class SensorPackHardness : public MyPaintSensorPack
{
public:
    KisSensorPackInterface * clone() const override
    {
        return new SensorPackHardness(*this);
    }
    void write(const KisCurveOptionDataCommon &data, KisPropertiesConfiguration *setting) const override
    {
        MyPaintSensorPack::write(data, setting);
        setting->setProperty(MYPAINT_HARDNESS, data.strengthValue);
    }
};


class SensorPackRadiusLogarithmic : public MyPaintSensorPack
{
public:
    KisSensorPackInterface * clone() const override {
        return new SensorPackRadiusLogarithmic(*this);
    }
    void write(const KisCurveOptionDataCommon &data, KisPropertiesConfiguration *setting) const override
    {
        MyPaintSensorPack::write(data, setting);
        setting->setProperty(MYPAINT_DIAMETER, 2.0 * exp(data.strengthValue));
    }
};

} // namespace deprecated_remove_after_krita6

MyPaintRadiusLogarithmicData::MyPaintRadiusLogarithmicData()
    : MyPaintCurveOptionData("",
                             KoID("radius_logarithmic",
                                  i18n("Radius Logarithmic")),
                             false, true, 0.01, 8.0,
                             new deprecated_remove_after_krita6::SensorPackRadiusLogarithmic())
{
}

MyPaintHardnessData::MyPaintHardnessData()
    : MyPaintCurveOptionData("",
                             KoID("hardness", i18n("Hardness")),
                             false, true, 0.02, 1.0,
                             new deprecated_remove_after_krita6::SensorPackHardness())
{
}

MyPaintOpacityData::MyPaintOpacityData()
    : MyPaintCurveOptionData("",
                             KoID("opaque", i18n("Opaque")),
                             false, true, 0.0, 2.0,
                             new deprecated_remove_after_krita6::SensorPackOpacity())
{
}
