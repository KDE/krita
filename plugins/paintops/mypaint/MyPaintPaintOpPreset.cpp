/*
 * SPDX-FileCopyrightText: 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "MyPaintPaintOpPreset.h"

#include <KisResourceLocator.h>
#include <KisResourceServerProvider.h>
#include <KoColorConversions.h>
#include <KoColorModelStandardIds.h>
#include <MyPaintPaintOpOption.h>
#include <QFile>
#include <QFileInfo>
#include <libmypaint/mypaint-brush.h>

#include "MyPaintPaintOpSettings.h"

class KisMyPaintPaintOpPreset::Private {

public:
    MyPaintBrush *brush;
    QImage icon;
    QByteArray json;
    float diameter;
    float hardness;
    float opacity;
    float offset;
    float isEraser;
};

KisMyPaintPaintOpPreset::KisMyPaintPaintOpPreset(const QString &fileName)
    : KisPaintOpPreset (fileName), d(new Private) {

    d->brush = mypaint_brush_new();
    mypaint_brush_from_defaults(d->brush);
}

KisMyPaintPaintOpPreset::~KisMyPaintPaintOpPreset() {

    mypaint_brush_unref(d->brush);
    delete d;
}

void KisMyPaintPaintOpPreset::setColor(const KoColor color, const KoColorSpace *colorSpace) {

    float hue, saturation, value;
    qreal r, g, b;
    QColor dstColor;

    if (colorSpace->colorModelId() == RGBAColorModelID) {
        colorSpace->toQColor(color.data(), &dstColor, colorSpace->profile());
        dstColor.getRgbF(&r, &g, &b);
    }

    RGBToHSV(r, g, b, &hue, &saturation, &value);

    mypaint_brush_set_base_value(d->brush, MYPAINT_BRUSH_SETTING_COLOR_H, (hue)/360);
    mypaint_brush_set_base_value(d->brush, MYPAINT_BRUSH_SETTING_COLOR_S, (saturation));
    mypaint_brush_set_base_value(d->brush, MYPAINT_BRUSH_SETTING_COLOR_V, (value));
}

void KisMyPaintPaintOpPreset::apply(KisPaintOpSettingsSP settings) {

    if(settings->getProperty(MYPAINT_JSON).isNull()) {
        mypaint_brush_from_defaults(d->brush);
    }
    else {
        QByteArray ba = settings->getProperty(MYPAINT_JSON).toByteArray();
        mypaint_brush_from_string(d->brush, ba);
    }

    float diameter = settings->getFloat(MYPAINT_DIAMETER);
    d->diameter = diameter;
    mypaint_brush_set_base_value(d->brush, MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC, log(diameter/2));

    float hardness = settings->getFloat(MYPAINT_HARDNESS);
    d->hardness = hardness;
    mypaint_brush_set_base_value(d->brush, MYPAINT_BRUSH_SETTING_HARDNESS, hardness);

    float opacity = settings->getFloat(MYPAINT_OPACITY);
    d->opacity = opacity;
    mypaint_brush_set_base_value(d->brush, MYPAINT_BRUSH_SETTING_OPAQUE, opacity);

    float offset = settings->getFloat(MYPAINT_OFFSET_BY_RANDOM);
    d->offset = offset;
    mypaint_brush_set_base_value(d->brush, MYPAINT_BRUSH_SETTING_OFFSET_BY_RANDOM, offset);

    mypaint_brush_new_stroke(d->brush);
}

MyPaintBrush* KisMyPaintPaintOpPreset::brush() {

    return d->brush;
}

bool KisMyPaintPaintOpPreset::loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface)
{
    // XXX: this breaks when myb files are in bundles!
    QString thumnailFile = KisResourceLocator::instance()->makeStorageLocationAbsolute(storageLocation()) + ResourceType::PaintOpPresets + '/' + QFileInfo(filename()).baseName() + "_prev.png";
    if (QFileInfo(thumnailFile).exists()) {
        d->icon.load(thumnailFile);
    }

    setImage(d->icon);

    QByteArray ba = dev->readAll();
    d->json = ba;
    mypaint_brush_from_string(d->brush, ba);
    d->diameter = 2*exp(mypaint_brush_get_base_value(d->brush, MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC));
    d->hardness = mypaint_brush_get_base_value(d->brush, MYPAINT_BRUSH_SETTING_HARDNESS);
    d->opacity = mypaint_brush_get_base_value(d->brush, MYPAINT_BRUSH_SETTING_OPAQUE);
    d->offset = mypaint_brush_get_base_value(d->brush, MYPAINT_BRUSH_SETTING_OFFSET_BY_RANDOM);
    d->isEraser = mypaint_brush_get_base_value(d->brush, MYPAINT_BRUSH_SETTING_ERASER);

    KisPaintOpSettingsSP s = new KisMyPaintOpSettings(resourcesInterface);
    s->setProperty("paintop", "mypaintbrush");
    s->setProperty("filename", this->filename());
    s->setProperty(MYPAINT_JSON, this->getJsonData());
    s->setProperty(MYPAINT_DIAMETER, this->getSize());
    s->setProperty(MYPAINT_HARDNESS, this->getHardness());
    s->setProperty(MYPAINT_OPACITY, this->getOpacity());
    s->setProperty(MYPAINT_OFFSET_BY_RANDOM, this->getOffset());
    s->setProperty(MYPAINT_ERASER, this->isEraser());
    s->setProperty("EraserMode", qRound(this->isEraser()));

    if (!metadata().contains("paintopid")) {
        addMetaData("paintopid", "mypaintbrush");
    }

    this->setSettings(s);
    setValid(true);

    return true;
}

bool KisMyPaintPaintOpPreset::save() {

    return false;
}

QByteArray KisMyPaintPaintOpPreset::getJsonData() {

    return d->json;
}

float KisMyPaintPaintOpPreset::getSize() {

    return d->diameter;
}

float KisMyPaintPaintOpPreset::getHardness() {

    return d->hardness;
}

float KisMyPaintPaintOpPreset::getOpacity() {

    return d->opacity;
}

float KisMyPaintPaintOpPreset::getOffset() {

    return d->offset;
}

float KisMyPaintPaintOpPreset::isEraser() {

    return d->isEraser;
}
