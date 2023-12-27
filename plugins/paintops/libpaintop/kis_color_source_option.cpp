/*
 * SPDX-FileCopyrightText: 2011 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_color_source_option.h"

#include <kis_properties_configuration.h>
#include "kis_color_source.h"
#include <kis_painter.h>
#include <kis_paint_device.h>
#include <resources/KoPattern.h>
#include <KisColorSourceOptionData.h>

struct KisColorSourceOption::Private {
    Private()
        : type(KisColorSourceOptionData::PLAIN)
    {}

    KisColorSourceOptionData::Type type;
};

KisColorSourceOption::KisColorSourceOption(const KisPropertiesConfiguration *setting)
    : d(new Private)
{
    KisColorSourceOptionData data;
    data.read(setting);
    d->type = data.type;
}

KisColorSourceOption::~KisColorSourceOption()
{
}

KisColorSource* KisColorSourceOption::createColorSource(const KisPainter* _painter) const
{
    Q_ASSERT(_painter);

    switch (d->type) {
    case KisColorSourceOptionData::PLAIN:
        return new KisPlainColorSource(_painter->backgroundColor(), _painter->paintColor());
    case KisColorSourceOptionData::GRADIENT:
        return new KisGradientColorSource(_painter->gradient(), _painter->paintColor().colorSpace());
    case KisColorSourceOptionData::UNIFORM_RANDOM:
        return new KisUniformRandomColorSource();
    case KisColorSourceOptionData::TOTAL_RANDOM:
        return new KisTotalRandomColorSource();
    case KisColorSourceOptionData::PATTERN: {
        if (_painter->pattern()) {
            KisPaintDevice* dev = new KisPaintDevice(_painter->paintColor().colorSpace(), _painter->pattern()->name());
            dev->convertFromQImage(_painter->pattern()->pattern(), 0);
            return new KoPatternColorSource(dev, _painter->pattern()->width(), _painter->pattern()->height(), false);
        }
        break;
    }
    case KisColorSourceOptionData::PATTERN_LOCKED: {
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
