/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSCREENTONEGENERATOR_H
#define KISSCREENTONEGENERATOR_H

#include <QObject>
#include "generator/kis_generator.h"

#include "KisScreentoneGeneratorConfiguration.h"

class KisConfigWidget;

class KisScreentoneGenerator : public KisGenerator
{
public:
    KisScreentoneGenerator();

    using KisGenerator::generate;

    virtual void generate(KisProcessingInformation dst,
                          const QSize& size,
                          const KisFilterConfigurationSP config,
                          KoUpdater* progressUpdater) const override;

    void generate(KisProcessingInformation dst,
                  const QSize& size,
                  const KisScreentoneGeneratorConfigurationSP config,
                  KoUpdater* progressUpdater) const;
    
    template <class Sampler>
    void generate(KisProcessingInformation dst,
                  const QSize &size,
                  const KisScreentoneGeneratorConfigurationSP config,
                  KoUpdater *progressUpdater,
                  const Sampler &sampler) const;
    
    template <class Sampler, class PostprocessingFunction>
    void generate(KisProcessingInformation dst,
                  const QSize &size,
                  const KisScreentoneGeneratorConfigurationSP config,
                  KoUpdater *progressUpdater,
                  const Sampler &sampler,
                  const PostprocessingFunction &postprocessingFunction) const;

    static inline KoID id() {
        return KoID(KisScreentoneGeneratorConfiguration::defaultName(), i18n("Screentone"));
    }

    KisFilterConfigurationSP factoryConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;

private:
    bool checkUpdaterInterruptedAndSetPercent(KoUpdater *progressUpdater, int percent) const;
};

#endif
