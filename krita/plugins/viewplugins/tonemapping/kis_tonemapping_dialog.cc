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

#include <kis_layer.h>

#include "kis_tone_mapping_operator.h"
#include "kis_tone_mapping_operator_configuration_widget.h"
#include "kis_tone_mapping_operators_registry.h"

#include "ui_wdgtonemappingdialog.h"

struct KisToneMappingOperatorConfigurationWidget;

struct KisToneMappingDialog::Private {
        KisLayerSP layer;
        Ui_WdgToneMappingDialog uiToneMappingDialog;
        KoGenericRegistryModel<KisToneMappingOperator*>* operatorsModel;
        KisToneMappingOperatorConfigurationWidget* currentConfigurationWidget;
        QWidget* currentCentralWidget;
        QGridLayout *widgetLayout;
};

KisToneMappingDialog::KisToneMappingDialog(QWidget* parent, KisLayerSP _layer) : QDialog(parent), d(new Private)
{
    d->layer = _layer;
    d->currentConfigurationWidget = 0;
    d->currentCentralWidget = 0;
    d->uiToneMappingDialog.setupUi(this);
    d->widgetLayout = new QGridLayout( d->uiToneMappingDialog.centralWidgetHolder );
    connect(d->uiToneMappingDialog.comboBoxOperators, SIGNAL(activated ( int )), SLOT(slotOperatorSelected(int )) );
    connect(d->uiToneMappingDialog.pushButtonOk, SIGNAL(pressed()), SLOT(accept()));
    connect(d->uiToneMappingDialog.pushButtonOk, SIGNAL(pressed()), SLOT(apply()));
    connect(d->uiToneMappingDialog.pushButtonCancel, SIGNAL(pressed()), SLOT(reject()));
    d->operatorsModel = new KoGenericRegistryModel<KisToneMappingOperator*>(KisToneMappingOperatorsRegistry::instance());
    d->uiToneMappingDialog.comboBoxOperators->setModel(d->operatorsModel);
    slotOperatorSelected(0);
}

void KisToneMappingDialog::apply()
{
    
}

void KisToneMappingDialog::slotOperatorSelected(int index)
{
    kDebug() << "slotOperatorSelected(" << index << ")";
    QModelIndex modelIndex = d->operatorsModel->index(index,0);
    KisToneMappingOperator* tmop = d->operatorsModel->get( modelIndex );
    if( tmop)
    {
        delete d->currentCentralWidget;
        KisToneMappingOperatorConfigurationWidget* widget = tmop->createConfigurationWidget( d->uiToneMappingDialog.centralWidgetHolder );
        if(widget)
        {
            d->currentConfigurationWidget = widget;
            d->currentCentralWidget = widget;
        } else {
            d->currentConfigurationWidget = 0;
            d->currentCentralWidget = new QLabel( i18n("No configuration option."), d->uiToneMappingDialog.centralWidgetHolder );
        }
        d->widgetLayout->addWidget( d->currentCentralWidget, 0 , 0);
    }
}

#include "kis_tonemapping_dialog.moc"
