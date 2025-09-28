/*
 * SPDX-FileCopyrightText: 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "MyPaintPaintOpPreset.h"

#include <QFile>
#include <QFileInfo>
#include <array>
#include <libmypaint/mypaint-brush.h>
#include <png.h>

#include <KisResourceLocator.h>
#include <KisResourceServerProvider.h>
#include <KoColorConversions.h>
#include <KoColorModelStandardIds.h>
#include <kis_debug.h>

#include "MyPaintPaintOpSettings.h"
#include "MyPaintSensorPack.h"
#include "MyPaintStandardOptionData.h"

class KisMyPaintPaintOpPreset::Private {

public:
    MyPaintBrush *brush;
    QImage icon;
    QByteArray json;
};

KisMyPaintPaintOpPreset::KisMyPaintPaintOpPreset(const QString &fileName)
    : KisPaintOpPreset(fileName)
    , d(new Private)
{
    d->brush = mypaint_brush_new();
    mypaint_brush_from_defaults(d->brush);
}

KisMyPaintPaintOpPreset::KisMyPaintPaintOpPreset(const KisMyPaintPaintOpPreset &rhs)
    : KisPaintOpPreset(rhs)
    , d(new Private(*rhs.d))
{
    d->brush = mypaint_brush_new();

    if (d->json.isEmpty()) {
        mypaint_brush_from_defaults(d->brush);
    } else {
        mypaint_brush_from_string(d->brush, d->json);
    }
}

KisMyPaintPaintOpPreset::~KisMyPaintPaintOpPreset() {

    mypaint_brush_unref(d->brush);
    delete d;
}

KoResourceSP KisMyPaintPaintOpPreset::clone() const
{
    return toQShared(new KisMyPaintPaintOpPreset(*this));
}

void KisMyPaintPaintOpPreset::setColor(const KoColor color, const KoColorSpace *colorSpace) {

    float hue, saturation, value;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    qreal r = 0, g = 0, b = 0;
#else
    float r = 0, g = 0, b = 0;
#endif
    QColor dstColor;

    if (colorSpace->colorModelId() == RGBAColorModelID) {
        colorSpace->toQColor(color.data(), &dstColor);
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

    mypaint_brush_new_stroke(d->brush);
}

MyPaintBrush* KisMyPaintPaintOpPreset::brush() {

    return d->brush;
}

bool KisMyPaintPaintOpPreset::loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface)
{
    if (!dev->isSequential())
        dev->seek(0); // ensure we do read *all* the bytes

    std::array<png_byte, 8> signature;
    dev->peek(reinterpret_cast<char*>(signature.data()), 8);

#if PNG_LIBPNG_VER < 10400
    if (png_check_sig(signature, 8)) {
#else
    if (png_sig_cmp(signature.data(), 0, 8) == 0) {
#endif
        // this is a koresource
        if (KisPaintOpPreset::loadFromDevice(dev, resourcesInterface)) {
            apply(settings());
            // correct filename
            const QString f = filename();
            if (f.endsWith(".myb", Qt::CaseInsensitive)) {
                setFilename(QFileInfo(f).completeBaseName().append(KisPaintOpPreset::defaultFileExtension()));
            }
            return true;
        } else {
            warnPlugins << "Failed loading MyPaint preset from KoResource serialization";
            return false;
        }
    }
    
    const QByteArray ba(dev->readAll());
    d->json = ba;
    // mypaint can handle invalid json files too, so this is the only way to find out if it was correct mypaint file or not...
    // if the json is incorrect, the brush will get the default mypaint brush settings
    // which looks like a round brush with low opacity and high spacing
    bool success = mypaint_brush_from_string(d->brush, ba);
    const float isEraser = mypaint_brush_get_base_value(d->brush, MYPAINT_BRUSH_SETTING_ERASER);

    KisPaintOpSettingsSP s = new KisMyPaintOpSettings(resourcesInterface);
    s->setProperty("paintop", "mypaintbrush");
    s->setProperty("filename", this->filename());
    s->setProperty(MYPAINT_JSON, this->getJsonData());
    s->setProperty("EraserMode", qRound(isEraser));


    {
        /**
         * See a comment in `namespace deprecated_remove_after_krita6` in
         * MyPaintStandardOptionData.cpp
         */

        auto recoverDeprecatedProperty = [] (auto data, KisPaintOpSettingsSP settings) {
            /// we just round-trip the save operation to save the property
            /// out to json object

            data.read(settings.data());
            data.write(settings.data());
        };

        recoverDeprecatedProperty(MyPaintRadiusLogarithmicData(), s);
        recoverDeprecatedProperty(MyPaintOpacityData(), s);
        recoverDeprecatedProperty(MyPaintHardnessData(), s);
    }


    if (!metadata().contains("paintopid")) {
        addMetaData("paintopid", "mypaintbrush");
    }

    this->setSettings(s);
    setName(QFileInfo(filename()).baseName());
    setValid(success);

    return success;
}

void KisMyPaintPaintOpPreset::updateThumbnail()
{
    d->icon = thumbnail();
}

QString KisMyPaintPaintOpPreset::thumbnailPath() const
{
    return QFileInfo(filename()).baseName() + "_prev.png";
}

QByteArray KisMyPaintPaintOpPreset::getJsonData() {

    return d->json;
}
