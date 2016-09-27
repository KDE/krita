/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_dlg_generator_layer.h"

#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QGridLayout>

#include <klocalizedstring.h>

#include <kis_config_widget.h>
#include <filter/kis_filter_configuration.h>
#include <kis_paint_device.h>
#include <kis_transaction.h>
#include <KisViewManager.h>

KisDlgGeneratorLayer::KisDlgGeneratorLayer(const QString & name, KisViewManager *view, QWidget *parent)
        : KoDialog(parent)
        , m_customName(false)
        , m_freezeName(false)
{

    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    QWidget * page = new QWidget(this);
    dlgWidget.setupUi(page);
    dlgWidget.wdgGenerator->initialize(view);

    setMainWidget(page);
    dlgWidget.txtLayerName->setText(name);
    connect(dlgWidget.txtLayerName, SIGNAL(textChanged(QString)),
            this, SLOT(slotNameChanged(QString)));
}

void KisDlgGeneratorLayer::slotNameChanged(const QString & text)
{
    if (m_freezeName)
        return;

    m_customName = !text.isEmpty();
    enableButtonOk(m_customName);
}

void KisDlgGeneratorLayer::setConfiguration(const KisFilterConfigurationSP  config)
{
    dlgWidget.wdgGenerator->setConfiguration(config);
}

KisFilterConfigurationSP  KisDlgGeneratorLayer::configuration() const
{
    return dlgWidget.wdgGenerator->configuration();
}

QString KisDlgGeneratorLayer::layerName() const
{
    return dlgWidget.txtLayerName->text();
}

