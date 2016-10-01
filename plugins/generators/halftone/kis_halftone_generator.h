/*
 * This file is part of Krita
 *
 * Copyright (c) 2016 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
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

#ifndef KISHALFTONEGENERATOR_H
#define KISHALFTONEGENERATOR_H

#include <QObject>
#include <QVariant>
#include <kis_generator.h>
#include <kis_filter_configuration.h>
#include <kis_config_widget.h>
#include "ui_wdghalftonegeneratoroptions.h"

class KritaHalftoneGenerator : public QObject
{
    Q_OBJECT
public:
    KritaHalftoneGenerator(QObject *parent, const QVariantList &);
    virtual ~KritaHalftoneGenerator();
};

class KisHalftoneGenerator : public KisGenerator
{
public:
    KisHalftoneGenerator();

    using KisGenerator::generate;

    void generate(KisProcessingInformation dst,
                  const QSize& size,
                  const KisFilterConfigurationSP config,
                  KoUpdater* progressUpdater
                 ) const;

    static inline KoID id() {
        return KoID("halftonegenerator", i18n("Halftone"));
    }
    virtual KisFilterConfigurationSP factoryConfiguration(const KisPaintDeviceSP) const;
    virtual KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev) const;
};

class KisHalftoneGeneratorConfigWidget : public KisConfigWidget
{
    Q_OBJECT
public:
    KisHalftoneGeneratorConfigWidget(QWidget *parent, KisPaintDeviceSP dev);
    virtual ~KisHalftoneGeneratorConfigWidget();

    virtual KisPropertiesConfigurationSP configuration() const;
    void setConfiguration(const KisPropertiesConfigurationSP config);
    Ui::KisWdgHalftoneGen m_page;
};

#endif // KISHALFTONEGENERATOR_H
