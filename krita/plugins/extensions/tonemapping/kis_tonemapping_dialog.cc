/*
 *  Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_tonemapping_dialog.h"
#include <KoGenericRegistryModel.h>
#include <KoColorSpace.h>

// Krita/Image
#include <kis_layer.h>
#include <kis_properties_configuration.h>
#include <kis_transaction.h>
#include <kis_undo_adapter.h>
#include <kis_image.h>
#include <kis_paint_device.h>

// Krita/UI
#include <kis_bookmarked_configuration_manager.h>
#include <kis_bookmarked_configurations_editor.h>

// Krita/Tone-Mapping
#include "kis_bookmarked_tone_mapping_operator_configurations_model.h"
#include "kis_tone_mapping_operator.h"
#include "kis_tone_mapping_operator_configuration_widget.h"
#include "kis_tone_mapping_operators_registry.h"

#include "ui_wdgtonemappingdialog.h"

class KisToneMappingOperatorConfigurationWidget;

struct KisToneMappingDialog::Private {
    KisLayerSP layer;
    KisPaintDeviceSP thumb;
    Ui_WdgToneMappingDialog uiToneMappingDialog;
    KoGenericRegistryModel<KisToneMappingOperator*>* operatorsModel;
    KisToneMappingOperatorConfigurationWidget* currentConfigurationWidget;
    KisToneMappingOperator* currentOperator;
    QWidget* currentCentralWidget;
    QGridLayout *widgetLayout;
    KisBookmarkedToneMappingOperatorConfigurationsModel* currentBookmarkedToneMappingConfigurationsModel;
};

KisToneMappingDialog::KisToneMappingDialog(QWidget* parent, KisLayerSP _layer) : QDialog(parent), d(new Private)
{
    d->layer = _layer;
    d->currentConfigurationWidget = 0;
    d->currentCentralWidget = 0;
    d->currentOperator = 0;
    d->currentBookmarkedToneMappingConfigurationsModel = 0;
    d->uiToneMappingDialog.setupUi(this);
    d->widgetLayout = new QGridLayout(d->uiToneMappingDialog.centralWidgetHolder);
    d->thumb = d->layer->paintDevice()->createThumbnailDevice(100, 100);
    connect(d->uiToneMappingDialog.comboBoxOperators, SIGNAL(activated(int)), SLOT(slotOperatorSelected(int)));
    connect(d->uiToneMappingDialog.pushButtonOk, SIGNAL(pressed()), SLOT(accept()));
    connect(d->uiToneMappingDialog.pushButtonOk, SIGNAL(pressed()), SLOT(apply()));
    connect(d->uiToneMappingDialog.pushButtonApply, SIGNAL(pressed()), SLOT(apply()));
    connect(d->uiToneMappingDialog.pushButtonCancel, SIGNAL(pressed()), SLOT(reject()));
    connect(d->uiToneMappingDialog.comboBoxPresets, SIGNAL(activated(int)), SLOT(slotBookmarkedToneMappingConfigurationSelected(int)));
    connect(d->uiToneMappingDialog.pushButtonEditPressets, SIGNAL(pressed()), SLOT(editConfigurations()));


    d->operatorsModel = new KoGenericRegistryModel<KisToneMappingOperator*>(KisToneMappingOperatorsRegistry::instance());
    d->uiToneMappingDialog.comboBoxOperators->setModel(d->operatorsModel);
    slotOperatorSelected(0);
}

void KisToneMappingDialog::apply()
{
    d->layer->image()->lock();
    KisPropertiesConfiguration* config = (d->currentConfigurationWidget) ? d->currentConfigurationWidget->configuration() : new KisPropertiesConfiguration;
    const KoColorSpace* colorSpace = d->currentOperator->colorSpace();

    if (d->layer->image()->undo()) {
        d->layer->image()->undoAdapter()->beginMacro(d->currentOperator->name());
    }
    if (!(*d->layer->paintDevice()->colorSpace() == *colorSpace)) {
        QUndoCommand* cmd = d->layer->paintDevice()->convertTo(colorSpace);
        if (d->layer->image()->undo()) {
            d->layer->image()->undoAdapter()->addCommand(cmd);
        } else {
            delete cmd;
        }
    }
    KisTransaction * cmd = 0;

    if (d->layer->image()->undo()) cmd = new KisTransaction(d->currentOperator->name(), d->layer->paintDevice());
    d->currentOperator->toneMap(d->layer->paintDevice(), config);
    if (cmd) d->layer->image()->undoAdapter()->addCommand(cmd);

    d->currentOperator->bookmarkManager()->save(KisBookmarkedConfigurationManager::ConfigLastUsed.id(), config);

    if (d->layer->image()->undo()) {
        d->layer->image()->undoAdapter()->endMacro();
    }

    d->layer->setDirty();
    d->layer->image()->unlock();
    delete config;
}

void KisToneMappingDialog::slotOperatorSelected(int index)
{
    dbgKrita << "slotOperatorSelected(" << index << ")";
    QModelIndex modelIndex = d->operatorsModel->index(index, 0);
    KisToneMappingOperator* tmop = d->operatorsModel->get(modelIndex);
    if (tmop) {
        delete d->currentCentralWidget;
        KisToneMappingOperatorConfigurationWidget* widget = tmop->createConfigurationWidget(d->uiToneMappingDialog.centralWidgetHolder);
        if (widget) {
            d->currentConfigurationWidget = widget;
            d->currentCentralWidget = widget;
            d->currentConfigurationWidget->setConfiguration(tmop->defaultConfiguration());
        } else {
            d->currentConfigurationWidget = 0;
            d->currentCentralWidget = new QLabel(i18n("No configuration option."), d->uiToneMappingDialog.centralWidgetHolder);
        }
        d->widgetLayout->addWidget(d->currentCentralWidget, 0 , 0);

        // Change the list of presets
        delete d->currentBookmarkedToneMappingConfigurationsModel;
        d->currentBookmarkedToneMappingConfigurationsModel = new KisBookmarkedToneMappingOperatorConfigurationsModel(d->thumb, tmop);
        d->uiToneMappingDialog.comboBoxPresets->setModel(d->currentBookmarkedToneMappingConfigurationsModel);
        d->currentOperator = tmop;
    }
}

void KisToneMappingDialog::slotBookmarkedToneMappingConfigurationSelected(int index)
{
    if (d->currentConfigurationWidget) {
        QModelIndex modelIndex = d->currentBookmarkedToneMappingConfigurationsModel->index(index, 0);
        KisPropertiesConfiguration* config  = d->currentBookmarkedToneMappingConfigurationsModel->configuration(modelIndex);
        d->currentConfigurationWidget->setConfiguration(config);
    }
}

void KisToneMappingDialog::editConfigurations()
{
    KisSerializableConfiguration* config =
        d->currentConfigurationWidget ? d->currentConfigurationWidget->configuration() : 0;
    KisBookmarkedConfigurationsEditor editor(this, d->currentBookmarkedToneMappingConfigurationsModel, config);
    editor.exec();
}

#include "kis_tonemapping_dialog.moc"
