#include "kis_mypaint_brush.h"

#include <KoColorConversions.h>
#include <QFile>
#include <QFileInfo>
#include <kis_my_paintop_option.h>
#include <libmypaint/mypaint-brush.h>
#include <KisResourceServerProvider.h>

class KisMyPaintBrush::Private {

public:
    MyPaintBrush *m_brush;
    QImage m_icon;
    QByteArray m_json;
    float diameter;
    float hardness;
    float opacity;
    float isEraser;
};

KisMyPaintBrush::KisMyPaintBrush(const QString &fileName)
    : KoResource (fileName), m_d(new Private) {

    m_d->m_brush = mypaint_brush_new();
    mypaint_brush_from_defaults(m_d->m_brush);
}

void KisMyPaintBrush::setColor(const KoColor color) {

    float hue, saturation, value;
    qreal r, g, b;

    color.toQColor().getRgbF(&r ,&g ,&b);
    RGBToHSV(r , g, b, &hue, &saturation, &value);

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
    mypaint_brush_set_base_value(m_d->m_brush, MYPAINT_BRUSH_SETTING_HARDNESS, hardness);

    float opacity = settings->getFloat(MYPAINT_OPACITY);
    mypaint_brush_set_base_value(m_d->m_brush, MYPAINT_BRUSH_SETTING_OPAQUE, opacity);

    mypaint_brush_new_stroke(m_d->m_brush);
}

MyPaintBrush* KisMyPaintBrush::brush() {

    return m_d->m_brush;
}

bool KisMyPaintBrush::load() {

    dbgImage << "Load MyPaint Brush " << filename();
    setValid(false);

    if (filename().isEmpty()) {
        return false;
    }

    QIODevice *dev = 0;
    QByteArray ba;

    dev = new QFile(filename());

    if (dev->size() == 0)
    {
        delete dev;
        return false;
    }

    if (!dev->open(QIODevice::ReadOnly)) {
        warnKrita << "Can't open file " << filename();
        delete dev;
        return false;
    }

    bool res = false;
    res = loadFromDevice(dev);

    delete dev;
    setValid(res);

    return res;
}

bool KisMyPaintBrush::loadFromDevice(QIODevice *dev) {

    QFileInfo fileInfo(filename());
    m_d->m_icon.load(fileInfo.path() + '/' + fileInfo.baseName() + "_prev.png");

    setImage(m_d->m_icon);

    QByteArray ba = dev->readAll();
    m_d->m_json = ba;
    mypaint_brush_from_string(m_d->m_brush, ba);
    m_d->diameter = 2*exp(mypaint_brush_get_base_value(m_d->m_brush, MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC));
    m_d->hardness = mypaint_brush_get_base_value(m_d->m_brush, MYPAINT_BRUSH_SETTING_HARDNESS);
    m_d->opacity = mypaint_brush_get_base_value(m_d->m_brush, MYPAINT_BRUSH_SETTING_OPAQUE);
    m_d->isEraser = mypaint_brush_get_base_value(m_d->m_brush, MYPAINT_BRUSH_SETTING_ERASER);

    return true;
}

bool KisMyPaintBrush::save() {

    return false;
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

float KisMyPaintBrush::isEraser() {

    return m_d->isEraser;
}
