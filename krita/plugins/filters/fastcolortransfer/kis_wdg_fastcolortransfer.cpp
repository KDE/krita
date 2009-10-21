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

#include "kis_wdg_fastcolortransfer.h"


#include <qlayout.h>

#include <kurlrequester.h>

#include "ui_wdgfastcolortransfer.h"
#include <filter/kis_filter_configuration.h>
KisWdgFastColorTransfer::KisWdgFastColorTransfer(QWidget * parent) : KisConfigWidget(parent)
{
    m_widget = new Ui_WdgFastColorTransfer();
    m_widget->setupUi(this);
    connect(m_widget->fileNameURLRequester, SIGNAL(textChanged(const QString&)), this, SIGNAL(sigConfigurationItemChanged()));
}


KisWdgFastColorTransfer::~KisWdgFastColorTransfer()
{
}

void KisWdgFastColorTransfer::setConfiguration(const KisPropertiesConfiguration* config)
{
    QVariant value;
    if (config->getProperty("filename", value)) {
        widget()->fileNameURLRequester->setUrl(value.toString());
    }

}

KisPropertiesConfiguration* KisWdgFastColorTransfer::configuration() const
{
    KisFilterConfiguration* config = new KisFilterConfiguration("colortransfer", 1);
    QString url = this->widget()->fileNameURLRequester->url().url();
    dbgPlugins << "Filename : " << this->widget()->fileNameURLRequester->url() << " = " << url;
    config->setProperty("filename", url);
    return config;
}
