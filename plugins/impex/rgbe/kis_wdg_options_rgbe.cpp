/*
 * SPDX-FileCopyrightText: 2023 Rasyuqa A. H. <qampidh@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KoColor.h>

#include "kis_wdg_options_rgbe.h"


KisWdgOptionsRGBE::KisWdgOptionsRGBE(QWidget *parent)
    : KisConfigWidget(parent)
{
    setupUi(this);
}

void KisWdgOptionsRGBE::setConfiguration(const KisPropertiesConfigurationSP cfg)
{
    KoColor background(KoColorSpaceRegistry::instance()->rgb8());
    background.fromQColor(Qt::white);
    bnTransparencyFillColor->setDefaultColor(background);

    bnTransparencyFillColor->setColor(cfg->getColor("transparencyFillcolor", background));
}

KisPropertiesConfigurationSP KisWdgOptionsRGBE::configuration() const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();

    QVariant transparencyFillcolor;
    transparencyFillcolor.setValue(bnTransparencyFillColor->color());

    cfg->setProperty("transparencyFillcolor", transparencyFillcolor);

    return cfg;
}
