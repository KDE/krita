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

class KisConfigWidget;

class KisScreentoneGeneratorHandle : public QObject
{
    Q_OBJECT
public:
    KisScreentoneGeneratorHandle(QObject *parent, const QVariantList &);
    ~KisScreentoneGeneratorHandle() override;
};

class KisScreentoneGenerator : public KisGenerator
{
public:
    KisScreentoneGenerator();

    using KisGenerator::generate;

    virtual void generate(KisProcessingInformation dst,
                          const QSize& size,
                          const KisFilterConfigurationSP config,
                          KoUpdater* progressUpdater) const override;
    
    template <class ScreentoneFunction>
    void generate(KisProcessingInformation dst,
                  const QSize &size,
                  const KisFilterConfigurationSP config,
                  KoUpdater *progressUpdater,
                  const ScreentoneFunction &screentoneFunction) const;
    
    template <class ScreentoneFunction, class BrightnessContrastFunction>
    void generate(KisProcessingInformation dst,
                  const QSize &size,
                  const KisFilterConfigurationSP config,
                  KoUpdater *progressUpdater,
                  const ScreentoneFunction &screentoneFunction,
                  const BrightnessContrastFunction &brightnessContrastFunction) const;

    static inline KoID id() {
        return KoID("screentone", i18n("Screentone"));
    }

    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;

private:
    bool checkUpdaterInterruptedAndSetPercent(KoUpdater *progressUpdater, int percent) const;
};

#endif
