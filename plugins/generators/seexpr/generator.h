/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SEEXPR_GENERATOR_H
#define SEEXPR_GENERATOR_H

#include <QObject>
#include <QVariant>

#include "generator/kis_generator.h"

#define BASE_SCRIPT                                                                                                                                                                                                                            \
    "$val=voronoi(5*[$u,$v,.5],4,.6,.2); \n \
$color=ccurve($val,\n\
    0.000, [0.141, 0.059, 0.051], 4,\n\
    0.185, [0.302, 0.176, 0.122], 4,\n\
    0.301, [0.651, 0.447, 0.165], 4,\n\
    0.462, [0.976, 0.976, 0.976], 4);\n\
$color\n\
"

class KisConfigWidget;

class KritaSeExprGenerator : public QObject
{
    Q_OBJECT
public:
    KritaSeExprGenerator(QObject *parent, const QVariantList &);
    ~KritaSeExprGenerator() override;
};

class KisSeExprGenerator : public KisGenerator
{
public:
    KisSeExprGenerator();

    using KisGenerator::generate;

    void generate(KisProcessingInformation dst,
                  const QSize& size,
                  const KisFilterConfigurationSP config,
                  KoUpdater* progressUpdater
                 ) const override;

    static inline KoID id()
    {
        return KoID("seexpr", i18n("SeExpr"));
    }
    KisFilterConfigurationSP factoryConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
    KisConfigWidget *createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
};

#endif
