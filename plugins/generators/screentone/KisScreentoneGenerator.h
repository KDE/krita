/*
 * KDE. Krita Project.
 *
 * Copyright (c) 2020 Deif Lou <ginoba@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
