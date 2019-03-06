/*
 * KDE. Krita Project.
 *
 * Copyright (c) 2019 Eoin O'Neill <eoinoneill1991@gmail.com>
 * Copyright (c) 2019 Emmet O'Neill <emmetoneill.pdx@gmail.com>
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

    KisFilterConfigurationSP factoryConfiguration() const override;
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev) const override;

    uint seedFromString(const QString &string) const;
    quint64 rotateLeft(const quint64 input, uint d) const;

    static inline double map_range(double value, double curr_min, double curr_max, double new_min, double new_max ) {
            return (value - curr_min) * (new_max - new_min) / (curr_max - curr_min) + new_min;
    }
};
#endif
