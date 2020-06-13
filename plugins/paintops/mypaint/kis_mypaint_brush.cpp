#include "kis_mypaint_brush.h"

#include <KoColorConversions.h>
#include <QFile>

KisMyPaintBrush::KisMyPaintBrush(KisPainter *painter, QObject *parent) : QObject(parent)
{
    m_painter = painter;
    m_brush = mypaint_brush_new();
    mypaint_brush_from_defaults(m_brush);
}

void KisMyPaintBrush::setColor(const KoColor color) {

    float hue, saturation, value;
    qreal r, g, b;

    color.toQColor().getRgbF(&r ,&g ,&b);
    RGBToHSV(r , g, b, &hue, &saturation, &value);

    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_COLOR_H, (hue)/360);
    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_COLOR_S, (saturation));
    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_COLOR_V, (value));
}

void KisMyPaintBrush::apply(KisPaintOpSettingsSP settings) {

//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC, -1.00);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_OPAQUE, 0.6);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_HARDNESS, 0.64);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_ERASER, false);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_SMUDGE, 0.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_COLOR_H, 0.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_COLOR_S, 0.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_COLOR_V, 0.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_COLORIZE, 0.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_LOCK_ALPHA, 0.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_SPEED1_GAMMA, 4.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_SPEED2_GAMMA, 4.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_ANTI_ALIASING, 1.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_RESTORE_COLOR, 0.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_SLOW_TRACKING, 0.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_SMUDGE_LENGTH, 0.5);
//    //mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_SNAP_TO_PIXEL, false);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_CHANGE_COLOR_H, 0.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_CHANGE_COLOR_L, 0.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_CHANGE_COLOR_V, 0.185);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_TRACKING_NOISE, 0.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_DABS_PER_SECOND, 0.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_OFFSET_BY_SPEED, 0.17);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_OPAQUE_MULTIPLY, 0.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_SPEED1_SLOWNESS, 0.09);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_SPEED2_SLOWNESS, 1.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_STROKE_HOLDTIME, 0.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_DIRECTION_FILTER, 2.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_OFFSET_BY_RANDOM, 2.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_OPAQUE_LINEARIZE, 0.9);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_RADIUS_BY_RANDOM, 0.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_STROKE_THRESHOLD, 0.0);
//    //mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_PRESSURE_GAIN_LOG, false);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_SMUDGE_RADIUS_LOG, 0.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_CHANGE_COLOR_HSL_S, 0.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_CHANGE_COLOR_HSV_S, 0.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_ELLIPTICAL_DAB_ANGLE, 90.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_ELLIPTICAL_DAB_RATIO, 1.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_CUSTOM_INPUT_SLOWNESS, 0.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_DABS_PER_BASIC_RADIUS, 0.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_SLOW_TRACKING_PER_DAB, 0.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_DABS_PER_ACTUAL_RADIUS, 2.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_OFFSET_BY_SPEED_SLOWNESS, 1.0);
//    mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_STROKE_DURATION_LOGARITHMIC, 4.0);

    QIODevice *dev = new QFile("/usr/share/mypaint-data/1.0/brushes/experimental/bubble.myb");

    if (!dev->open(QIODevice::ReadOnly)) {
        qDebug() << "Can't open file ";
        delete dev;
    }
    else {
        QByteArray ba = dev->readAll();
        mypaint_brush_from_string(m_brush, ba);
    }
    mypaint_brush_new_stroke(m_brush);

}

MyPaintBrush* KisMyPaintBrush::brush() {

    return m_brush;
}
