/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2015 Moritz Molch <kde@moritzmolch.de>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_specific_color_selector_widget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QSpacerItem>

#include <klocalizedstring.h>
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
#include "kis_signal_compressor.h"


KisSpecificColorSelectorWidget::KisSpecificColorSelectorWidget(QWidget* parent)
    : QWidget(parent)
    , m_colorSpace(0)
    , m_spacer(0)
    , m_updateCompressor(new KisSignalCompressor(10, KisSignalCompressor::POSTPONE, this))
    , m_customColorSpaceSelected(false)
    , m_displayRenderer(0)
    , m_fallbackRenderer(new KoDumbColorDisplayRenderer())
{

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0,0,0,0);
    m_layout->setSpacing(1);
    m_updateAllowed = true;
    connect(m_updateCompressor, SIGNAL(timeout()), SLOT(updateTimeout()));

    m_colorspaceSelector = new KisColorSpaceSelector(this);
    m_colorspaceSelector->layout()->setSpacing(1);
    connect(m_colorspaceSelector, SIGNAL(colorSpaceChanged(const KoColorSpace*)), this, SLOT(setCustomColorSpace(const KoColorSpace*)));

    m_chkShowColorSpaceSelector = new QCheckBox(i18n("Show Colorspace Selector"), this);
    connect(m_chkShowColorSpaceSelector, SIGNAL(toggled(bool)), m_colorspaceSelector, SLOT(setVisible(bool)));

    KConfigGroup cfg =  KSharedConfig::openConfig()->group("");
    m_chkShowColorSpaceSelector->setChecked(cfg.readEntry("SpecificColorSelector/ShowColorSpaceSelector", true));

    m_colorspaceSelector->setVisible(m_chkShowColorSpaceSelector->isChecked());
    m_layout->addWidget(m_chkShowColorSpaceSelector);
    m_layout->addWidget(m_colorspaceSelector);

    m_spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_layout->addItem(m_spacer);
}

KisSpecificColorSelectorWidget::~KisSpecificColorSelectorWidget()
{
    KConfigGroup cfg =  KSharedConfig::openConfig()->group("");
    cfg.writeEntry("SpecificColorSelector/ShowColorSpaceSelector", m_chkShowColorSpaceSelector->isChecked());

}

bool KisSpecificColorSelectorWidget::customColorSpaceUsed()
{
    return m_customColorSpaceSelected;
}

void KisSpecificColorSelectorWidget::setDisplayRenderer(KoColorDisplayRendererInterface *displayRenderer)
{
    m_displayRenderer = displayRenderer;

    if (m_colorSpace) {
        setColorSpace(m_colorSpace);
    }
}

void KisSpecificColorSelectorWidget::setColorSpace(const KoColorSpace* cs)
{
    Q_ASSERT(cs);
    dbgPlugins << cs->id() << " " << cs->profile()->name();

    if (cs == m_colorSpace) {
        return;
    }

    m_colorSpace = KoColorSpaceRegistry::instance()->colorSpace(cs->colorModelId().id(), cs->colorDepthId().id(), cs->profile());
    Q_ASSERT(m_colorSpace);
    Q_ASSERT(*m_colorSpace == *cs);
    m_color = KoColor(m_color, m_colorSpace);
    foreach(KisColorInput* input, m_inputs) {
        delete input;
    }
    m_inputs.clear();

    m_layout->removeItem(m_spacer);

    QList<KoChannelInfo *> channels = KoChannelInfo::displayOrderSorted(m_colorSpace->channels());

    KoColorDisplayRendererInterface *displayRenderer = m_displayRenderer ? m_displayRenderer : m_fallbackRenderer;

    foreach(KoChannelInfo* channel, channels) {
        if (channel->channelType() == KoChannelInfo::COLOR) {
            KisColorInput* input = 0;
            switch (channel->channelValueType()) {
            case KoChannelInfo::UINT8:
            case KoChannelInfo::UINT16:
            case KoChannelInfo::UINT32: {
                input = new KisIntegerColorInput(this, channel, &m_color, displayRenderer);
            }
            break;
            case KoChannelInfo::FLOAT16:
            case KoChannelInfo::FLOAT32: {
                input = new KisFloatColorInput(this, channel, &m_color, displayRenderer);
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

    QList<QLabel*> labels;
    int labelWidth = 0;

    Q_FOREACH (KisColorInput* input, m_inputs) {
        Q_FOREACH (QLabel* label, input->findChildren<QLabel*>()) {
            labels.append(label);
            labelWidth = qMax(labelWidth, label->sizeHint().width());
        }
    }

    Q_FOREACH (QLabel *label, labels) {
        label->setMinimumWidth(labelWidth);
    }

    bool allChannels8Bit = true;
    foreach (KoChannelInfo* channel, channels) {
        if (channel->channelType() == KoChannelInfo::COLOR && channel->channelValueType() != KoChannelInfo::UINT8) {
            allChannels8Bit = false;
        }
    }
    if (allChannels8Bit) {
        KisColorInput* input = new KisHexColorInput(this, &m_color, displayRenderer);
        m_inputs.append(input);
        m_layout->addWidget(input);
        connect(input, SIGNAL(updated()), this,  SLOT(update()));
        connect(this,  SIGNAL(updated()), input, SLOT(update()));
    }
    m_layout->addItem(m_spacer);

    m_colorspaceSelector->blockSignals(true);
    m_colorspaceSelector->setCurrentColorSpace(cs);
    m_colorspaceSelector->blockSignals(false);
}

void KisSpecificColorSelectorWidget::update()
{
    if (m_updateAllowed) {
        m_updateCompressor->start();
    }
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
    emit(colorChanged(m_color));
}

void KisSpecificColorSelectorWidget::setCustomColorSpace(const KoColorSpace *colorSpace)
{
    m_customColorSpaceSelected = true;
    setColorSpace(colorSpace);
    setColor(m_color);
}

#include "moc_kis_specific_color_selector_widget.cpp"