/*
 * This file is part of Krita
 *
 * Copyright (c) 2020 L. E. Segovia <amy@amyspark.me>
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

#include <KoColor.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>
#include <filter/kis_filter_configuration.h>

#include "generator.h"
#include "kis_wdg_seexpr.h"
#include "ui_wdgseexpr.h"

KisWdgSeExpr::KisWdgSeExpr(QWidget* parent)
        : KisConfigWidget(parent)
{
    m_widget = new Ui_WdgSeExpr();
    m_widget->setupUi(this);
    connect(m_widget->btnUpdate, SIGNAL(clicked()), this, SIGNAL(sigConfigurationUpdated()));
}

KisWdgSeExpr::~KisWdgSeExpr()
{
    delete m_widget;
}

inline const Ui_WdgSeExpr *KisWdgSeExpr::widget() const {
    return m_widget;
}

void KisWdgSeExpr::setConfiguration(const KisPropertiesConfigurationSP config)
{
    Q_ASSERT(!config->getString("script").isEmpty());
    QString script = config->getString("script");

    widget()->txtEditor->setPlainText(script);
}

KisPropertiesConfigurationSP KisWdgSeExpr::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("seexpr", 1);
    QVariant v(widget()->txtEditor->toPlainText());

    config->setProperty("script", v);

    return config;
}

