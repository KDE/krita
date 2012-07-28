/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_specific_color_selector_widget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>
#include <QCheckBox>

#include <klocale.h>
#include <kconfiggroup.h>
#include <kconfig.h>
#include <kglobal.h>

#include <KoChannelInfo.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_color_input.h"
#include <KoColorProfile.h>
#include "kis_debug.h"
#include "kis_color_space_selector.h"

KisSpecificColorSelectorWidget::KisSpecificColorSelectorWidget(QWidget* parent)
    : QWidget(parent)
    , m_colorSpace(0)
    , m_customColorSpaceSelected(false)
{
    m_layout = new QVBoxLayout(this);
    m_updateAllowed = true;
    m_delayTimer = new QTimer(this);
    m_delayTimer->setInterval(50);
    connect(m_delayTimer, SIGNAL(timeout()), this, SLOT(updateTimeout()));


    m_colorspaceSelector = new KisColorSpaceSelector(this);
    connect(m_colorspaceSelector, SIGNAL(colorSpaceChanged(const KoColorSpace*)), this, SLOT(setCustomColorSpace(const KoColorSpace*)));

    m_chkShowColorSpaceSelector = new QCheckBox(i18n("Show Colorspace Selector"), this);
    connect(m_chkShowColorSpaceSelector, SIGNAL(toggled(bool)), m_colorspaceSelector, SLOT(setVisible(bool)));

    KConfigGroup cfg = KGlobal::config()->group("");
    m_chkShowColorSpaceSelector->setChecked(cfg.readEntry("SpecificColorSelector/ShowColorSpaceSelector", true));
    m_colorspaceSelector->setVisible(m_chkShowColorSpaceSelector->isChecked());
    m_layout->addWidget(m_chkShowColorSpaceSelector);
    m_layout->addWidget(m_colorspaceSelector);


}

KisSpecificColorSelectorWidget::~KisSpecificColorSelectorWidget()
{
    KConfigGroup cfg = KGlobal::config()->group("");
    cfg.writeEntry("SpecificColorSelector/ShowColorSpaceSelector", m_chkShowColorSpaceSelector->isChecked());

}

bool KisSpecificColorSelectorWidget::customColorSpaceUsed()
{
    return m_customColorSpaceSelected;
}

void KisSpecificColorSelectorWidget::setColorSpace(const KoColorSpace* cs)
{
    Q_ASSERT(cs);
    if (m_colorSpace && *m_colorSpace == *cs) return;
    dbgPlugins << cs->id() << " " << cs->profile()->name();
    m_colorSpace = KoColorSpaceRegistry::instance()->colorSpace(cs->colorModelId().id(), cs->colorDepthId().id(), cs->profile());
    Q_ASSERT(m_colorSpace);
    Q_ASSERT(*m_colorSpace == *cs);
    m_color = KoColor(m_color, m_colorSpace);
    foreach(KisColorInput* input, m_inputs) {
        delete input;
    }
    m_inputs.clear();

    QList<KoChannelInfo *> channels = KoChannelInfo::displayOrderSorted(m_colorSpace->channels());

    foreach(KoChannelInfo* channel, channels) {
        if (channel->channelType() == KoChannelInfo::COLOR) {
            KisColorInput* input = 0;
            switch (channel->channelValueType()) {
            case KoChannelInfo::UINT8:
            case KoChannelInfo::UINT16:
            case KoChannelInfo::UINT32: {
                input = new KisIntegerColorInput(this, channel, &m_color);
            }
            break;
            case KoChannelInfo::FLOAT16:
            case KoChannelInfo::FLOAT32: {
                input = new KisFloatColorInput(this, channel, &m_color);
            }
            break;
            default:
                Q_ASSERT(false);
                input = 0;
            }
            if (input) {
                connect(input, SIGNAL(updated()), this,  SLOT(update()));
                connect(this,  SIGNAL(updated()), input, SLOT(update()));

                m_inputs.append(input);
                m_layout->addWidget(input);
            }
        }
    }
    bool allChannels8Bit = true;
    foreach (KoChannelInfo* channel, channels) {
        if (channel->channelType() == KoChannelInfo::COLOR && channel->channelValueType() != KoChannelInfo::UINT8) {
            allChannels8Bit = false;
        }
    }
    if (allChannels8Bit) {
        KisColorInput* input = new KisHexColorInput(this, &m_color);
        m_inputs.append(input);
        m_layout->addWidget(input);
        connect(input, SIGNAL(updated()), this,  SLOT(update()));
        connect(this,  SIGNAL(updated()), input, SLOT(update()));
    }
    m_layout->addStretch(10);

    m_colorspaceSelector->blockSignals(true);
    m_colorspaceSelector->setCurrentColorSpace(cs);
    m_colorspaceSelector->blockSignals(false);
}

void KisSpecificColorSelectorWidget::update()
{
    m_delayTimer->start();
}

void KisSpecificColorSelectorWidget::setColor(const KoColor& c)
{
    m_updateAllowed = false;
    m_color.fromKoColor(c);
    emit(updated());
    m_updateAllowed = true;
}

void KisSpecificColorSelectorWidget::updateTimeout()
{
    if (m_updateAllowed)
        emit(colorChanged(m_color));
    m_delayTimer->stop();
}

void KisSpecificColorSelectorWidget::setCustomColorSpace(const KoColorSpace *colorSpace)
{
    m_customColorSpaceSelected = true;
    setColorSpace(colorSpace);
    setColor(m_color);
}
