/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "mypaint_brush_resource.h"

#include <QFileInfo>
#include <QFile>
#include <QString>
#include <QTextStream>

#include <klocale.h>
#include <kglobal.h>

#include "helpers.hpp"

class BrushInputDefinition {

public:

    BrushInputDefinition(QString _name,
                         float _hard_minimum,
                         float _soft_minimum,
                         float _normal,
                         float _soft_maximum,
                         float _hard_maximum,
                         QString _tooltip)
         : name(_name)
         , hard_minimum(_hard_minimum)
         , soft_minimum(_soft_minimum)
         , normal(_normal)
         , soft_maximum(_soft_maximum)
         , hard_maximum(_hard_maximum)
         , tooltip(_tooltip)
    {
    }

    int index;
    QString name;
    float hard_minimum;
    float soft_minimum;
    float normal;
    float soft_maximum;
    float hard_maximum;
    QString tooltip;
};

class BrushInputDefinitions {

public:

    QVector<BrushInputDefinition*> list;
    QMap<QString, BrushInputDefinition*> dict;
    ~BrushInputDefinitions()
    {
        dbgKrita << "Deleting static definitions for brush inputs";
        qDeleteAll(list);
        dict.clear();
    }

    BrushInputDefinitions()
    {
        dbgKrita << "Constructing static definitions for brush inputs";

        float None = -1;
        //  name, hard minimum, soft minimum, normal[1], soft maximum, hard maximum, tooltip

        list << new BrushInputDefinition("pressure", 0.0,  0.0,  0.4,  1.0, 1.0,  i18n("The pressure reported by the tablet, between 0.0 and 1.0. If you use the mouse, it will be 0.5 when a button is pressed and 0.0 otherwise."));
        list << new BrushInputDefinition("speed1",   None, 0.0,  0.5,  4.0, None, i18n("How fast you currently move. This can change very quickly. Try 'print input values' from the 'help' menu to get a feeling for the range; negative values are rare but possible for very low speed."));
        list << new BrushInputDefinition("speed2",   None, 0.0,  0.5,  4.0, None, i18n("Same as speed1, but changes slower. Also look at the 'speed2 filter' setting."));
        list << new BrushInputDefinition("random",   0.0,  0.0,  0.5,  1.0, 1.0,  i18n("Fast random noise, changing at each evaluation. Evenly distributed between 0 and 1."));
        list << new BrushInputDefinition("stroke",   0.0,  0.0,  0.5,  1.0, 1.0,  i18n("This input slowly goes from zero to one while you draw a stroke. It can also be configured to jump back to zero periodically while you move. Look at the 'stroke duration' and 'stroke hold time' settings."));
        list << new BrushInputDefinition("direction",0.0,  0.0,  0.0,  180.0, 180.0, i18n("The angle of the stroke, in degrees. The value will stay between 0.0 and 180.0, effectively ignoring turns of 180 degrees."));
        list << new BrushInputDefinition("custom",   None, -2.0,  0.0, +2.0, None, i18n("This is a user defined input. Look at the 'custom input' setting for details."));

        int i = 0;
        foreach(BrushInputDefinition* def, list) {
            def->index = i++;
            dict[def->name] = def;
        }

        // [1] If, for example, the user increases the "by pressure" slider
        // in the "radius" control, then this should change the reaction to
        // pressure and not the "normal" radius. To implement this, we need
        // a guess what the user considers to be normal pressure.
    }
};

class BrushSettingDefinition {

public:

    int index;
    QString cname;
    QString displayed_name;
    bool constant;
    float minimum;
    float default_value;
    float maximum;
    QString tooltip;

    BrushSettingDefinition( QString _cname,
                            QString _displayed_name,
                            bool _constant,
                            float _minimum,
                            float _default_value,
                            float _maximum,
                            QString _tooltip)
        : cname(_cname)
        , displayed_name(_displayed_name)
        , constant(_constant)
        , minimum(_minimum)
        , default_value(_default_value)
        , maximum(_maximum)
        , tooltip(_tooltip)
    {
    }

};

class BrushSettingDefinitions {

public:

    QVector<BrushSettingDefinition*> list;
    QMap<QString, BrushSettingDefinition*> dict;

    ~BrushSettingDefinitions()
    {
        dbgKrita << "Deleting static definitions for brush settings";
        qDeleteAll(list);
        dict.clear();
    }

    BrushSettingDefinitions()
    {
        // internal name, displayed name, constant, minimum, default, maximum, tooltip
        dbgKrita << "Constructing static definitions for brush settings";
        list << new BrushSettingDefinition("opaque", i18n("opaque"), false, 0.0, 1.0, 1.0, i18n("0 means brush is transparent, 1 fully visible\n(also known as alpha or opacity)"));
        list << new BrushSettingDefinition("opaque_multiply", i18n("opaque multiply"), false, 0.0, 0.0, 1.0, i18n("This gets multiplied with opaque. It is used for making opaque depend on pressure (or other inputs)."));
        list << new BrushSettingDefinition("opaque_linearize", i18n("opaque linearize"), true, 0.0, 0.9, 2.0, i18n("Correct the nonlinearity introduced by blending multiple dabs on top of each other. This correction should get you a linear (\"natural\") pressure response when pressure is mapped to opaque_multiply, as it is usually done. 0.9 is good for standard strokes, set it smaller if your brush scatters a lot, or higher if you use dabs_per_second.\n0.0 the opaque value above is for the individual dabs\n1.0 the opaque value above is for the final brush stroke, assuming each pixel gets (dabs_per_radius*2) brushdabs on average during a stroke"));
        list << new BrushSettingDefinition("radius_logarithmic", i18n("radius"), false, -2.0, 2.0, 5.0, i18n("basic brush radius (logarithmic)\n 0.7 means 2 pixels\n 3.0 means 20 pixels"));
        list << new BrushSettingDefinition("hardness", i18n("hardness"), false, 0.0, 0.8, 1.0, i18n("hard brush-circle borders (setting to zero will draw nothing)"));
        list << new BrushSettingDefinition("dabs_per_basic_radius", i18n("dabs per basic radius"), true, 0.0, 0.0, 6.0, i18n("how many dabs to draw while the pointer moves a distance of one brush radius (more precise: the base value of the radius)"));
        list << new BrushSettingDefinition("dabs_per_actual_radius", i18n("dabs per actual radius"), true, 0.0, 2.0, 6.0, i18n("same as above, but the radius actually drawn is used, which can change dynamically"));
        list << new BrushSettingDefinition("dabs_per_second", i18n("dabs per second"), true, 0.0, 0.0, 80.0, i18n("dabs to draw each second, no matter how far the pointer moves"));
        list << new BrushSettingDefinition("radius_by_random", i18n("radius by random"), false, 0.0, 0.0, 1.5, i18n("Alter the radius randomly each dab. You can also do this with the by_random input on the radius setting. If you do it here, there are two differences:\n1) the opaque value will be corrected such that a big-radius dabs is more transparent\n2) it will not change the actual radius seen by dabs_per_actual_radius"));
        list << new BrushSettingDefinition("speed1_slowness", i18n("speed1 filter"), false, 0.0, 0.04, 0.2, i18n("how slowly the input speed1 follows the real speed.\n0.0 changes immediately as your speed changes (not recommended, but try it.)"));
        list << new BrushSettingDefinition("speed2_slowness", i18n("speed2 filter"), false, 0.0, 0.8, 3.0, i18n("same as 'speed1 slowness' but note that the range is different"));
        list << new BrushSettingDefinition("speed1_gamma", i18n("speed1 gamma"), true, -8.0, 4.0, 8.0, i18n("This changes the reaction of the speed1 input to extreme physical speed. You will see the difference best if speed1 is mapped to the radius.\n-8.0 very fast speed does not increase speed1 much more\n+8.0 very fast speed increases speed1 a lot\nFor very slow speed the opposite happens."));
        list << new BrushSettingDefinition("speed2_gamma", i18n("speed2 gamma"), true, -8.0, 4.0, 8.0, i18n("same as 'speed1 gamma' for speed2"));
        list << new BrushSettingDefinition("offset_by_random", i18n("jitter"), false, 0.0, 0.0, 2.0, i18n("add a random offset to the position where each dab is drawn\n 0.0 disabled\n 1.0 standard deviation is one basic radius away\n<0.0 negative values produce no jitter"));
        list << new BrushSettingDefinition("offset_by_speed", i18n("offset by speed"), false, -3.0, 0.0, 3.0, i18n("change position depending on pointer speed\n= 0 disable\n> 0 draw where the pointer moves to\n< 0 draw where the pointer comes from"));
        list << new BrushSettingDefinition("offset_by_speed_slowness", i18n("offset by speed filter"), false, 0.0, 1.0, 15.0, i18n("how slow the offset goes back to zero when the cursor stops moving"));
        list << new BrushSettingDefinition("slow_tracking", i18n("slow position tracking"), true, 0.0, 0.0, 10.0, i18n("Slowdown pointer tracking speed. 0 disables it, higher values remove more jitter in cursor movements. Useful for drawing smooth, comic-like outlines."));
        list << new BrushSettingDefinition("slow_tracking_per_dab", i18n("slow tracking per dab"), false, 0.0, 0.0, 10.0, i18n("Similar as above but at brushdab level (ignoring how much time has past, if brushdabs do not depend on time)"));
        list << new BrushSettingDefinition("tracking_noise", i18n("tracking noise"), true, 0.0, 0.0, 12.0, i18n("add randomness to the mouse pointer; this usually generates many small lines in random directions; maybe try this together with 'slow tracking'"));

        list << new BrushSettingDefinition("color_h", i18n("color hue"), true, 0.0, 0.0, 1.0, i18n("color hue"));
        list << new BrushSettingDefinition("color_s", i18n("color saturation"), true, -0.5, 0.0, 1.5, i18n("color saturation"));
        list << new BrushSettingDefinition("color_v", i18n("color value"), true, -0.5, 0.0, 1.5, i18n("color value (brightness, intensity)"));
        list << new BrushSettingDefinition("change_color_h", i18n("change color hue"), false, -2.0, 0.0, 2.0, i18n("Change color hue.\n-0.1 small clockwise color hue shift\n 0.0 disable\n 0.5 counterclockwise hue shift by 180 degrees"));
        list << new BrushSettingDefinition("change_color_l", i18n("change color lightness (HSL)"), false, -2.0, 0.0, 2.0, i18n("Change the color lightness (luminance) using the HSL color model.\n-1.0 blacker\n 0.0 disable\n 1.0 whiter"));
        list << new BrushSettingDefinition("change_color_hsl_s", i18n("change color satur. (HSL)"), false, -2.0, 0.0, 2.0, i18n("Change the color saturation using the HSL color model.\n-1.0 more grayish\n 0.0 disable\n 1.0 more saturated"));
        list << new BrushSettingDefinition("change_color_v", i18n("change color value (HSV)"), false, -2.0, 0.0, 2.0, i18n("Change the color value (brightness, intensity) using the HSV color model. HSV changes are applied before HSL.\n-1.0 darker\n 0.0 disable\n 1.0 brigher"));
        list << new BrushSettingDefinition("change_color_hsv_s", i18n("change color satur. (HSV)"), false, -2.0, 0.0, 2.0, i18n("Change the color saturation using the HSV color model. HSV changes are applied before HSL.\n-1.0 more grayish\n 0.0 disable\n 1.0 more saturated"));
        list << new BrushSettingDefinition("smudge", i18n("smudge"), false, 0.0, 0.0, 1.0, i18n("Paint with the smudge color instead of the brush color. The smudge color is slowly changed to the color you are painting on.\n 0.0 do not use the smudge color\n 0.5 mix the smudge color with the brush color\n 1.0 use only the smudge color"));
        list << new BrushSettingDefinition("smudge_length", i18n("smudge length"), false, 0.0, 0.5, 1.0, i18n("This controls how fast the smudge color becomes the color you are painting on.\n0.0 immediately change the smudge color\n1.0 never change the smudge color"));
        list << new BrushSettingDefinition("eraser", i18n("eraser"), false, 0.0, 0.0, 1.0, i18n("how much this tool behaves like an eraser\n 0.0 normal painting\n 1.0 standard eraser\n 0.5 pixels go towards 50% transparency"));

        list << new BrushSettingDefinition("stroke_treshold", i18n("stroke threshold"), true, 0.0, 0.0, 0.5, i18n("How much pressure is needed to start a stroke. This affects the stroke input only. Mypaint does not need a minimal pressure to start drawing."));
        list << new BrushSettingDefinition("stroke_duration_logarithmic", i18n("stroke duration"), false, -1.0, 4.0, 7.0, i18n("How far you have to move until the stroke input reaches 1.0. This value is logarithmic (negative values will not invert the process)."));
        list << new BrushSettingDefinition("stroke_holdtime", i18n("stroke hold time"), false, 0.0, 0.0, 10.0, i18n("This defines how long the stroke input stays at 1.0. After that it will reset to 0.0 and start growing again, even if the stroke is not yet finished.\n2.0 means twice as long as it takes to go from 0.0 to 1.0\n9.9 and bigger stands for infinite"));
        list << new BrushSettingDefinition("custom_input", i18n("custom input"), false, -5.0, 0.0, 5.0, i18n("Set the custom input to this value. If it is slowed down, move it towards this value (see below). The idea is that you make this input depend on a mixture of pressure/speed/whatever, and then make other this depend on this 'custom input' instead of repeating this combination everywhere you need it.\nIf you make it change 'by random' you can generate a slow (smooth) random input."));
        list << new BrushSettingDefinition("custom_input_slowness", i18n("custom input filter"), false, 0.0, 0.0, 10.0, i18n("How slow the custom input actually follows the desired value (the one above). This happens at brushdab level (ignoring how much time has past, if brushdabs do not depend on time).\n0.0 no slowdown (changes apply instantly)"));

        list << new BrushSettingDefinition("elliptical_dab_ratio", i18n("elliptical dab: ratio"), false, 1.0, 1.0, 10.0, i18n("aspect ratio of the dabs; must be >= 1.0, where 1.0 means a perfectly round dab. TODO: linearize? start at 0.0 maybe, or log?"));
        list << new BrushSettingDefinition("elliptical_dab_angle", i18n("elliptical dab: angle"), false, 0.0, 90.0, 180.0, i18n("this defines the angle by which eliptical dabs are tilted\n 0.0 horizontal dabs\n 45.0 45 degrees, turned clockwise\n 180.0 horizontal again"));
        list << new BrushSettingDefinition("direction_filter", i18n("direction filter"), false, 0.0, 2.0, 10.0, i18n("a low value will make the direction input adapt more quickly, a high value will make it smoother"));

        int i = 0;
        foreach(BrushSettingDefinition* def, list) {
            dict[def->cname] = def;
            def->index = i++;
        }
    }

};

K_GLOBAL_STATIC(BrushInputDefinitions, s_brush_input_definitions);
K_GLOBAL_STATIC(BrushSettingDefinitions, s_brush_setting_definitions);

class BrushSetting {

public:

    Brush* parent_brush;
    BrushSettingDefinition* brush_setting_definition;

    float base_value;
    typedef QVector<QPointF> vPoints;
    QVector<vPoints> pointsPerInput;

    BrushSetting(BrushSettingDefinition* _brush_setting_definition,
                 Brush* _parent_brush)
        : parent_brush(_parent_brush)
        , brush_setting_definition(_brush_setting_definition)
        , base_value(brush_setting_definition->default_value)
    {
        for (int i = 0; i < s_brush_input_definitions->list.size(); ++i) {
            pointsPerInput << vPoints();
        }
        if (brush_setting_definition->cname == "opaque_multiply") {
            vPoints points;
            points << QPointF(0.0, 0.0) << QPointF(1.0, 1.0);
            set_points(s_brush_input_definitions->dict["pressure"], points);
        }
    }

    BrushSetting(const BrushSetting& rhs)
    {
        load_from_string(rhs.save_to_string());
    }

    void set_base_value(float value)
    {
        if (base_value == value) return;
        base_value = value;
        parent_brush->set_base_value(brush_setting_definition->index, value);
    }

    bool has_only_base_value()
    {
        foreach(const vPoints& points, pointsPerInput) {
            if (points.size() > 0 ) {
                return false;
            }
        }
        return true;
    }

    bool has_input(BrushInputDefinition* input)
    {
        Q_ASSERT(input->index < pointsPerInput.size());
        return (pointsPerInput[input->index].size() > 0);
    }

    void set_points(BrushInputDefinition* input, vPoints& points)
    {
        Q_ASSERT(points.size() != 1);
        parent_brush->set_mapping_n(brush_setting_definition->index,
                                    input->index,
                                    points.size());
        int i = 0;
        foreach(const QPointF point, points) {
            parent_brush->set_mapping_point(brush_setting_definition->index,
                                            input->index,
                                            i,
                                            point.x(),
                                            point.y());
            ++i;
        }
    }

    QString save_to_string() const
    {
        QString s = QString("%1").arg(base_value);

        foreach(BrushInputDefinition* input, s_brush_input_definitions->list) {
            Q_ASSERT(pointsPerInput.size() < input->index);
            vPoints points = pointsPerInput[input->index];
            s += " | " + input->name + ", ";
            foreach(QPointF point, points) {
                s += QString("(%1 %2)").arg(point.x(), point.y());
            }
        }
        return s;
    }

    void load_from_string(const QString& s)
    {
        dbgKrita << brush_setting_definition->cname << s;

        pointsPerInput.clear();
        for (int i = 0; i < s_brush_input_definitions->list.size(); ++i) {
            pointsPerInput << vPoints();
        }

        QStringList parts = s.split("|");
        Q_ASSERT(parts.size() > 0);

        set_base_value(parts.takeFirst().toFloat());

        if (parts.size() > 0 ) {

            dbgKrita << parts;

            parts = parts[0].trimmed().split(" ", QString::SkipEmptyParts);

            dbgKrita << "parts" << parts;

            QString command = parts.takeFirst().trimmed();
            parts = parts.join(" ").split(",");

            dbgKrita << "command" << command << "parts" << parts;

            Q_ASSERT(s_brush_input_definitions->dict.contains(command));
            BrushInputDefinition* input = s_brush_input_definitions->dict[command];
            vPoints points;

            foreach(const QString& sPoint, parts) {
                QString sPointTrimmed = sPoint.trimmed();

                Q_ASSERT(sPointTrimmed.startsWith('('));
                Q_ASSERT(sPointTrimmed.endsWith(')'));

                sPointTrimmed = sPointTrimmed.mid(1, sPointTrimmed.length() - 2);
                QStringList xy = sPointTrimmed.split(' ');
                points << QPointF(xy[0].toFloat(), xy[1].toFloat());
            }

            Q_ASSERT(points.size() >= 2);

            set_points(input, points);
        }
    }
};

MyPaintBrushResource::MyPaintBrushResource(const QString& filename)
    : KoResource( filename )
{
    foreach(BrushSettingDefinition* settingDefinition, s_brush_setting_definitions->list) {
        m_settings << new BrushSetting(settingDefinition, this);
    }
}

MyPaintBrushResource::~MyPaintBrushResource()
{
    m_settings.clear();

}

bool MyPaintBrushResource::load()
{
    dbgKrita << filename();

    QFileInfo iconFile(filename());
    m_icon.load(iconFile.path() + "/" + iconFile.baseName() + "_prev.png");

    int version = -1;

    QFile f(filename());
    if (f.open( QIODevice::ReadOnly)) {
        QTextStream stream(&f);
        QString line;
        while(!stream.atEnd()) {
            line = stream.readLine().trimmed();
            if (line.startsWith('#')) {
                dbgKrita << "\t comment, skip";
                continue;
            }
            if (!line.contains(' ')) {
                dbgKrita << "\t no space, skip";
                continue;
            }
            QStringList parts = line.split(' ');
            Q_ASSERT(!parts.isEmpty());
            QString command = parts.takeFirst();
            command = command.trimmed();
            if (command == "version" && parts.takeFirst().toInt() == 2) {
                version = 2;
            }
            else {
                if (version != 2) {
                    warnKrita << "\t we only read version 2 .myb files";
                    setValid(false);
                    return false;
                }
                else if (s_brush_setting_definitions->dict.contains(command)) {
                    BrushSetting* setting = setting_by_cname(command);
                    setting->load_from_string(parts.join(" "));
                }
            }

        }
        setValid(true);
        return true;
    }
    setValid(false);
    return false;
}

bool MyPaintBrushResource::save()
{
#if 0
    # OPTIMIZE: this cache could be more useful, the current "copy_settings_from()"
    #           brush selection mechanism invalidates it at every brush change
    if self.saved_string: return self.saved_string
    res  = '# mypaint brush file\n'
    res += '# you can edit this file and then select the brush in mypaint (again) to reload\n'
    res += 'version %d\n' % current_brushfile_version
    for s in brushsettings.settings:
        res += s.cname + ' ' + self.settings[s.index].save_to_string() + '\n'
    self.saved_string = res
    return res
#endif

    return true;
}


QImage MyPaintBrushResource::image() const
{
    return m_icon;
}

BrushSetting* MyPaintBrushResource::setting_by_cname(const QString& cname)
{
    BrushSettingDefinition* settingDefinition = s_brush_setting_definitions->dict[cname];
    return m_settings[settingDefinition->index];
}

void MyPaintBrushResource::get_color_hsv(float* h, float* s, float* v)
{
    *h = setting_by_cname("color_h")->base_value;
    *s = setting_by_cname("color_s")->base_value;
    *v = setting_by_cname("color_v")->base_value;
}

void MyPaintBrushResource::set_color_hsv(float h, float s, float v)
{
    setting_by_cname("color_h")->set_base_value(h);
    setting_by_cname("color_s")->set_base_value(s);
    setting_by_cname("color_v")->set_base_value(v);
}

void MyPaintBrushResource::set_color_rgb(QRgb rgb)
{
    QColor c(rgb);
    qreal h, s, v;
    c.getHsvF(&h, &s, &v);
    set_color_hsv(h, s, v);
}

void MyPaintBrushResource::set_color_rgb(float r, float g, float b)
{
    rgb_to_hsv_float(&r, &g, &b);
    set_color_hsv(r, g, b);
}

QRgb MyPaintBrushResource::get_color_rgb()
{
    float h, s, v;
    get_color_hsv(&h, &s, &v);
    return QColor::fromHsvF(h, s, v).rgba();
}

bool MyPaintBrushResource::is_eraser() {
    return setting_by_cname("eraser")->base_value > 0.9;
}
