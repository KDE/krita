/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#include <kis_debug.h>
#include <kis_global.h>

#include "widgets/squeezedcombobox.h"

#include "widgets/kis_cmb_composite.h"
#include "widgets/kis_cmb_idlist.h"
#include "KoColorProfile.h"
#include "widgets/kis_channelflags_widget.h"

KisDlgLayerProperties::KisDlgLayerProperties(const QString& deviceName,
        qint32 opacity,
        const KoCompositeOp* compositeOp,
        const KoColorSpace * colorSpace,
        const QBitArray & channelFlags,
        QWidget *parent, const char *name, Qt::WFlags f)
        : KDialog(parent)
        , m_colorSpace(colorSpace)
{
    Q_UNUSED(f);
    setCaption(i18n("Layer Properties"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    setObjectName(name);
    m_page = new WdgLayerProperties(this);

    opacity = int((opacity * 100.0) / 255 + 0.5);

    setMainWidget(m_page);

    m_page->editName->setText(deviceName);
    connect(m_page->editName, SIGNAL(textChanged(const QString &)), this, SLOT(slotNameChanged(const QString &)));

    m_page->lblColorSpace->setText(colorSpace->name());

    if (const KoColorProfile* profile = colorSpace->profile()) {
        m_page->lblProfile->setText(profile->name());
    }

    m_page->intOpacity->setMinimum(0);
    m_page->intOpacity->setMaximum(100);
    m_page->intOpacity->setValue(opacity);

    m_page->cmbComposite->setCompositeOpList(colorSpace->compositeOps());
    m_page->cmbComposite->setCurrent(compositeOp);

    slotNameChanged(m_page->editName->text());

    QVBoxLayout * vbox = new QVBoxLayout;
    m_channelFlags = new KisChannelFlagsWidget(colorSpace);
    vbox->addWidget(m_channelFlags);
    vbox->addStretch(1);
    m_page->grpActiveChannels->setLayout(vbox);
    m_channelFlags->setChannelFlags(channelFlags);

    setMinimumSize(m_page->sizeHint());

}

KisDlgLayerProperties::~KisDlgLayerProperties()
{
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

const QString& KisDlgLayerProperties::getCompositeOp() const
{
    return m_page->cmbComposite->currentItem();
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
