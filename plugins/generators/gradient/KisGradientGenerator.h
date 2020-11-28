/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISGRADIENTGENERATOR_H
#define KISGRADIENTGENERATOR_H

#include <QObject>

#include "generator/kis_generator.h"

#include "KisGradientGeneratorConfiguration.h"

class KisConfigWidget;

class KisGradientGenerator : public KisGenerator
{
public:
    KisGradientGenerator();

    using KisGenerator::generate;

    virtual void generate(KisProcessingInformation dst,
                          const QSize& size,
                          const KisFilterConfigurationSP config,
                          KoUpdater* progressUpdater) const override;
    
    static inline KoID id() {
        return KoID(KisGradientGeneratorConfiguration::defaultName(), i18n("Gradient"));
    }

    KisFilterConfigurationSP factoryConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
    KisConfigWidget* createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
};

#endif
