/*
 * Copyright (c) 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <QFile>
#include <QFileInfo>

#include <KoColorConversions.h>
#include <kis_my_paintop_option.h>
#include <libmypaint/mypaint-brush.h>
#include <KisResourceServerProvider.h>
#include <KoColorModelStandardIds.h>

#include "kis_mypaint_brush.h"
#include "kis_my_paintop_settings.h"

class KisMyPaintBrush::Private {

public:
    MyPaintBrush *m_brush;
    QImage m_icon;
    QByteArray m_json;
    float diameter;
    float hardness;
    float opacity;
    float offset;
    float isEraser;
};

KisMyPaintBrush::KisMyPaintBrush(const QString &fileName)
    : KisPaintOpPreset (fileName), m_d(new Private) {

    m_d->m_brush = mypaint_brush_new();
    mypaint_brush_from_defaults(m_d->m_brush);
}

KisMyPaintBrush::~KisMyPaintBrush() {

    mypaint_brush_unref(m_d->m_brush);
    delete m_d;
}

void KisMyPaintBrush::setColor(const KoColor color, const KoColorSpace *colorSpace) {

    float hue, saturation, value;
    qreal r, g, b;
    QColor dstColor;

    if (colorSpace->colorModelId() == RGBAColorModelID) {
        colorSpace->toQColor(color.data(), &dstColor, colorSpace->profile());
        dstColor.getRgbF(&r, &g, &b);
    }

    RGBToHSV(r, g, b, &hue, &saturation, &value);

    mypaint_brush_set_base_value(m_d->m_brush, MYPAINT_BRUSH_SETTING_COLOR_H, (hue)/360);
    mypaint_brush_set_base_value(m_d->m_brush, MYPAINT_BRUSH_SETTING_COLOR_S, (saturation));
    mypaint_brush_set_base_value(m_d->m_brush, MYPAINT_BRUSH_SETTING_COLOR_V, (value));
}

void KisMyPaintBrush::apply(KisPaintOpSettingsSP settings) {

    if(settings->getProperty(MYPAINT_JSON).isNull()) {
        mypaint_brush_from_defaults(m_d->m_brush);
    }
    else {
        QByteArray ba = settings->getProperty(MYPAINT_JSON).toByteArray();
        mypaint_brush_from_string(m_d->m_brush, ba);
    }

    float diameter = settings->getFloat(MYPAINT_DIAMETER);
    m_d->diameter = diameter;
    mypaint_brush_set_base_value(m_d->m_brush, MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC, log(diameter/2));

    float hardness = settings->getFloat(MYPAINT_HARDNESS);
    m_d->hardness = hardness;
    mypaint_brush_set_base_value(m_d->m_brush, MYPAINT_BRUSH_SETTING_HARDNESS, hardness);

    float opacity = settings->getFloat(MYPAINT_OPACITY);
    m_d->opacity = opacity;
    mypaint_brush_set_base_value(m_d->m_brush, MYPAINT_BRUSH_SETTING_OPAQUE, opacity);

    float offset = settings->getFloat(MYPAINT_OFFSET_BY_RANDOM);
    m_d->offset = offset;
    mypaint_brush_set_base_value(m_d->m_brush, MYPAINT_BRUSH_SETTING_OFFSET_BY_RANDOM, offset);

    mypaint_brush_new_stroke(m_d->m_brush);
}

MyPaintBrush* KisMyPaintBrush::brush() {

    return m_d->m_brush;
}

bool KisMyPaintBrush::loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface)
{
    Q_UNUSED(resourcesInterface);


    QFileInfo fileInfo(filename());
    m_d->m_icon.load(fileInfo.path() + '/' + fileInfo.baseName() + "_prev.png");

    setImage(m_d->m_icon);

    QByteArray ba = dev->readAll();
    m_d->m_json = ba;
    mypaint_brush_from_string(m_d->m_brush, ba);
    m_d->diameter = 2*exp(mypaint_brush_get_base_value(m_d->m_brush, MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC));
    m_d->hardness = mypaint_brush_get_base_value(m_d->m_brush, MYPAINT_BRUSH_SETTING_HARDNESS);
    m_d->opacity = mypaint_brush_get_base_value(m_d->m_brush, MYPAINT_BRUSH_SETTING_OPAQUE);
    m_d->offset = mypaint_brush_get_base_value(m_d->m_brush, MYPAINT_BRUSH_SETTING_OFFSET_BY_RANDOM);
    m_d->isEraser = mypaint_brush_get_base_value(m_d->m_brush, MYPAINT_BRUSH_SETTING_ERASER);


    return true;
}

bool KisMyPaintBrush::save() {

    return false;
}

void KisMyPaintBrush::reloadSettings() {

    QFileInfo fileInfo(this->filename());

    KisPaintOpSettingsSP s = new KisMyPaintOpSettings(settings()->resourcesInterface());
    s->setProperty("paintop", "mypaintbrush");
    s->setProperty("filename", this->filename());
    s->setProperty(MYPAINT_JSON, this->getJsonData());
    s->setProperty(MYPAINT_DIAMETER, this->getSize());
    s->setProperty(MYPAINT_HARDNESS, this->getHardness());
    s->setProperty(MYPAINT_OPACITY, this->getOpacity());
    s->setProperty(MYPAINT_OFFSET_BY_RANDOM, this->getOffset());
    s->setProperty(MYPAINT_ERASER, this->isEraser());
    s->setProperty("EraserMode", qRound(this->isEraser()));

    this->setSettings(s);
}

QByteArray KisMyPaintBrush::getJsonData() {

    return m_d->m_json;
}

float KisMyPaintBrush::getSize() {

    return m_d->diameter;
}

float KisMyPaintBrush::getHardness() {

    return m_d->hardness;
}

float KisMyPaintBrush::getOpacity() {

    return m_d->opacity;
}

float KisMyPaintBrush::getOffset() {

    return m_d->offset;
}

float KisMyPaintBrush::isEraser() {

    return m_d->isEraser;
}
