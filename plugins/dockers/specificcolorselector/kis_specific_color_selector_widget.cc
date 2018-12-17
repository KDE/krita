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
#include <ksharedconfig.h>

#include <KoChannelInfo.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>

#include <kis_color_input.h>
#include <KoColorProfile.h>
#include <kis_debug.h>
#include <kis_color_space_selector.h>
#include <kis_signal_compressor.h>
#include <kis_display_color_converter.h>
#include <kis_popup_button.h>
#include <kis_icon_utils.h>

#include "ui_wdgSpecificColorSelectorWidget.h"

KisSpecificColorSelectorWidget::KisSpecificColorSelectorWidget(QWidget* parent)
    : QWidget(parent)
    , m_colorSpace(0)
    , m_spacer(0)
    , m_updateCompressor(new KisSignalCompressor(10, KisSignalCompressor::POSTPONE, this))
    , m_customColorSpaceSelected(false)
    , m_displayConverter(0)
{

    m_ui = new Ui_wdgSpecificColorSelectorWidget();
    m_ui->setupUi(this);

    m_updateAllowed = true;
    connect(m_updateCompressor, SIGNAL(timeout()), SLOT(updateTimeout()));

    m_colorspaceSelector = new KisColorSpaceSelector(this);
    connect(m_colorspaceSelector, SIGNAL(colorSpaceChanged(const KoColorSpace*)), this, SLOT(setCustomColorSpace(const KoColorSpace*)));

    m_ui->colorspacePopupButton->setPopupWidget(m_colorspaceSelector);

    connect(m_ui->chkUsePercentage, SIGNAL(toggled(bool)), this, SLOT(onChkUsePercentageChanged(bool)));

    KConfigGroup cfg =  KSharedConfig::openConfig()->group(QString());
    m_ui->chkUsePercentage->setChecked(cfg.readEntry("SpecificColorSelector/UsePercentage", false));
    m_ui->chkUsePercentage->setIcon(KisIconUtils::loadIcon("ratio"));

    m_colorspaceSelector->showColorBrowserButton(false);

    m_spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_ui->slidersLayout->addItem(m_spacer);
}

KisSpecificColorSelectorWidget::~KisSpecificColorSelectorWidget()
{
    KConfigGroup cfg =  KSharedConfig::openConfig()->group(QString());
    cfg.writeEntry("SpecificColorSelector/UsePercentage", m_ui->chkUsePercentage->isChecked());
}

bool KisSpecificColorSelectorWidget::customColorSpaceUsed()
{
    return m_customColorSpaceSelected;
}

void KisSpecificColorSelectorWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (m_colorSpace) {
        QString elidedColorspaceName = m_ui->colorspacePopupButton->fontMetrics().elidedText(
                    m_colorSpace->name(), Qt::ElideRight,
                    m_ui->colorspacePopupButton->width()
                    );
        m_ui->colorspacePopupButton->setText(elidedColorspaceName);
    }
}

void KisSpecificColorSelectorWidget::setDisplayConverter(KisDisplayColorConverter *displayConverter)
{
    const bool needsForceUpdate = m_displayConverter != displayConverter;

    m_displayConverter = displayConverter;

    if (m_displayConverter) {
        m_converterConnection.clear();
        m_converterConnection.addConnection(m_displayConverter, SIGNAL(displayConfigurationChanged()), this, SLOT(rereadCurrentColorSpace()), Qt::UniqueConnection);
    }

    rereadCurrentColorSpace(needsForceUpdate);
}

void KisSpecificColorSelectorWidget::rereadCurrentColorSpace(bool force)
{
    if (m_displayConverter && !m_customColorSpaceSelected) {
        m_colorSpace = m_displayConverter->paintingColorSpace();
    }

    setColorSpace(m_colorSpace, force);
    setColor(m_color);
}

void KisSpecificColorSelectorWidget::setColorSpace(const KoColorSpace* cs, bool force)
{
    Q_ASSERT(cs);
    dbgPlugins << cs->id() << " " << cs->profile()->name();

    if (*m_colorSpace == *cs && !force) {
        Q_FOREACH (KisColorInput* input, m_inputs) {
            input->update();
        }

        return;
    }

    if (cs->colorDepthId() == Integer8BitsColorDepthID || cs->colorDepthId() == Integer16BitsColorDepthID) {
        m_ui->chkUsePercentage->setVisible(true);
    } else {
        m_ui->chkUsePercentage->setVisible(false);
    }

    m_colorSpace = KoColorSpaceRegistry::instance()->colorSpace(cs->colorModelId().id(), cs->colorDepthId().id(), cs->profile());
    Q_ASSERT(m_colorSpace);
    Q_ASSERT(*m_colorSpace == *cs);

    QString elidedColorspaceName = m_ui->colorspacePopupButton->fontMetrics().elidedText(
                m_colorSpace->name(), Qt::ElideRight,
                m_ui->colorspacePopupButton->width()
                );
    m_ui->colorspacePopupButton->setText(elidedColorspaceName);

    m_color = KoColor(m_color, m_colorSpace);
    Q_FOREACH (KisColorInput* input, m_inputs) {
        delete input;
    }
    m_inputs.clear();

    m_ui->slidersLayout->removeItem(m_spacer);

    QList<KoChannelInfo *> channels = KoChannelInfo::displayOrderSorted(m_colorSpace->channels());

    KoColorDisplayRendererInterface *displayRenderer =
        m_displayConverter ?
        m_displayConverter->displayRendererInterface() :
        KisDisplayColorConverter::dumbConverterInstance()->displayRendererInterface();

    Q_FOREACH (KoChannelInfo* channel, channels) {
        if (channel->channelType() == KoChannelInfo::COLOR) {
            KisColorInput* input = 0;
            switch (channel->channelValueType()) {
            case KoChannelInfo::UINT8:
            case KoChannelInfo::UINT16:
            case KoChannelInfo::UINT32: {
                input = new KisIntegerColorInput(this, channel, &m_color, displayRenderer, m_ui->chkUsePercentage->isChecked());
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
                m_ui->slidersLayout->addWidget(input);
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
    Q_FOREACH (KoChannelInfo* channel, channels) {
        if (channel->channelType() == KoChannelInfo::COLOR && channel->channelValueType() != KoChannelInfo::UINT8) {
            allChannels8Bit = false;
        }
    }
    if (allChannels8Bit) {
        KisColorInput* input = new KisHexColorInput(this, &m_color, displayRenderer);
        m_inputs.append(input);
        m_ui->slidersLayout->addWidget(input);
        connect(input, SIGNAL(updated()), this,  SLOT(update()));
        connect(this,  SIGNAL(updated()), input, SLOT(update()));
    }
    m_ui->slidersLayout->addItem(m_spacer);

    m_colorspaceSelector->blockSignals(true);
    m_colorspaceSelector->setCurrentColorSpace(cs);
    m_colorspaceSelector->blockSignals(false);

    m_updateAllowed = false;
    emit(updated());
    m_updateAllowed = true;
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

void KisSpecificColorSelectorWidget::onChkUsePercentageChanged(bool isChecked)
{
    for (auto input: m_inputs) {
        input->setPercentageWise(isChecked);
    }
    emit(updated());
}
