/*
 * SPDX-FileCopyrightText: 2011 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_color_source_option.h"

#include <QMap>
#include <KoID.h>
#include <kis_properties_configuration.h>
#include "kis_color_source.h"
#include <kis_painter.h>
#include <kis_paint_device.h>
#include <resources/KoPattern.h>

struct KisColorSourceOption::Private {
    Private()
        : type(PLAIN)
    {}

    KisColorSourceOption::Type type;

    static QMap<KisColorSourceOption::Type, KoID> type2id;
    static QMap<QString, KisColorSourceOption::Type> id2type;
    static void addType(KisColorSourceOption::Type _type, KoID _id);
};

QMap<KisColorSourceOption::Type, KoID> KisColorSourceOption::Private::type2id;
QMap<QString, KisColorSourceOption::Type> KisColorSourceOption::Private::id2type;

void KisColorSourceOption::Private::addType(KisColorSourceOption::Type _type, KoID _id)
{
    type2id[_type] = _id;
    id2type[_id.id()] = _type;
}


KisColorSourceOption::KisColorSourceOption() : d(new Private)
{
    if (Private::type2id.isEmpty()) {
        Private::addType(PLAIN, KoID("plain", i18n("Plain color")));
        Private::addType(GRADIENT, KoID("gradient", i18n("Gradient")));
        Private::addType(UNIFORM_RANDOM, KoID("uniform_random", i18n("Uniform random")));
        Private::addType(TOTAL_RANDOM, KoID("total_random", i18n("Total random")));
        Private::addType(PATTERN, KoID("pattern", i18n("Pattern")));
        Private::addType(PATTERN_LOCKED, KoID("lockedpattern", i18n("Locked pattern")));
    }
}

KisColorSourceOption::~KisColorSourceOption()
{
    delete d;
}

void KisColorSourceOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    setting->setProperty("ColorSource/Type", Private::type2id.value(d->type).id());
}

void KisColorSourceOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    QString colorSourceType = setting->getString("ColorSource/Type", "plain");
    d->type = Private::id2type.value(colorSourceType, PLAIN);
}

KisColorSource* KisColorSourceOption::createColorSource(const KisPainter* _painter) const
{
    Q_ASSERT(_painter);

    switch (d->type) {
    case PLAIN:
        return new KisPlainColorSource(_painter->backgroundColor(), _painter->paintColor());
    case GRADIENT:
        return new KisGradientColorSource(_painter->gradient(), _painter->paintColor().colorSpace());
    case UNIFORM_RANDOM:
        return new KisUniformRandomColorSource();
    case TOTAL_RANDOM:
        return new KisTotalRandomColorSource();
    case PATTERN: {
        if (_painter->pattern()) {
            KisPaintDevice* dev = new KisPaintDevice(_painter->paintColor().colorSpace(), _painter->pattern()->name());
            dev->convertFromQImage(_painter->pattern()->pattern(), 0);
            return new KoPatternColorSource(dev, _painter->pattern()->width(), _painter->pattern()->height(), false);
        }
        break;
    }
    case PATTERN_LOCKED: {
        if (_painter->pattern()) {
            KisPaintDevice* dev = new KisPaintDevice(_painter->paintColor().colorSpace(), _painter->pattern()->name());
            dev->convertFromQImage(_painter->pattern()->pattern(), 0);
            return new KoPatternColorSource(dev, _painter->pattern()->width(), _painter->pattern()->height(), true);
        }

    }
    }
    // Fallback in case the patterns are messed up
    return new KisPlainColorSource(_painter->backgroundColor(), _painter->paintColor());
}

QString KisColorSourceOption::colorSourceTypeId() const
{
    return Private::type2id.value(d->type).id();
}

void KisColorSourceOption::setColorSourceType(Type _type)
{
    d->type = _type;
}

void KisColorSourceOption::setColorSourceType(const QString& _id)
{
    d->type = Private::id2type[_id];
}

QList<KoID> KisColorSourceOption::sourceIds()
{
    return Private::type2id.values();
}

KisColorSourceOption::Type KisColorSourceOption::type() const
{
    return d->type;
}
