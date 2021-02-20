/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COLOR_GENERATOR_H
#define COLOR_GENERATOR_H

#include <QObject>
#include <QVariant>
#include "generator/kis_generator.h"

class KisConfigWidget;

class KritaColorGenerator : public QObject
{
    Q_OBJECT
public:
    KritaColorGenerator(QObject *parent, const QVariantList &);
    ~KritaColorGenerator() override;
};

class KisColorGenerator : public KisGenerator
{
public:

    KisColorGenerator();

    using KisGenerator::generate;

    void generate(KisProcessingInformation dst,
                  const QSize& size,
                  const KisFilterConfigurationSP config,
                  KoUpdater* progressUpdater
                 ) const override;

    static inline KoID id() {
        return KoID("color", i18n("Color"));
    }
    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
};

#endif
