/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef PATTERN_GENERATOR_H
#define PATTERN_GENERATOR_H

#include <QObject>
#include <QVariant>
#include "generator/kis_generator.h"

class KisConfigWidget;

class KritaPatternGenerator : public QObject
{
    Q_OBJECT
public:
    KritaPatternGenerator(QObject *parent, const QVariantList &);
    ~KritaPatternGenerator() override;
};

class PatternGenerator : public KisGenerator
{
public:

    PatternGenerator();

    using KisGenerator::generate;

    void generate(KisProcessingInformation dst,
                  const QSize& size,
                  const KisFilterConfigurationSP config,
                  KoUpdater* progressUpdater
                 ) const override;

    static inline KoID id() {
        return KoID("pattern", i18n("Pattern"));
    }

    KisFilterConfigurationSP factoryConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;

    /**
     * This generator pastes a single instance of pattern over the whole layer.
     */
    bool allowsSplittingIntoPatches() const override { return false; }
};

#endif
