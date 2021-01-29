/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 *  Copyright (c) 2020 L. E. Segovia <amy@amyspark.me>
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
#include <QPushButton>
#include <QCheckBox>

#include <klocalizedstring.h>

#include <kis_config_widget.h>
#include <filter/kis_filter_configuration.h>
#include <kis_paint_device.h>
#include <kis_transaction.h>
#include <commands/kis_change_filter_command.h>
#include <kis_generator_layer.h>
#include <KisViewManager.h>
#include <KisDocument.h>

#define UPDATE_DELAY 100 /*ms */

KisDlgGeneratorLayer::KisDlgGeneratorLayer(const QString & defaultName, KisViewManager *view, QWidget *parent, KisGeneratorLayerSP glayer = 0, const KisFilterConfigurationSP previousConfig = 0, const KisStrokeId stroke = KisStrokeId())
        : QDialog(parent)
        , layer(glayer)
        , m_view(view)
        , isEditing(layer && previousConfig)
        , m_customName(false)
        , m_freezeName(false)
        , m_stroke(stroke)
        , m_compressor(UPDATE_DELAY, KisSignalCompressor::FIRST_INACTIVE)
{
    if(isEditing){
        setModal(false);
        configBefore = previousConfig;
    }

    dlgWidget.setupUi(this);
    dlgWidget.wdgGenerator->initialize(m_view);
    dlgWidget.btnBox->button(QDialogButtonBox::Ok)->setDefault(true);

    dlgWidget.txtLayerName->setText( isEditing ? layer->name() : defaultName );
    connect(dlgWidget.txtLayerName, SIGNAL(textChanged(QString)),
            this, SLOT(slotNameChanged(QString)));
    connect(dlgWidget.wdgGenerator, SIGNAL(previewConfiguration()), this, SLOT(previewGenerator()));
    connect(&m_compressor, SIGNAL(timeout()), this, SLOT(slotDelayedPreviewGenerator()));

    dlgWidget.filterGalleryToggle->setIcon(QPixmap(":/pics/sidebaricon.png"));
    dlgWidget.filterGalleryToggle->setChecked(true);
    connect(dlgWidget.filterGalleryToggle, &QCheckBox::toggled, dlgWidget.wdgGenerator, &KisWdgGenerator::showFilterGallery);
    connect(dlgWidget.btnBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(dlgWidget.btnBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(this, SIGNAL(accepted()), this, SLOT(saveLayer()));
    connect(this, SIGNAL(rejected()), this, SLOT(restoreLayer()));

    if (layer && !isEditing) {
        slotDelayedPreviewGenerator();
    }
    restoreGeometry(KisConfig(true).readEntry("generatordialog/geometry", QByteArray()));
}

void KisDlgGeneratorLayer::saveLayer()
{
    /*
     * Editing a layer should be using the show function with automatic deletion on close.
     * Because of this, the action should be taken care of when the window is closed and
     * the user has accepted the changes.
     */
    if (isEditing) {
        layer->setName(layerName());

        KisFilterConfigurationSP configAfter(configuration());
        Q_ASSERT(configAfter);
        QString xmlBefore = configBefore->toXML();
        QString xmlAfter = configAfter->toXML();

        if (xmlBefore != xmlAfter) {
            KisChangeFilterCmd *cmd
                    = new KisChangeFilterCmd(layer,
                                             configBefore->name(),
                                             xmlBefore,
                                             configAfter->name(),
                                             xmlAfter,
                                             true);

            m_view->undoAdapter()->addCommand(cmd);
            m_view->document()->setModified(true);
        }
    } else {
        KIS_ASSERT_RECOVER_RETURN(layer);
        layer->setFilter(configuration());
    }
}

void KisDlgGeneratorLayer::restoreLayer()
{
    if (isEditing)
    {
        layer->setFilter(configBefore);
    }
}

KisDlgGeneratorLayer::~KisDlgGeneratorLayer()
{
    KisConfig(false).writeEntry("generatordialog/geometry", saveGeometry());
}

void KisDlgGeneratorLayer::slotNameChanged(const QString & text)
{
    if (m_freezeName)
        return;

    m_customName = !text.isEmpty();
    dlgWidget.btnBox->button(QDialogButtonBox::Ok)->setEnabled(m_customName);
}

void KisDlgGeneratorLayer::slotDelayedPreviewGenerator()
{
    if (!m_stroke.isNull()) {
        layer->setFilterWithoutUpdate(configuration());
        layer->previewWithStroke(m_stroke);
    }
}

void KisDlgGeneratorLayer::previewGenerator()
{
    if (!m_stroke.isNull()) {
        m_compressor.start();
    }
    else {
        KIS_ASSERT_RECOVER_RETURN(layer);
        layer->setFilter(configuration());
    }
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

