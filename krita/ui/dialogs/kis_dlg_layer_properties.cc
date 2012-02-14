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

#include <klineedit.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <knuminput.h>

#include <KoColorSpaceRegistry.h>
#include <KoChannelInfo.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>

#include "kis_undo_adapter.h"
#include "QTimer"
#include "commands/kis_layer_commands.h"
#include "kis_layer.h"
#include "kis_view2.h"
#include "kis_doc2.h"
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
};

KisDlgLayerProperties::KisDlgLayerProperties(KisLayerSP layer, KisView2 *view, KisDoc2 *doc, QWidget *parent, const char *name, Qt::WFlags f)
                      : KDialog(parent)
                      , m_layer(layer)
                      , m_view(view)
                      , m_doc(doc)
                      , d(new Private())
{
    Q_UNUSED(f);
    setCaption(i18n("Layer Properties"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    setModal(false);

    setObjectName(name);
    m_page = new WdgLayerProperties(this);

    d->deviceName = layer->name();
    d->colorSpace = layer->colorSpace();
    d->compositeOp = layer->compositeOp();
    d->channelFlags = layer->channelFlags();
    d->opacity = layer->opacity();
    
    quint8 sliderOpacity = int((d->opacity * 100.0) / 255 + 0.5);

    setMainWidget(m_page);

    m_page->editName->setText(d->deviceName);
    connect(m_page->editName, SIGNAL(textChanged(const QString &)), this, SLOT(slotNameChanged(const QString &)));

    m_page->lblColorSpace->setText(d->colorSpace->name());

    if (const KoColorProfile* profile = d->colorSpace->profile()) {
        m_page->lblProfile->setText(profile->name());
    }

    m_page->intOpacity->setRange(0, 100);
    m_page->intOpacity->setValue(sliderOpacity);

    m_page->cmbComposite->setEnabled(d->compositeOp);
    if(d->compositeOp) {
        m_page->cmbComposite->getModel()->validateCompositeOps(d->colorSpace);
        m_page->cmbComposite->setCurrentIndex(m_page->cmbComposite->indexOf(KoID(d->compositeOp->id())));
    }

    slotNameChanged(m_page->editName->text());

    QVBoxLayout * vbox = new QVBoxLayout;
    m_channelFlags = new KisChannelFlagsWidget(d->colorSpace);
    vbox->addWidget(m_channelFlags);
    vbox->addStretch(1);
    m_page->grpActiveChannels->setLayout(vbox);

    m_channelFlags->setChannelFlags(d->channelFlags);

    setMinimumSize(m_page->sizeHint());

    QTimer* ticker = new QTimer;
    ticker->start(200);
    connect(ticker, SIGNAL(timeout()), SLOT(updatePreview()));
}

KisDlgLayerProperties::~KisDlgLayerProperties()
{
    if (result() == QDialog::Accepted) {
        applyNewProperties();
    } else { // QDialog::Rejected
        cleanPreviewChanges();
        m_doc->setModified(true);
        m_layer->setDirty();
    }
}

bool KisDlgLayerProperties::haveChanges() const
{
    return m_layer->name() !=  getName()
        || m_layer->opacity() !=  getOpacity()
        || m_layer->channelFlags() !=  getChannelFlags()
        || (d->compositeOp &&
            m_layer->compositeOp()->id() !=  getCompositeOp());

}


void KisDlgLayerProperties::updatePreview()
{
    if (!m_layer) return;

    if(m_page->checkBoxPreview->isChecked()) {
        if (haveChanges()) {
            m_layer->setOpacity(getOpacity());
            m_layer->setCompositeOp(getCompositeOp());
            m_layer->setName(getName());
            m_layer->setChannelFlags(getChannelFlags());
            m_doc->setModified(true);
        }
        m_layer->setDirty();
    }
}

void KisDlgLayerProperties::applyNewProperties()
{
    if (!m_layer) return;
    
    cleanPreviewChanges();
    
    if (haveChanges()) {
        QApplication::setOverrideCursor(KisCursor::waitCursor());
        KUndo2Command *change = new KisLayerPropsCommand(m_layer,
                                            m_layer->opacity(),       getOpacity(),
                                            m_layer->compositeOpId(), getCompositeOp(),
                                            m_layer->name(),          getName(),
                                            m_layer->channelFlags(),  getChannelFlags(),
                                            true);
        m_view->undoAdapter()->addCommand(change);
        QApplication::restoreOverrideCursor();
        m_doc->setModified(true);
        m_layer->setDirty();
    }
}

void KisDlgLayerProperties::cleanPreviewChanges()
{
    m_layer->setOpacity(d->opacity);
    m_layer->setName(d->deviceName);
    m_layer->setChannelFlags(d->channelFlags);

    if(d->compositeOp) {
        m_layer->setCompositeOp(d->compositeOp->id());
    }
}

void KisDlgLayerProperties::slotNameChanged(const QString &_text)
{
    enableButtonOk(!_text.isEmpty());
}

QString KisDlgLayerProperties::getName() const
{
    return m_page->editName->text();
}

int KisDlgLayerProperties::getOpacity() const
{
    qint32 opacity = m_page->intOpacity->value();

    if (!opacity)
        return 0;

    opacity = int((opacity * 255.0) / 100 + 0.5);
    if (opacity > 255)
        opacity = 255;
    return opacity;
}

QString KisDlgLayerProperties::getCompositeOp() const
{
    KoID compositeOp;
    
    if(m_page->cmbComposite->entryAt(compositeOp, m_page->cmbComposite->currentIndex()))
        return compositeOp.id();
    
    return KoCompositeOpRegistry::instance().getDefaultCompositeOp().id();
}

QBitArray KisDlgLayerProperties::getChannelFlags() const
{
    QBitArray flags = m_channelFlags->channelFlags();
    for (int i = 0; i < flags.size(); ++i) {
        dbgUI << "Received flag from channelFlags widget, flag " << i << " is " << flags.testBit(i);
    }
    return flags;
}

#include "kis_dlg_layer_properties.moc"
