/*
 * This file is part of Krita
 *
 * Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

class KoPatternGenerator : public KisGenerator
{
public:

    KoPatternGenerator();

    using KisGenerator::generate;

    void generate(KisProcessingInformation dst,
                  const QSize& size,
                  const KisFilterConfigurationSP config,
                  KoUpdater* progressUpdater
                 ) const override;

    static inline KoID id() {
        return KoID("pattern", i18n("Pattern"));
    }
    KisFilterConfigurationSP defaultConfiguration() const override;
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
};

#endif
