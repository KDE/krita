/* This file is part of the KDE project
 * Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "kis_pressure_hsv_option.h"
#include <KoColorTransformation.h>
#include <kis_debug.h>

KisPressureHSVOption* KisPressureHSVOption::createHueOption() {
    return new KisPressureHSVOption("h");
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
    return new KisPressureHSVOption("s");
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
    return new KisPressureHSVOption("v");
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
    QString parameterName;
    int paramId;
};

KisPressureHSVOption::KisPressureHSVOption(const QString& parameterName)
    : KisCurveOption(parameterName, KisPaintOpOption::COLOR, false)
    , d(new Private())
{
    d->parameterName = parameterName;
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
        d->paramId = transfo->parameterId(d->parameterName);
    }

    qreal v = computeSizeLikeValue(info);
    if (d->parameterName == "h") {
        v = computeRotationLikeValue(info, 0, false);
    } else {
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
    if (name() == "h") {
        return -180;
    } else {
        return -100;
    }
}

int KisPressureHSVOption::intMaxValue() const
{
    if (name() == "h") {
        return 180;
    } else {
        return 100;
    }
}

QString KisPressureHSVOption::valueSuffix() const
{
    if (name() == "h") {
        return i18n("°");
    } else {
        return i18n("%");
    }
}
