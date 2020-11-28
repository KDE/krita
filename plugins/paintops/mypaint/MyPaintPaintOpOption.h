/*
 * SPDX-FileCopyrightText: 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_MY_PAINTOP_OPTION_H
#define KIS_MY_PAINTOP_OPTION_H

#include <KisPaintopPropertiesBase.h>
#include <kis_properties_configuration.h>
#include <kis_paintop_option.h>
#include <kis_slider_spin_box.h>

const QString MYPAINT_DIAMETER = "MyPaint/diameter";
const QString MYPAINT_HARDNESS = "MyPaint/hardness";
const QString MYPAINT_OPACITY = "MyPaint/opcity";
const QString MYPAINT_ERASER = "MyPaint/eraser";
const QString MYPAINT_JSON = "MyPaint/json";
const QString MYPAINT_BRUSH = "MyPaint/brush";
const QString MYPAINT_OFFSET_BY_RANDOM = "MyPaint/Offset";

class KisMyPaintOpOptionsWidget;

class KisMyPaintOpOption : public KisPaintOpOption
{
public:
    KisMyPaintOpOption();
    ~KisMyPaintOpOption() override;

    void setRadius(int radius) const;
    int radius() const;

    void setHardness(int hardness) const;
    int hardness() const;

    void setOpacity(int opacity) const;
    int opacity() const;

    void setEraser(bool isEraser) const;
    bool eraser() const;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;
    void lodLimitations(KisPaintopLodLimitations *l) const override;

    void refresh();
    KisDoubleSliderSpinBox* radiusSlider();
    KisDoubleSliderSpinBox* hardnessSlider();
    KisDoubleSliderSpinBox* opacitySlider();

private:
    KisMyPaintOpOptionsWidget *m_options;
    QByteArray json;
    float eraserVal;

};

class KisMyPaintOptionProperties: public KisPaintopPropertiesBase
{
public:
    float radius() const {
        return log(diameter/2);
    }

    void readOptionSettingImpl(const KisPropertiesConfiguration *settings) override {

        hardness = settings->getFloat(MYPAINT_HARDNESS);
        eraserMode = settings->getBool("EraserMode");
        opacity = settings->getFloat(MYPAINT_OPACITY);
        diameter = settings->getFloat(MYPAINT_DIAMETER);
        json = settings->getProperty(MYPAINT_JSON).toByteArray();
        eraser = settings->getFloat(MYPAINT_ERASER);
        offset = settings->getFloat(MYPAINT_OFFSET_BY_RANDOM);

    }

    void writeOptionSettingImpl(KisPropertiesConfiguration *settings) const override {

        settings->setProperty(MYPAINT_DIAMETER, diameter);
        settings->setProperty(MYPAINT_ERASER, eraser);
        settings->setProperty(MYPAINT_OPACITY, opacity);
        settings->setProperty(MYPAINT_HARDNESS, hardness);
        settings->setProperty(MYPAINT_JSON, json);
        settings->setProperty("EraserMode", eraserMode);
        settings->setProperty(MYPAINT_OFFSET_BY_RANDOM, offset);
    }


public:
    float diameter;
    float hardness;
    float opacity;
    float eraser;
    float offset;
    bool eraserMode;
    QByteArray json;

};

#endif // KIS_MY_PAINTOP_OPTION_H
