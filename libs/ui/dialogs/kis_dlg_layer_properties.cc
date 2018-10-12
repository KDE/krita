/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
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

#include "kis_dlg_layer_properties.h"
#include <limits.h>

#include <QLabel>
#include <QLayout>
#include <QString>
#include <QGroupBox>
#include <QVBoxLayout>

#include <klocalizedstring.h>

#include <KoColorSpace.h>

#include "KisViewManager.h"
#include <kis_debug.h>
#include <kis_global.h>

#include "widgets/kis_cmb_composite.h"
#include "KoColorProfile.h"
#include "kis_multinode_property.h"
#include "kis_layer_utils.h"
#include "kis_image.h"
#include "kis_layer_properties_icons.h"
#include "kis_signal_compressor.h"
#include "commands_new/kis_saved_commands.h"
#include "kis_post_execution_undo_adapter.h"


struct KisDlgLayerProperties::Private
{
    Private() : updatesCompressor(500, KisSignalCompressor::POSTPONE) {}

    KisNodeList nodes;
    const KoColorSpace *colorSpace;
    KisViewManager *view;
    WdgLayerProperties *page;

    QSharedPointer<KisMultinodeCompositeOpProperty> compositeOpProperty;
    QSharedPointer<KisMultinodeOpacityProperty> opacityProperty;
    QSharedPointer<KisMultinodeNameProperty> nameProperty;
    QSharedPointer<KisMultinodeColorLabelProperty> colorLabelProperty;

    QList<KisMultinodePropertyInterfaceSP> layerProperties;
    QList<QPointer<QCheckBox> > layerPropCheckboxes;

    QList<KisMultinodePropertyInterfaceSP> channelFlagsProps;
    QList<QPointer<QCheckBox> > channelFlagsCheckboxes;

    KisSignalCompressor updatesCompressor;

    QList<KisMultinodePropertyInterfaceSP> allProperties() const {
        QList<KisMultinodePropertyInterfaceSP> props;
        props << compositeOpProperty;
        props << opacityProperty;
        props << nameProperty;
        props << layerProperties;
        props << channelFlagsProps;
        props << colorLabelProperty;
        return props;
    }
};

KisDlgLayerProperties::KisDlgLayerProperties(KisNodeList nodes, KisViewManager *view, QWidget *parent, const char *name, Qt::WindowFlags f)
    : KoDialog(parent)
    , d(new Private())
{
    nodes = KisLayerUtils::sortMergableNodes(view->image()->root(), nodes);
    d->nodes = nodes;

    Q_UNUSED(f);
    setCaption(i18n("Layer Properties"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    setModal(false);

    setObjectName(name);
    d->page = new WdgLayerProperties(this);
    setMainWidget(d->page);
    d->view = view;
    d->colorSpace = d->nodes.first()->colorSpace();

    d->page->editName->setFocus();
    d->nameProperty.reset(new KisMultinodeNameProperty(nodes));
    d->nameProperty->connectIgnoreCheckBox(d->page->chkName);
    d->nameProperty->connectAutoEnableWidget(d->page->editName);
    d->nameProperty->connectValueChangedSignal(this, SLOT(slotNameValueChangedInternally()));
    connect(d->page->editName, SIGNAL(textChanged(const QString &)), SLOT(slotNameValueChangedExternally()));

    d->page->intOpacity->setRange(0, 100);
    d->page->intOpacity->setSuffix("%");
    d->opacityProperty.reset(new KisMultinodeOpacityProperty(nodes));
    d->opacityProperty->connectIgnoreCheckBox(d->page->chkOpacity);
    d->opacityProperty->connectAutoEnableWidget(d->page->intOpacity);
    d->opacityProperty->connectValueChangedSignal(this, SLOT(slotOpacityValueChangedInternally()));
    d->opacityProperty->connectValueChangedSignal(&d->updatesCompressor, SLOT(start()));
    connect(d->page->intOpacity, SIGNAL(valueChanged(int)), SLOT(slotOpacityValueChangedExternally()));

    d->compositeOpProperty.reset(new KisMultinodeCompositeOpProperty(nodes));
    d->compositeOpProperty->connectIgnoreCheckBox(d->page->chkCompositeOp);
    d->compositeOpProperty->connectAutoEnableWidget(d->page->cmbComposite);
    d->compositeOpProperty->connectValueChangedSignal(this, SLOT(slotCompositeOpValueChangedInternally()));
    d->compositeOpProperty->connectValueChangedSignal(&d->updatesCompressor, SLOT(start()));
    connect(d->page->cmbComposite, SIGNAL(currentIndexChanged(int)), SLOT(slotCompositeOpValueChangedExternally()));

    d->page->colorLabelSelector->setFocusPolicy(Qt::StrongFocus);
    d->colorLabelProperty.reset(new KisMultinodeColorLabelProperty(nodes));
    d->colorLabelProperty->connectIgnoreCheckBox(d->page->chkColorLabel);
    d->colorLabelProperty->connectAutoEnableWidget(d->page->colorLabelSelector);
    d->colorLabelProperty->connectValueChangedSignal(this, SLOT(slotColorLabelValueChangedInternally()));
    d->colorLabelProperty->connectValueChangedSignal(&d->updatesCompressor, SLOT(start()));
    connect(d->page->colorLabelSelector, SIGNAL(currentIndexChanged(int)), SLOT(slotColorLabelValueChangedExternally()));

    if (!KisLayerUtils::checkNodesDiffer<const KoColorSpace*>(d->nodes, [](KisNodeSP node) { return node->colorSpace(); })) {

        d->page->lblColorSpace->setText(d->colorSpace->name());
        if (const KoColorProfile* profile = d->colorSpace->profile()) {
            d->page->lblProfile->setText(profile->name());
        }

        QRect bounds = d->nodes.first()->exactBounds();
        d->page->lblDimensions->setText(i18nc("layer dimensions", "(%1, %2), (%3, %4)",
                                        bounds.x(),
                                        bounds.y(),
                                        bounds.width(),
                                        bounds.height()));

        ChannelFlagAdapter::PropertyList props = ChannelFlagAdapter::adaptersList(nodes);
        if (!props.isEmpty()) {
            QVBoxLayout *vbox = new QVBoxLayout;
            Q_FOREACH (const ChannelFlagAdapter::Property &prop, props) {
                QCheckBox *chk = new QCheckBox(prop.name, this);
                vbox->addWidget(chk);

                KisMultinodePropertyInterface *multiprop =
                    new KisMultinodeProperty<ChannelFlagAdapter>(
                        nodes,
                        ChannelFlagAdapter(prop));

                multiprop->connectIgnoreCheckBox(chk);
                multiprop->connectValueChangedSignal(this, SLOT(slotFlagsValueChangedInternally()));
                multiprop->connectValueChangedSignal(&d->updatesCompressor, SLOT(start()));

                d->channelFlagsCheckboxes << chk;
                d->channelFlagsProps << toQShared(multiprop);
            }

            d->page->grpActiveChannels->setLayout(vbox);
        } else {
            d->page->grpActiveChannels->setVisible(false);
            d->page->lineActiveChannels->setVisible(false);
        }
    } else {
        d->page->grpActiveChannels->setVisible(false);
        d->page->lineActiveChannels->setVisible(false);
        d->page->cmbComposite->setEnabled(false);
        d->page->chkCompositeOp->setEnabled(false);
        d->page->lblDimensions->setText(i18n("*varies*"));
        d->page->lblColorSpace->setText(i18n("*varies*"));
        d->page->lblProfile->setText(i18n("*varies*"));
    }

    {
        QVBoxLayout *vbox = new QVBoxLayout;

        KisBaseNode::PropertyList props = LayerPropertyAdapter::adaptersList(nodes);
        Q_FOREACH (const KisBaseNode::Property &prop, props) {
            QCheckBox *chk = new QCheckBox(prop.name, this);
            chk->setIcon(prop.onIcon);
            vbox->addWidget(chk);

            KisMultinodePropertyInterface *multiprop =
                new KisMultinodeProperty<LayerPropertyAdapter>(
                    nodes,
                    LayerPropertyAdapter(prop.name));

            multiprop->connectIgnoreCheckBox(chk);
            multiprop->connectValueChangedSignal(this, SLOT(slotPropertyValueChangedInternally()));
            multiprop->connectValueChangedSignal(&d->updatesCompressor, SLOT(start()));

            d->layerPropCheckboxes << chk;
            d->layerProperties << toQShared(multiprop);
        }

        d->page->grpProperties->setLayout(vbox);
    }

    connect(&d->updatesCompressor, SIGNAL(timeout()), SLOT(updatePreview()));
}

KisDlgLayerProperties::~KisDlgLayerProperties()
{
    if (result() == QDialog::Accepted) {
        if (d->updatesCompressor.isActive()) {
            d->updatesCompressor.stop();
            updatePreview();
        }

        KisPostExecutionUndoAdapter *adapter =
            d->view->image()->postExecutionUndoAdapter();
        KisSavedMacroCommand *macro = adapter->createMacro(kundo2_i18n("Change Layer Properties"));
        macro->addCommand(toQShared(new KisLayerUtils::KisSimpleUpdateCommand(d->nodes, false)));
        Q_FOREACH(auto prop, d->allProperties()) {
            if (!prop->isIgnored()) {
                macro->addCommand(toQShared(prop->createPostExecutionUndoCommand()));
            }
        }
        macro->addCommand(toQShared(new KisLayerUtils::KisSimpleUpdateCommand(d->nodes, true)));
        adapter->addMacro(macro);
    }
    else /* if (result() == QDialog::Rejected) */ {
        Q_FOREACH(auto prop, d->allProperties()) {
            prop->setIgnored(true);
        }
        updatePreview();
    }
}

void KisDlgLayerProperties::slotCompositeOpValueChangedInternally()
{
    d->page->cmbComposite->validate(d->colorSpace);
    d->page->cmbComposite->selectCompositeOp(KoID(d->compositeOpProperty->value()));
    d->page->cmbComposite->setEnabled(!d->compositeOpProperty->isIgnored());
}

void KisDlgLayerProperties::slotCompositeOpValueChangedExternally()
{
    if (d->compositeOpProperty->isIgnored()) return;
    d->compositeOpProperty->setValue(d->page->cmbComposite->selectedCompositeOp().id());
}

void KisDlgLayerProperties::slotColorLabelValueChangedInternally()
{
    d->page->colorLabelSelector->setCurrentIndex(d->colorLabelProperty->value());
    d->page->colorLabelSelector->setEnabled(!d->colorLabelProperty->isIgnored());
}

void KisDlgLayerProperties::slotColorLabelValueChangedExternally()
{
    if (d->colorLabelProperty->isIgnored()) return;
    d->colorLabelProperty->setValue(d->page->colorLabelSelector->currentIndex());
}

void KisDlgLayerProperties::slotOpacityValueChangedInternally()
{
    d->page->intOpacity->setValue(d->opacityProperty->value());
    d->page->intOpacity->setEnabled(!d->opacityProperty->isIgnored());
}

void KisDlgLayerProperties::slotOpacityValueChangedExternally()
{
    if (d->opacityProperty->isIgnored()) return;
    d->opacityProperty->setValue(d->page->intOpacity->value());
}

void KisDlgLayerProperties::slotNameValueChangedInternally()
{
    if (d->page->editName->text() != d->nameProperty->value()) {
        d->page->editName->setText(d->nameProperty->value());
    }

    d->page->editName->setEnabled(!d->nameProperty->isIgnored());
}

void KisDlgLayerProperties::slotNameValueChangedExternally()
{
    if (d->nameProperty->isIgnored()) return;

    if (d->page->editName->text() != d->nameProperty->value()) {
        d->nameProperty->setValue(d->page->editName->text());
    }
}

void KisDlgLayerProperties::slotPropertyValueChangedInternally()
{
    Q_FOREACH (KisMultinodePropertyInterfaceSP prop, d->channelFlagsProps) {
        prop->rereadCurrentValue();
    }
}

void KisDlgLayerProperties::slotFlagsValueChangedInternally()
{
    Q_FOREACH (KisMultinodePropertyInterfaceSP prop, d->layerProperties) {
        prop->rereadCurrentValue();
    }
}

void KisDlgLayerProperties::updatePreview()
{
    KisLayerUtils::KisSimpleUpdateCommand::updateNodes(d->nodes);
}
