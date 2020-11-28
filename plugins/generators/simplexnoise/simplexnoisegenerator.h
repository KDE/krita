/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2019 Eoin O 'Neill <eoinoneill1991@gmail.com>
 * SPDX-FileCopyrightText: 2019 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef NOISEFILTER_H
#define NOISEFILTER_H

#include <QObject>
#include "generator/kis_generator.h"

class KisConfigWidget;

class KisSimplexNoiseGeneratorHandle : public QObject
{
    Q_OBJECT
public:
    KisSimplexNoiseGeneratorHandle(QObject *parent, const QVariantList &);
    ~KisSimplexNoiseGeneratorHandle() override;
};

class KisSimplexNoiseGenerator : public KisGenerator
{
public:
    KisSimplexNoiseGenerator();

    using KisGenerator::generate;

    virtual void generate(KisProcessingInformation dst,
                          const QSize& size,
                          const KisFilterConfigurationSP config,
                          KoUpdater* progressUpdater
                         ) const override;

    static inline KoID id() {
        return KoID("simplex_noise", i18n("Simplex Noise"));
    }

    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;

    uint seedFromString(const QString &string) const;
    quint64 rotateLeft(const quint64 input, uint d) const;

    static inline double map_range(double value, double curr_min, double curr_max, double new_min, double new_max ) {
            return (value - curr_min) * (new_max - new_min) / (curr_max - curr_min) + new_min;
    }
};
#endif
