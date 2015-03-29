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
#include <QSlider>
#include <QString>
#include <QBitArray>
#include <QVector>
#include <QGroupBox>
#include <QVBoxLayout>

#include <klocale.h>

#include <KoChannelInfo.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>

#include "kis_undo_adapter.h"
#include "QTimer"
#include "commands/kis_layer_commands.h"
#include "kis_layer.h"
#include "KisViewManager.h"
#include "KisDocument.h"
#include "kis_cursor.h"
#include <kis_debug.h>
#include <kis_global.h>

#include "widgets/squeezedcombobox.h"

#include "widgets/kis_cmb_composite.h"
#include "widgets/kis_cmb_idlist.h"
#include "KoColorProfile.h"
#include "widgets/kis_channelflags_widget.h"
#include <kis_composite_ops_model.h>


struct KisDlgLayerProperties::Private
{
    QString deviceName;
    const KoColorSpace *colorSpace;
    const KoCompositeOp *compositeOp;
    QBitArray channelFlags;
    quint8 opacity;
    KisLayerSP layer;
    KisViewManager *view;
    KisDocument *doc;
    WdgLayerProperties *page;
    KisChannelFlagsWidget *channelFlagsWidget;
    QTimer previewTimer;
};

KisDlgLayerProperties::KisDlgLayerProperties(KisLayerSP layer, KisViewManager *view, KisDocument *doc, QWidget *parent, const char *name, Qt::WFlags f)
    : KDialog(parent)
    , d(new Private())
{

    Q_UNUSED(f);
    setCaption(i18n("Layer Properties"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    setModal(false);

    setObjectName(name);
    d->page = new WdgLayerProperties(this);
    d->layer = layer;
    d->view = view;
    d->doc = doc;
    d->deviceName = layer->name();
    d->colorSpace = layer->colorSpace();
    d->compositeOp = layer->compositeOp();
    d->channelFlags = layer->channelFlags();
    d->opacity = layer->opacity();

    quint8 sliderOpacity = int((d->opacity * 100.0) / 255 + 0.5);

    setMainWidget(d->page);

    d->page->editName->setText(d->deviceName);
    d->page->editName->setFocus();
    connect(d->page->editName, SIGNAL(textChanged(const QString &)), this, SLOT(slotNameChanged(const QString &)));

    d->page->lblColorSpace->setText(d->colorSpace->name());

    if (const KoColorProfile* profile = d->colorSpace->profile()) {
        d->page->lblProfile->setText(profile->name());
    }

    d->page->intOpacity->setRange(0, 100);
    d->page->intOpacity->setValue(sliderOpacity);
    d->page->intOpacity->setSuffix("%");
    connect(d->page->intOpacity, SIGNAL(valueChanged(int)), SLOT(kickTimer()));

    d->page->cmbComposite->setEnabled(d->compositeOp);
    connect(d->page->cmbComposite, SIGNAL(currentIndexChanged(int)), SLOT(kickTimer()));

    if (d->compositeOp) {
        d->page->cmbComposite->validate(d->colorSpace);
        d->page->cmbComposite->selectCompositeOp(KoID(d->compositeOp->id()));
    }

    slotNameChanged(d->page->editName->text());

    QVBoxLayout * vbox = new QVBoxLayout;
    d->channelFlagsWidget = new KisChannelFlagsWidget(d->colorSpace);
    connect(d->channelFlagsWidget, SIGNAL(channelSelectionChanced()), SLOT(kickTimer()));

    vbox->addWidget(d->channelFlagsWidget);
    vbox->addStretch(1);
    d->page->grpActiveChannels->setLayout(vbox);

    d->channelFlagsWidget->setChannelFlags(d->channelFlags);

    setMinimumSize(d->page->sizeHint());

    connect(&d->previewTimer, SIGNAL(timeout()), SLOT(updatePreview()));
}

KisDlgLayerProperties::~KisDlgLayerProperties()
{
    if (result() == QDialog::Accepted) {
        applyNewProperties();
    }
    else { // QDialog::Rejected
        cleanPreviewChanges();
        d->doc->setModified(true);
        d->layer->setDirty();
    }

    delete d;
}

void KisDlgLayerProperties::updatePreview()
{
    if (!d->layer) return;

    if (d->page->checkBoxPreview->isChecked()) {
        d->layer->setOpacity(getOpacity());
        d->layer->setCompositeOp(getCompositeOp());
        d->layer->setName(getName());
        d->layer->setChannelFlags(getChannelFlags());
        d->doc->setModified(true);
        d->layer->setDirty();
    }
}


bool KisDlgLayerProperties::haveChanges() const
{
    return d->layer->name() !=  getName()
        || d->layer->opacity() !=  getOpacity()
        || d->layer->channelFlags() !=  getChannelFlags()
        || (d->compositeOp && d->layer->compositeOp() &&
            d->layer->compositeOp()->id()!= getCompositeOp());

}


void KisDlgLayerProperties::applyNewProperties()
{
    if (!d->layer) return;

    cleanPreviewChanges();

    if (haveChanges()) {
        QApplication::setOverrideCursor(KisCursor::waitCursor());
        KUndo2Command *change = new KisLayerPropsCommand(d->layer,
                                                         d->layer->opacity(),       getOpacity(),
                                                         d->layer->compositeOpId(), getCompositeOp(),
                                                         d->layer->name(),          getName(),
                                                         d->layer->channelFlags(),  getChannelFlags(),
                                                         true);
        d->view->undoAdapter()->addCommand(change);
        QApplication::restoreOverrideCursor();
        d->doc->setModified(true);
        d->layer->setDirty();
    }
}

void KisDlgLayerProperties::cleanPreviewChanges()
{
    d->layer->setOpacity(d->opacity);
    d->layer->setName(d->deviceName);
    d->layer->setChannelFlags(d->channelFlags);

    if (d->compositeOp) {
        d->layer->setCompositeOp(d->compositeOp->id());
    }
}

void KisDlgLayerProperties::kickTimer()
{
    d->previewTimer.start(200);
}

void KisDlgLayerProperties::slotNameChanged(const QString &_text)
{
    enableButtonOk(!_text.isEmpty());
}

QString KisDlgLayerProperties::getName() const
{
    return d->page->editName->text();
}

int KisDlgLayerProperties::getOpacity() const
{
    qint32 opacity = d->page->intOpacity->value();
    if (opacity > 0 ) {
        opacity = int((opacity * 255.0) / 100 + 0.5);
    }
    if (opacity > 255) {
        opacity = 255;
    }
    return opacity;
}

QString KisDlgLayerProperties::getCompositeOp() const
{
    return d->page->cmbComposite->selectedCompositeOp().id();
}

QBitArray KisDlgLayerProperties::getChannelFlags() const
{
    QBitArray flags = d->channelFlagsWidget->channelFlags();
    for (int i = 0; i < flags.size(); ++i) {
        dbgUI << "Received flag from channelFlags widget, flag " << i << " is " << flags.testBit(i);
    }
    return flags;
}

