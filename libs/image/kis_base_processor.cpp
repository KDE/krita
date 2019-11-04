/*
 *  Copyright (c) 2004,2006-2007 Cyrille Berger <cberger@cberger.net>
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

#include "kis_base_processor.h"

#include <QString>

#include "kis_bookmarked_configuration_manager.h"
#include "filter/kis_filter_configuration.h"
#include "kis_paint_device.h"
#include "kis_selection.h"


class KisBaseProcessorConfigurationFactory : public KisSerializableConfigurationFactory
{
public:
    KisBaseProcessorConfigurationFactory(KisBaseProcessor* _generator) : m_generator(_generator) {}
    ~KisBaseProcessorConfigurationFactory() override {}
    KisSerializableConfigurationSP createDefault() override {
        return m_generator->defaultConfiguration();
    }
    KisSerializableConfigurationSP create(const QDomElement& e) override {
        KisSerializableConfigurationSP config = m_generator->factoryConfiguration();
        config->fromXML(e);
        return config;
    }
private:
    KisBaseProcessor* m_generator;
};

struct Q_DECL_HIDDEN KisBaseProcessor::Private {
    Private()
            : bookmarkManager(0)
            , supportsPainting(false)
            , supportsAdjustmentLayers(true)
            , supportsThreading(true)
            , showConfigurationWidget(true)
            , colorSpaceIndependence(FULLY_INDEPENDENT) {
    }

    KisBookmarkedConfigurationManager* bookmarkManager;

    KoID id;
    KoID category; // The category in the filter menu this filter fits
    QString entry; // the i18n'ed accelerated menu text
    QKeySequence shortcut;
    bool supportsPainting;
    bool supportsAdjustmentLayers;
    bool supportsThreading;
    bool showConfigurationWidget;
    ColorSpaceIndependence colorSpaceIndependence;
};

KisBaseProcessor::KisBaseProcessor(const KoID& id, const KoID & category, const QString & entry)
        : d(new Private)
{
    d->id = id;
    d->category = category;
    d->entry = entry;
}

void KisBaseProcessor::init(const QString& configEntryGroup)
{
    d->bookmarkManager = new KisBookmarkedConfigurationManager(configEntryGroup, new KisBaseProcessorConfigurationFactory(this));
}

KisBaseProcessor::~KisBaseProcessor()
{
    delete d->bookmarkManager;
    delete d;
}

KisFilterConfigurationSP KisBaseProcessor::factoryConfiguration() const
{
    return new KisFilterConfiguration(id(), 1);
}

KisFilterConfigurationSP KisBaseProcessor::defaultConfiguration() const
{
    return factoryConfiguration();
}

KisConfigWidget * KisBaseProcessor::createConfigurationWidget(QWidget *, const KisPaintDeviceSP, bool) const
{
    return 0;
}

KisBookmarkedConfigurationManager* KisBaseProcessor::bookmarkManager()
{
    return d->bookmarkManager;
}

const KisBookmarkedConfigurationManager* KisBaseProcessor::bookmarkManager() const
{
    return d->bookmarkManager;
}

QString KisBaseProcessor::id() const
{
    return d->id.id();
}

QString KisBaseProcessor::name() const
{
    return d->id.name();
}

KoID KisBaseProcessor::menuCategory() const
{
    return d->category;
}

QString KisBaseProcessor::menuEntry() const
{
    return d->entry;
}

QKeySequence KisBaseProcessor::shortcut() const
{
    return d->shortcut;
}

void KisBaseProcessor::setShortcut(const QKeySequence & shortcut)
{
    d->shortcut = shortcut;
}

bool KisBaseProcessor::supportsPainting() const
{
    return d->supportsPainting;
}

bool KisBaseProcessor::supportsAdjustmentLayers() const
{
    return d->supportsAdjustmentLayers;
}

bool KisBaseProcessor::supportsThreading() const
{
    return d->supportsThreading;
}

ColorSpaceIndependence KisBaseProcessor::colorSpaceIndependence() const
{
    return d->colorSpaceIndependence;
}

void KisBaseProcessor::setSupportsPainting(bool v)
{
    d->supportsPainting = v;
}

void KisBaseProcessor::setSupportsAdjustmentLayers(bool v)
{
    d->supportsAdjustmentLayers = v;
}

void KisBaseProcessor::setSupportsThreading(bool v)
{
    d->supportsThreading = v;
}

void KisBaseProcessor::setColorSpaceIndependence(ColorSpaceIndependence v)
{
    d->colorSpaceIndependence = v;
}

bool KisBaseProcessor::showConfigurationWidget()
{
    return d->showConfigurationWidget;
}

void KisBaseProcessor::setShowConfigurationWidget(bool v)
{
    d->showConfigurationWidget = v;
}
