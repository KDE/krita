/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "kis_pressure_hsv_option.h"
#include <KoColorTransformation.h>
#include <kis_debug.h>

KisPressureHSVOption* KisPressureHSVOption::createHueOption() {
    return new KisPressureHSVOption(KoID("h", i18n("Hue")));
}

QString KisPressureHSVOption::hueMinLabel()
{
    // xgettext: no-c-format
    QString activeColorMsg = i18n("(0° is active color)");
    QString br("<br />");
    QString fullPercent = i18n("+180°");
    QString zeroPercent = i18n("-180°");

    return QString(zeroPercent + br + i18n("CCW hue") + br + activeColorMsg);
}

QString KisPressureHSVOption::huemaxLabel()
{
    // xgettext: no-c-format
    QString activeColorMsg = i18n("(0° is active color)");
    QString br("<br />");
    QString fullPercent = i18n("+180°");
    QString zeroPercent = i18n("-180°");

    return QString(fullPercent + br + i18n("CW hue"));
}

KisPressureHSVOption* KisPressureHSVOption::createSaturationOption() {
    return new KisPressureHSVOption(KoID("s", i18n("Saturation")));
}

QString KisPressureHSVOption::saturationMinLabel()
{
    // xgettext: no-c-format
    QString activeColorMsg = i18n("(50% is active color)");
    QString br("<br />");
    QString fullPercent = i18n("+100%");
    QString zeroPercent = i18n("-100%");

    return QString(zeroPercent + br + i18n("Less saturation ") + br + activeColorMsg);

}

QString KisPressureHSVOption::saturationmaxLabel()
{
    // xgettext: no-c-format
    QString activeColorMsg = i18n("(50% is active color)");
    QString br("<br />");
    QString fullPercent = i18n("+100%");
    QString zeroPercent = i18n("-100%");

    return QString(fullPercent + br + i18n("More saturation"));
}

KisPressureHSVOption* KisPressureHSVOption::createValueOption() {
    return new KisPressureHSVOption(KoID("v", i18nc("Label of Brightness value in Color Smudge brush engine options", "Value")));
}

QString KisPressureHSVOption::valueMinLabel()
{
    // xgettext: no-c-format
    QString activeColorMsg = i18n("(50% is active color)");
    QString br("<br />");
    QString fullPercent = i18n("+100%");
    QString zeroPercent = i18n("-100%");

    return QString(zeroPercent + br + i18n("Lower value ") + br + activeColorMsg);

}

QString KisPressureHSVOption::valuemaxLabel()
{
    // xgettext: no-c-format
    QString activeColorMsg = i18n("(50% is active color)");
    QString br("<br />");
    QString fullPercent = i18n("+100%");
    QString zeroPercent = i18n("-100%");

    return QString(fullPercent + br + i18n("Higher value"));


}

struct KisPressureHSVOption::Private
{
    int paramId;
};

KisPressureHSVOption::KisPressureHSVOption(const KoID &id)
    : KisCurveOption(id, KisPaintOpOption::COLOR, false)
    , d(new Private())
{
    d->paramId = -1;
}

KisPressureHSVOption::~KisPressureHSVOption()
{
    delete d;
}

void KisPressureHSVOption::apply(KoColorTransformation* transfo, const KisPaintInformation& info) const
{
    if (!isChecked()) {
        return;
    }

    if (d->paramId == -1) {
        d->paramId = transfo->parameterId(m_id.id());
    }

    qreal v = 0;
    if (m_id.id() == "h") {
        const qreal scalingPartCoeff = 1.0;
        v = computeRotationLikeValue(info, 0, false, scalingPartCoeff, info.isHoveringMode());
    } else {
        v = computeSizeLikeValue(info);
        qreal halfValue = this->value()*0.5;
        v = (v*this->value()) + (0.5-halfValue);
        v = (v*2)-1;
    }
    transfo->setParameter(d->paramId, v);
    transfo->setParameter(3, 0); //sets the type to HSV.
    transfo->setParameter(4, false); //sets the colorize to false.
}


int KisPressureHSVOption::intMinValue() const
{
    if (m_id.id() == "h") {
        return -180;
    } else {
        return -100;
    }
}

int KisPressureHSVOption::intMaxValue() const
{
    if (m_id.id() == "h") {
        return 180;
    } else {
        return 100;
    }
}

QString KisPressureHSVOption::valueSuffix() const
{
    if (m_id.id() == "h") {
        return i18n("°");
    } else {
        return i18n("%");
    }
}
