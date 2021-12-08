/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Frederic Coiffier <fcoiffie@gmail.com>
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <cmath>

#include <klocalizedstring.h>

#include <QtGlobal>
#include <QPixmap>
#include <QSpinBox>
#include <QMessageBox>
#include <QEvent>
#include <QPainter>
#include <QCheckBox>

#include <KoBasicHistogramProducers.h>
#include <kis_paint_device.h>
#include <kis_histogram.h>
#include <kis_painter.h>
#include <kis_processing_information.h>
#include <kis_selection.h>
#include <kis_types.h>
#include <KisGlobalResourcesInterface.h>
#include <kis_color_transformation_configuration.h>
#include <KoColorSpaceMaths.h>
#include <kis_signals_blocker.h>
#include <kis_icon_utils.h>
#include <KoColorModelStandardIds.h>
#include <kis_painting_tweaks.h>
#include <KoDialog.h>
#include <KisAutoLevels.h>
#include <KisAutoLevelsWidget.h>

#include "../colorsfilters/kis_multichannel_utils.h"

#include "KisLevelsConfigWidget.h"
#include "KisLevelsFilter.h"

static int deNormalizeValue(qreal value, int min, int max)
{
    return min + static_cast<int>(qRound(static_cast<qreal>(max - min) * value));
}

static qreal normalizeValue(int value, int min, int max)
{
    return static_cast<qreal>(value - min) / static_cast<qreal>(max - min);
}

KisLevelsConfigWidget::KisLevelsConfigWidget(QWidget * parent, KisPaintDeviceSP dev, const KoColorSpace* colorSpace)
    : KisConfigWidget(parent)
    , m_dev(dev)
    , m_colorSpace(colorSpace)
    , m_activeChannel(0)
    , m_activeLevelsCurve(nullptr)
    , m_channelsHistogram(nullptr)
    , m_lightnessHistogram(nullptr)
{
    Q_ASSERT(m_dev);
    Q_ASSERT(m_colorSpace);

    m_virtualChannels = KisMultiChannelUtils::getVirtualChannels(m_colorSpace);
    computeChannelsMinMaxRanges();

    m_page.setupUi(this);
    // In some styles the combo box is higher than the tool buttons so
    // hiding/unhiding it can make the layout jump in that area.
    // We set the spacer item height to the same as the combo box
    m_page.spacer01->changeSize(0, m_page.comboBoxChannel->sizeHint().height(),
                                QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Only enable the "all-channels auto levels" button if the color space
    // is rgb
    m_page.buttonAutoLevelsAllChannels->setEnabled(m_colorSpace->colorModelId() == RGBAColorModelID);

    setButtonsIcons();

    const KisLevelsCurve defaultLevelsCurve = KisLevelsFilterConfiguration::defaultLevelsCurve();
    for (int i = 0; i < m_virtualChannels.size(); ++i) {
        m_levelsCurves.append(defaultLevelsCurve);
        m_levelsCurves[i].setName(m_virtualChannels[i].name());
        m_page.comboBoxChannel->addItem(m_virtualChannels[i].name(), i);
    }
    m_lightnessLevelsCurve.setName(i18nc("Lightness value in Lab color model", "Lightness"));

    m_activeLevelsCurve = &m_lightnessLevelsCurve;
    m_activeChannelMin = m_lightnessMinMaxRanges.first;
    m_activeChannelMax = m_lightnessMinMaxRanges.second;

    updateHistograms();
    updateWidgets();

    connect(m_page.buttonGroupMode, SIGNAL(buttonToggled(QAbstractButton*, bool)), SLOT(slot_buttonGroupMode_buttonToggled(QAbstractButton*, bool)));
    connect(m_page.comboBoxChannel, SIGNAL(activated(int)), SLOT(slot_comboBoxChannel_activated(int)));
    connect(m_page.buttonGroupHistogramMode, SIGNAL(buttonToggled(QAbstractButton*, bool)), SLOT(slot_buttonGroupHistogramMode_buttonToggled(QAbstractButton*, bool)));
    connect(m_page.buttonScaleHistogramToFit, SIGNAL(clicked()), m_page.widgetHistogram, SLOT(setScaleToFit()));
    connect(m_page.buttonScaleHistogramToCutLongPeaks, SIGNAL(clicked()), m_page.widgetHistogram, SLOT(setScaleToCutLongPeaks()));
    connect(m_page.buttonResetAll, SIGNAL(clicked()), SLOT(resetAll()));
    connect(m_page.buttonResetInputLevels, SIGNAL(clicked()), SLOT(resetInputLevels()));
    connect(m_page.buttonResetOutputLevels, SIGNAL(clicked()), SLOT(resetOutputLevels()));
    connect(m_page.buttonResetAllChannels, SIGNAL(clicked()), SLOT(resetAllChannels()));
    connect(m_page.spinBoxInputBlackPoint, SIGNAL(valueChanged(int)), SLOT(slot_spinBoxInputBlackPoint_valueChanged(int)));
    connect(m_page.spinBoxInputWhitePoint, SIGNAL(valueChanged(int)), SLOT(slot_spinBoxInputWhitePoint_valueChanged(int)));
    connect(m_page.spinBoxInputGamma, SIGNAL(valueChanged(qreal)), SLOT(slot_spinBoxInputGamma_valueChanged(qreal)));
    connect(m_page.spinBoxOutputBlackPoint, SIGNAL(valueChanged(int)), SLOT(slot_spinBoxOutputBlackPoint_valueChanged(int)));
    connect(m_page.spinBoxOutputWhitePoint, SIGNAL(valueChanged(int)), SLOT(slot_spinBoxOutputWhitePoint_valueChanged(int)));
    connect(m_page.sliderInputLevels, SIGNAL(blackPointChanged(qreal)), SLOT(slot_sliderInputLevels_blackPointChanged(qreal)));
    connect(m_page.sliderInputLevels, SIGNAL(whitePointChanged(qreal)), SLOT(slot_sliderInputLevels_whitePointChanged(qreal)));
    connect(m_page.sliderInputLevels, SIGNAL(gammaChanged(qreal)), SLOT(slot_sliderInputLevels_gammaChanged(qreal)));
    connect(m_page.sliderOutputLevels, SIGNAL(blackPointChanged(qreal)), SLOT(slot_sliderOutputLevels_blackPointChanged(qreal)));
    connect(m_page.sliderOutputLevels, SIGNAL(whitePointChanged(qreal)), SLOT(slot_sliderOutputLevels_whitePointChanged(qreal)));
    connect(m_page.buttonAutoLevels, SIGNAL(clicked()), SLOT(slot_buttonAutoLevels_clicked()));
    connect(m_page.buttonAutoLevelsAllChannels, SIGNAL(clicked()), SLOT(slot_buttonAutoLevelsAllChannels_clicked()));
}

KisLevelsConfigWidget::~KisLevelsConfigWidget()
{}

KisPropertiesConfigurationSP KisLevelsConfigWidget::configuration() const
{
    KisLevelsFilterConfiguration *config =
        new KisLevelsFilterConfiguration(m_virtualChannels.size(), KisGlobalResourcesInterface::instance());

    KIS_ASSERT_RECOVER(m_activeChannel < m_levelsCurves.size()) { return config; }

    config->setLevelsCurves(m_levelsCurves);
    config->setLightnessLevelsCurve(m_lightnessLevelsCurve);
    config->setUseLightnessMode(m_page.buttonLightnessMode->isChecked());
    config->setShowLogarithmicHistogram(m_page.buttonLogarithmicHistogram->isChecked());

    return config;
}

void KisLevelsConfigWidget::setConfiguration(const KisPropertiesConfigurationSP config)
{
    const KisLevelsFilterConfiguration *filterConfig =
        dynamic_cast<const KisLevelsFilterConfiguration *>(config.data());
    KIS_SAFE_ASSERT_RECOVER_RETURN(filterConfig);

    {
        KisSignalsBlocker blocker(this, m_page.buttonLightnessMode, m_page.buttonAllChannelsMode);
        
        if (filterConfig->levelsCurves().empty() || filterConfig->levelsCurves().size() > m_virtualChannels.size()) {
            /**
             * HACK ALERT: our configuration factory generates
             * default configuration with nTransfers==0.
             * Catching it here. Just set everything to defaults instead.
             * Also reset to default if the mode is not "advanced" but the
             * configuration was created for a different colorspace.
             */
            KisPropertiesConfigurationSP defaultConfiguration =
                new KisLevelsFilterConfiguration(m_virtualChannels.size(), KisGlobalResourcesInterface::instance());
            KisLevelsFilterConfiguration *defaultFilterConfig =
                dynamic_cast<KisLevelsFilterConfiguration*>(defaultConfiguration.data());
            KIS_SAFE_ASSERT_RECOVER_RETURN(defaultFilterConfig);

            if (filterConfig->levelsCurves().size() > m_virtualChannels.size()) {
                QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("The current configuration was created for a different colorspace and cannot be used.\nThe channels will be reset."));
                warnKrita << "WARNING: trying to load levels info with invalid number of channels!";
                warnKrita << "WARNING:   expected:" << m_virtualChannels.size();
                warnKrita << "WARNING:        got:" << filterConfig->levelsCurves().size();
            }

            defaultFilterConfig->setLightnessLevelsCurve(filterConfig->lightnessLevelsCurve());
            defaultFilterConfig->setUseLightnessMode(filterConfig->useLightnessMode());
            defaultFilterConfig->setShowLogarithmicHistogram(filterConfig->showLogarithmicHistogram());
            setConfiguration(defaultConfiguration);

            return;
        }

        if (filterConfig->levelsCurves().size() < m_virtualChannels.size()) {
            // The configuration does not cover all our channels.
            // This happens when loading a document from an older version, which supported fewer channels.
            // Reset to make sure the unspecified channels have their default values.
            resetAllChannels();
        }

        for (int ch = 0; ch < filterConfig->levelsCurves().size(); ++ch) {
            m_levelsCurves[ch] = filterConfig->levelsCurves()[ch];
        }

        m_lightnessLevelsCurve = filterConfig->lightnessLevelsCurve();


        if (filterConfig->showLogarithmicHistogram()) {
            m_page.buttonLogarithmicHistogram->setChecked(true);
        } else {
            m_page.buttonLinearHistogram->setChecked(true);
        }

        if (filterConfig->useLightnessMode()) {
            m_page.buttonLightnessMode->setChecked(true);
            slot_buttonGroupMode_buttonToggled(m_page.buttonLightnessMode, true);
        } else {
            m_page.buttonAllChannelsMode->setChecked(true);
            slot_buttonGroupMode_buttonToggled(m_page.buttonAllChannelsMode, true);
        }
    }

    emit sigConfigurationItemChanged();
}

void KisLevelsConfigWidget::resetAll()
{
    m_activeLevelsCurve->resetAll();
    updateWidgets();
    emit sigConfigurationItemChanged();
}

void KisLevelsConfigWidget::resetInputLevels()
{
    m_activeLevelsCurve->resetInputLevels();
    updateWidgets();
    emit sigConfigurationItemChanged();
}

void KisLevelsConfigWidget::resetOutputLevels()
{
    m_activeLevelsCurve->resetOutputLevels();
    updateWidgets();
    emit sigConfigurationItemChanged();
}

void KisLevelsConfigWidget::resetAllChannels()
{
    for (KisLevelsCurve &levelsCurve : m_levelsCurves) {
        levelsCurve.resetAll();
    }
    updateWidgets();
    emit sigConfigurationItemChanged();
}

void KisLevelsConfigWidget::setActiveChannel(int ch)
{
    m_activeChannel = ch;
    if (m_page.buttonAllChannelsMode->isChecked()) {
        m_activeChannelMin = m_virtualChannelsMinMaxRanges[m_activeChannel].first;
        m_activeChannelMax = m_virtualChannelsMinMaxRanges[m_activeChannel].second;
        m_activeLevelsCurve = &m_levelsCurves[m_activeChannel];
        updateWidgets();
    }
}

void KisLevelsConfigWidget::computeChannelsMinMaxRanges()
{
    m_virtualChannelsMinMaxRanges.clear();
    for (int i = 0; i < m_virtualChannels.size(); ++i) {
        const VirtualChannelInfo &channel = m_virtualChannels[i];
        const KoChannelInfo::enumChannelValueType valueType = channel.valueType();
        
        switch (valueType) {
        case KoChannelInfo::UINT8:
            m_virtualChannelsMinMaxRanges.append({KoColorSpaceMathsTraits<quint8>::min, KoColorSpaceMathsTraits<quint8>::max});
            break;
        case KoChannelInfo::UINT16:
            m_virtualChannelsMinMaxRanges.append({KoColorSpaceMathsTraits<quint16>::min, KoColorSpaceMathsTraits<quint16>::max});
            break;
        default:
            //Hack Alert: should be changed to float
            int min, max;
            if (m_colorSpace->colorModelId() == LABAColorModelID || m_colorSpace->colorModelId() == CMYKAColorModelID) {
                if (i < m_dev->colorSpace()->channels().length()) {
                    min = m_colorSpace->channels()[i]->getUIMin();
                    max = m_colorSpace->channels()[i]->getUIMax();
                } else {
                    // it must be "Hue", "Saturation" or other "channel" that isn't actually accessible in the color space
                    min = 0;
                    // specific number apparently doesn't matter,
                    // if there is 255, it will work just fine, too
                    max = 100;
                }
            } else {
                min = 0;
                max = 100;
            }
            m_virtualChannelsMinMaxRanges.append({min, max});
            break;
        }
    }
    for (int i = 0; i < m_virtualChannels.size(); ++i) {
        if (m_virtualChannels[i].type() == VirtualChannelInfo::REAL) {
            m_lightnessMinMaxRanges = m_virtualChannelsMinMaxRanges[i];
            break;
        }
    }
}

void KisLevelsConfigWidget::updateWidgets()
{
    KisSignalsBlocker blocker1(m_page.comboBoxChannel, m_page.sliderInputLevels, m_page.sliderOutputLevels);
    KisSignalsBlocker blocker2(m_page.spinBoxInputBlackPoint, m_page.spinBoxInputWhitePoint, m_page.spinBoxInputGamma,
                               m_page.spinBoxOutputBlackPoint, m_page.spinBoxOutputWhitePoint);

    if (m_page.buttonLightnessMode->isChecked()) {
        m_page.comboBoxChannel->hide();
        m_page.containerAllChannels->hide();
    } else {
        m_page.comboBoxChannel->show();
        m_page.containerAllChannels->show();
        const int index = m_page.comboBoxChannel->findData(m_activeChannel);
        m_page.comboBoxChannel->setCurrentIndex(index);
    }
    
    m_page.sliderInputLevels->reset(m_activeLevelsCurve->inputBlackPoint(),
                                    m_activeLevelsCurve->inputWhitePoint(),
                                    m_activeLevelsCurve->inputGamma());
    m_page.sliderOutputLevels->reset(m_activeLevelsCurve->outputBlackPoint(),
                                     m_activeLevelsCurve->outputWhitePoint());
    {
        QColor leftColor(Qt::black), rightColor(Qt::white);
        if (m_page.buttonAllChannelsMode->isChecked() &&
            (m_colorSpace->colorModelId() == RGBAColorModelID || m_colorSpace->colorModelId() == CMYKAColorModelID) &&
            m_virtualChannels[m_activeChannel].type() == VirtualChannelInfo::REAL &&
            !m_virtualChannels[m_activeChannel].isAlpha()) {
            const int channelIndex = m_virtualChannels[m_activeChannel].pixelIndex();
            if (m_colorSpace->colorModelId() == RGBAColorModelID) {
                leftColor = Qt::black;
                rightColor = channelIndex == 0 ? Qt::blue : (channelIndex == 1 ? Qt::green : Qt::red);
            } else {
                leftColor = Qt::white;
                rightColor = channelIndex == 0 ? Qt::cyan : (channelIndex == 1 ? Qt::magenta : (channelIndex == 2 ? Qt::yellow : Qt::black));
                rightColor = KoColor(rightColor, KoColorSpaceRegistry::instance()->rgb8())
                             .convertedTo(m_colorSpace,
                                          KoColorConversionTransformation::IntentSaturation,
                                          KoColorConversionTransformation::Empty)
                             .toQColor();
            }
        } else if (m_page.buttonAllChannelsMode->isChecked() &&
                   m_colorSpace->colorModelId() == CMYKAColorModelID &&
                   m_virtualChannels[m_activeChannel].type() == VirtualChannelInfo::ALL_COLORS) {
            leftColor = Qt::white;
            rightColor = Qt::black;
        }
        m_page.sliderInputLevels->setHandleColor(0, leftColor);
        m_page.sliderInputLevels->setHandleColor(1, KisPaintingTweaks::blendColors(leftColor, rightColor, 0.5));
        m_page.sliderInputLevels->setHandleColor(2, rightColor);
        m_page.sliderOutputLevels->setHandleColor(0, leftColor);
        m_page.sliderOutputLevels->setHandleColor(1, rightColor);
    }

    m_page.spinBoxInputBlackPoint->setRange(m_activeChannelMin, m_activeChannelMax);
    m_page.spinBoxInputWhitePoint->setRange(m_activeChannelMin, m_activeChannelMax);
    m_page.spinBoxOutputBlackPoint->setRange(m_activeChannelMin, m_activeChannelMax);
    m_page.spinBoxOutputWhitePoint->setRange(m_activeChannelMin, m_activeChannelMax);
    m_page.spinBoxInputBlackPoint->setValue(deNormalizeValue(m_activeLevelsCurve->inputBlackPoint(), m_activeChannelMin, m_activeChannelMax));
    m_page.spinBoxInputWhitePoint->setValue(deNormalizeValue(m_activeLevelsCurve->inputWhitePoint(), m_activeChannelMin, m_activeChannelMax));
    m_page.spinBoxInputGamma->setValue(m_activeLevelsCurve->inputGamma());
    m_page.spinBoxOutputBlackPoint->setValue(deNormalizeValue(m_activeLevelsCurve->outputBlackPoint(), m_activeChannelMin, m_activeChannelMax));
    m_page.spinBoxOutputWhitePoint->setValue(deNormalizeValue(m_activeLevelsCurve->outputWhitePoint(), m_activeChannelMin, m_activeChannelMax));

    if ((m_page.buttonLightnessMode->isChecked() ||
        m_virtualChannels[m_activeChannel].type() == VirtualChannelInfo::LIGHTNESS) &&
        m_lightnessHistogram) {

        m_page.buttonAutoLevels->setEnabled(true);

    } else if (m_virtualChannels[m_activeChannel].type() == VirtualChannelInfo::REAL &&
               m_channelsHistogram) {
        m_page.buttonAutoLevels->setEnabled(true);
    } else {
        m_page.buttonAutoLevels->setEnabled(false);
    }
}

void KisLevelsConfigWidget::updateHistograms()
{
    QVector<KisHistogram*> histograms;
    QVector<const KoColorSpace*> colorSpaces;
    QVector<QVector<int>> channels;

    // Init lightness histogram
    // This is used in the simple mode and if the lightness virtual channel is selected
    if (!m_lightnessHistogram) {
        KoHistogramProducer *lightnessHistogramProducer = new KoGenericLabHistogramProducer();
        m_lightnessHistogram.reset(new KisHistogram(m_dev, m_dev->exactBounds(), lightnessHistogramProducer, LINEAR));
        histograms.append(m_lightnessHistogram.data());
        colorSpaces.append(KoColorSpaceRegistry::instance()->lab16());
        channels.append(QVector<int>({0}));
    }
    // Init channels histogram
    if (!m_channelsHistogram) {
        const KoColorSpace *colorSpace = m_colorSpace;
        QList<QString> keys = KoHistogramProducerFactoryRegistry::instance()->keysCompatibleWith(colorSpace);
        if (keys.size() > 0) {
            KoHistogramProducerFactory *hpf = KoHistogramProducerFactoryRegistry::instance()->get(keys.at(0));
            m_channelsHistogram.reset(new KisHistogram(m_dev, m_dev->exactBounds(), hpf->generate(), LINEAR));
            histograms.append(m_channelsHistogram.data());
            colorSpaces.append(colorSpace);
            channels.append(QVector<int>());
        }
    }

    if (histograms.size() > 0) {
        m_page.widgetHistogram->setup(histograms, colorSpaces, channels);
        updateHistogramViewChannels();
        m_page.widgetHistogram->setDefaultColor(Qt::gray);
    }
}

void KisLevelsConfigWidget::updateHistogramViewChannels()
{
    m_page.widgetHistogram->clearChannels();

    if (m_page.buttonLightnessMode->isChecked() ||
        m_virtualChannels[m_activeChannel].type() == VirtualChannelInfo::LIGHTNESS) {
        if (m_lightnessHistogram) {
            m_page.widgetHistogram->setChannel(0, 0);
        }
    } else {
        const VirtualChannelInfo::Type channelType = m_virtualChannels[m_activeChannel].type();
        if (m_channelsHistogram &&
            (channelType == VirtualChannelInfo::ALL_COLORS || channelType == VirtualChannelInfo::REAL)) {

            if (channelType == VirtualChannelInfo::ALL_COLORS) {
                QVector<int> channels;
                for (const VirtualChannelInfo &channelInfo : m_virtualChannels) {
                    if (channelInfo.type() == VirtualChannelInfo::REAL && !channelInfo.isAlpha()) {
                        channels.append(channelInfo.pixelIndex());
                    }
                }
                m_page.widgetHistogram->setChannels(channels, 1);

            } else if (channelType == VirtualChannelInfo::REAL) {
                m_page.widgetHistogram->setChannel(m_virtualChannels[m_activeChannel].pixelIndex(), 1);
            }
        }
    }
}

void KisLevelsConfigWidget::setButtonsIcons()
{
    m_page.buttonLightnessMode->setIcon(KisIconUtils::loadIcon("color-adjustement-mode-lightness"));
    m_page.buttonAllChannelsMode->setIcon(KisIconUtils::loadIcon("color-adjustement-mode-channels"));
    m_page.buttonLinearHistogram->setIcon(KisIconUtils::loadIcon("histogram-linear"));
    m_page.buttonLogarithmicHistogram->setIcon(KisIconUtils::loadIcon("histogram-logarithmic"));
    m_page.buttonScaleHistogramToFit->setIcon(KisIconUtils::loadIcon("histogram-show-all"));
    m_page.buttonScaleHistogramToCutLongPeaks->setIcon(KisIconUtils::loadIcon("histogram-show-best"));
    m_page.buttonResetAll->setIcon(KisIconUtils::loadIcon("reload-preset"));
    m_page.buttonResetInputLevels->setIcon(KisIconUtils::loadIcon("reload-preset"));
    m_page.buttonResetOutputLevels->setIcon(KisIconUtils::loadIcon("reload-preset"));
    m_page.buttonResetAllChannels->setIcon(KisIconUtils::loadIcon("reload-preset"));
    m_page.buttonAutoLevels->setIcon(KisIconUtils::loadIcon("autolevels"));
    m_page.buttonAutoLevelsAllChannels->setIcon(KisIconUtils::loadIcon("autolevels"));
}

void KisLevelsConfigWidget::slot_buttonGroupMode_buttonToggled(QAbstractButton *button, bool checked)
{
    if (!checked) {
        return;
    }
    if (button == m_page.buttonLightnessMode) {
        m_activeLevelsCurve = &m_lightnessLevelsCurve;
        m_activeChannelMin = m_lightnessMinMaxRanges.first;
        m_activeChannelMax = m_lightnessMinMaxRanges.second;
    } else {
        m_activeLevelsCurve = &m_levelsCurves[m_activeChannel];
        m_activeChannelMin = m_virtualChannelsMinMaxRanges[m_activeChannel].first;
        m_activeChannelMax = m_virtualChannelsMinMaxRanges[m_activeChannel].second;
    }
    updateWidgets();
    updateHistogramViewChannels();
    emit sigConfigurationItemChanged();
}

void KisLevelsConfigWidget::slot_comboBoxChannel_activated(int index)
{
    const int virtualChannel = m_page.comboBoxChannel->itemData(index).toInt();
    setActiveChannel(virtualChannel);
    updateHistogramViewChannels();
}

void KisLevelsConfigWidget::slot_buttonGroupHistogramMode_buttonToggled(QAbstractButton *button, bool checked)
{
    if (!checked) {
        return;
    }
    m_page.widgetHistogram->setLogarithmic(button == m_page.buttonLogarithmicHistogram);
}

void KisLevelsConfigWidget::slot_spinBoxInputBlackPoint_valueChanged(int value)
{
    if (value >= m_page.spinBoxInputWhitePoint->value()) {
        m_page.spinBoxInputBlackPoint->setValue(m_page.spinBoxInputWhitePoint->value() - 1);
    }

    KisSignalsBlocker blocker(m_page.sliderInputLevels);
    const qreal normalizedValue = normalizeValue(value, m_activeChannelMin, m_activeChannelMax);
    m_activeLevelsCurve->setInputBlackPoint(normalizedValue);
    m_page.sliderInputLevels->setBlackPoint(normalizedValue);
    emit sigConfigurationItemChanged();
}

void KisLevelsConfigWidget::slot_spinBoxInputWhitePoint_valueChanged(int value)
{
    if (value <= m_page.spinBoxInputBlackPoint->value()) {
        m_page.spinBoxInputWhitePoint->setValue(m_page.spinBoxInputBlackPoint->value() + 1);
    }

    KisSignalsBlocker blocker(m_page.sliderInputLevels);
    const qreal normalizedValue = normalizeValue(value, m_activeChannelMin, m_activeChannelMax);
    m_activeLevelsCurve->setInputWhitePoint(normalizedValue);
    m_page.sliderInputLevels->setWhitePoint(normalizedValue);
    emit sigConfigurationItemChanged();
}

void KisLevelsConfigWidget::slot_spinBoxInputGamma_valueChanged(qreal value)
{
    KisSignalsBlocker blocker(m_page.sliderInputLevels);
    m_activeLevelsCurve->setInputGamma(value);
    m_page.sliderInputLevels->setGamma(value);
    emit sigConfigurationItemChanged();
}

void KisLevelsConfigWidget::slot_spinBoxOutputBlackPoint_valueChanged(int value)
{
    KisSignalsBlocker blocker(m_page.sliderOutputLevels);
    const qreal normalizedValue = normalizeValue(value, m_activeChannelMin, m_activeChannelMax);
    m_activeLevelsCurve->setOutputBlackPoint(normalizedValue);
    m_page.sliderOutputLevels->setBlackPoint(normalizedValue);
    emit sigConfigurationItemChanged();
}

void KisLevelsConfigWidget::slot_spinBoxOutputWhitePoint_valueChanged(int value)
{
    KisSignalsBlocker blocker(m_page.sliderOutputLevels);
    const qreal normalizedValue = normalizeValue(value, m_activeChannelMin, m_activeChannelMax);
    m_activeLevelsCurve->setOutputWhitePoint(normalizedValue);
    m_page.sliderOutputLevels->setWhitePoint(normalizedValue);
    emit sigConfigurationItemChanged();
}

void KisLevelsConfigWidget::slot_sliderInputLevels_blackPointChanged(qreal value)
{
    KisSignalsBlocker blocker(m_page.spinBoxInputBlackPoint);
    m_activeLevelsCurve->setInputBlackPoint(value);
    m_page.spinBoxInputBlackPoint->setValue(deNormalizeValue(value, m_activeChannelMin, m_activeChannelMax));
    emit sigConfigurationItemChanged();
}

void KisLevelsConfigWidget::slot_sliderInputLevels_whitePointChanged(qreal value)
{
    KisSignalsBlocker blocker(m_page.spinBoxInputWhitePoint);
    m_activeLevelsCurve->setInputWhitePoint(value);
    m_page.spinBoxInputWhitePoint->setValue(deNormalizeValue(value, m_activeChannelMin, m_activeChannelMax));
    emit sigConfigurationItemChanged();
}

void KisLevelsConfigWidget::slot_sliderInputLevels_gammaChanged(qreal value)
{
    KisSignalsBlocker blocker(m_page.spinBoxInputGamma);
    m_activeLevelsCurve->setInputGamma(value);
    m_page.spinBoxInputGamma->setValue(value);
    emit sigConfigurationItemChanged();
}

void KisLevelsConfigWidget::slot_sliderOutputLevels_blackPointChanged(qreal value)
{
    KisSignalsBlocker blocker(m_page.spinBoxOutputBlackPoint);
    m_activeLevelsCurve->setOutputBlackPoint(value);
    m_page.spinBoxOutputBlackPoint->setValue(deNormalizeValue(value, m_activeChannelMin, m_activeChannelMax));
    emit sigConfigurationItemChanged();
}

void KisLevelsConfigWidget::slot_sliderOutputLevels_whitePointChanged(qreal value)
{
    KisSignalsBlocker blocker(m_page.spinBoxOutputWhitePoint);
    m_activeLevelsCurve->setOutputWhitePoint(value);
    m_page.spinBoxOutputWhitePoint->setValue(deNormalizeValue(value, m_activeChannelMin, m_activeChannelMax));
    emit sigConfigurationItemChanged();
}

void KisLevelsConfigWidget::slot_buttonAutoLevels_clicked()
{
    KisLevelsCurve previousLevelsCurve = *m_activeLevelsCurve;

    KoDialog *autolevelsDialog = new KoDialog(this);

    m_autoLevelsWidget = new KisAutoLevelsWidget(autolevelsDialog);
    // Lock contrast method
    m_autoLevelsWidget->setShadowsAndHighlightsAdjustementMethod(
        KisAutoLevels::ShadowsAndHighlightsAdjustmentMethod_MonochromaticContrast
    );
    m_autoLevelsWidget->lockShadowsAndHighlightsAdjustementMethod();
    // Set some defaultparameters based on the selected channel. These were
    // selected empirically, there is no strong reason why they should be like this
    if (m_page.buttonLightnessMode->isChecked() ||
        m_virtualChannels[m_activeChannel].type() == VirtualChannelInfo::LIGHTNESS ||
        (m_colorSpace->colorModelId() == LABAColorModelID && m_virtualChannels[m_activeChannel].pixelIndex() == 0) ||
        (m_colorSpace->colorModelId() == CMYKAColorModelID && m_virtualChannels[m_activeChannel].pixelIndex() == 3) ||
        (m_colorSpace->colorModelId() == GrayAColorModelID && m_virtualChannels[m_activeChannel].pixelIndex() == 0)) {
        m_autoLevelsWidget->setMaximumInputBlackAndWhiteOffset(10.0);
        m_autoLevelsWidget->setMidtonesAdjustmentMethod(KisAutoLevels::MidtonesAdjustmentMethod_UseMedian);
        m_autoLevelsWidget->setMidtonesAdjustmentAmount(25.0);
    } else if (m_colorSpace->colorModelId() == CMYKAColorModelID) {
        if (m_virtualChannels[m_activeChannel].pixelIndex() == 0 ||
            m_virtualChannels[m_activeChannel].pixelIndex() == 1 ||
            m_virtualChannels[m_activeChannel].pixelIndex() == 2) {
            m_autoLevelsWidget->setMaximumInputBlackAndWhiteOffset(25.0);
        }
    }
    // Set the output colors
    const KoColorSpace *outputColorsColorSpace = KoColorSpaceRegistry::instance()->colorSpace(GrayAColorModelID.id(),
                                                                                              m_colorSpace->colorDepthId().id());
    m_autoLevelsWidget->setShadowsColor(KoColor(Qt::black, outputColorsColorSpace));
    m_autoLevelsWidget->setHighlightsColor(KoColor(Qt::white, outputColorsColorSpace));
    // Ensure that the midtone color has 50% value
    QVector<float> normalizedMidtonesColor{0.5, 1.0};
    KoColor midtonesColor(outputColorsColorSpace);
    outputColorsColorSpace->fromNormalisedChannelsValue(midtonesColor.data(), normalizedMidtonesColor);
    m_autoLevelsWidget->setMidtonesColor(midtonesColor);

    connect(m_autoLevelsWidget, SIGNAL(parametersChanged()), SLOT(slot_autoLevelsWidget_parametersChanged()));
    slot_autoLevelsWidget_parametersChanged();

    autolevelsDialog->setCaption(i18nc("@title:window", "Auto Levels"));
    autolevelsDialog->setMainWidget(m_autoLevelsWidget);
    autolevelsDialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(autolevelsDialog, &QDialog::rejected,
        [this, previousLevelsCurve]()
        {
            *m_activeLevelsCurve = previousLevelsCurve;
            updateWidgets();
            emit sigConfigurationItemChanged();
        }
    );
    connect(autolevelsDialog, &QDialog::finished, [this](){ setEnabled(true); });

    setEnabled(false);
    autolevelsDialog->setEnabled(true);

    autolevelsDialog->show();
    autolevelsDialog->raise();
    autolevelsDialog->activateWindow();
}

void KisLevelsConfigWidget::slot_buttonAutoLevelsAllChannels_clicked()
{
    // We can not use the copy constructor here since it makes use of implicit
    // sharing. We mantain a pointer to the active levels info in m_levelsCurves.
    // So, if we change an element of m_levelsCurves that would create a new
    // vector and the pointer will be invalidated
    QVector<KisLevelsCurve> previousLevelsCurves;
    for (const KisLevelsCurve &levelsCurve : m_levelsCurves) {
        previousLevelsCurves.append(levelsCurve);
    }

    KoDialog *autolevelsDialog = new KoDialog(this);

    m_autoLevelsWidget = new KisAutoLevelsWidget(autolevelsDialog);
    m_autoLevelsWidget->setShadowsAndHighlightsAdjustementMethod(
        KisAutoLevels::ShadowsAndHighlightsAdjustmentMethod_MonochromaticContrast
    );

    m_autoLevelsWidget->setShadowsColor(KoColor(Qt::black, m_colorSpace));
    m_autoLevelsWidget->setHighlightsColor(KoColor(Qt::white, m_colorSpace));
    m_autoLevelsWidget->setMidtonesColor(KoColor(Qt::gray, m_colorSpace));
    // Ensure that the midtone color has 50% value
    QVector<float> normalizedMidtonesColor{0.5, 0.5, 0.5, 1.0};
    KoColor midtonesColor(m_colorSpace);
    m_colorSpace->fromNormalisedChannelsValue(midtonesColor.data(), normalizedMidtonesColor);
    m_autoLevelsWidget->setMidtonesColor(midtonesColor);

    connect(m_autoLevelsWidget, SIGNAL(parametersChanged()), SLOT(slot_autoLevelsWidgetAllChannels_parametersChanged()));
    slot_autoLevelsWidgetAllChannels_parametersChanged();

    autolevelsDialog->setCaption(i18nc("@title:window", "Auto Levels"));
    autolevelsDialog->setMainWidget(m_autoLevelsWidget);
    autolevelsDialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(autolevelsDialog, &QDialog::rejected,
        [this, previousLevelsCurves]()
        {
            // We mantain a pointer to the active levels info in m_levelsCurves
            // so we use this loop instead of the asignment operator to avoid
            // invalidation of the pointer
            for (int i = 0; i < m_levelsCurves.size(); ++i) {
                m_levelsCurves[i] = previousLevelsCurves[i];
            }
            updateWidgets();
            emit sigConfigurationItemChanged();
        }
    );
    connect(autolevelsDialog, &QDialog::finished, [this](){ setEnabled(true); });

    setEnabled(false);
    autolevelsDialog->setEnabled(true);

    autolevelsDialog->show();
    autolevelsDialog->raise();
    autolevelsDialog->activateWindow();
}

bool KisLevelsConfigWidget::event(QEvent *e)
{
    if (e->type() == QEvent::PaletteChange) {
        KisIconUtils::updateIcon(m_page.buttonLightnessMode);
        KisIconUtils::updateIcon(m_page.buttonAllChannelsMode);
        KisIconUtils::updateIcon(m_page.buttonLinearHistogram);
        KisIconUtils::updateIcon(m_page.buttonLogarithmicHistogram);
        KisIconUtils::updateIcon(m_page.buttonScaleHistogramToFit);
        KisIconUtils::updateIcon(m_page.buttonScaleHistogramToCutLongPeaks);
        KisIconUtils::updateIcon(m_page.buttonResetAll);
        KisIconUtils::updateIcon(m_page.buttonResetInputLevels);
        KisIconUtils::updateIcon(m_page.buttonResetOutputLevels);
        KisIconUtils::updateIcon(m_page.buttonResetAllChannels);
        KisIconUtils::updateIcon(m_page.buttonAutoLevels);
        KisIconUtils::updateIcon(m_page.buttonAutoLevelsAllChannels);
    } else if (e->type() == QEvent::StyleChange) {
        m_page.spacer01->changeSize(0, m_page.comboBoxChannel->sizeHint().height(),
                                    QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
    return QWidget::event(e);
}

void KisLevelsConfigWidget::slot_autoLevelsWidget_parametersChanged()
{
    KisHistogram *histogram;
    int channel;
    bool isCMYK = false;

    if (m_page.buttonLightnessMode->isChecked() ||
        m_virtualChannels[m_activeChannel].type() == VirtualChannelInfo::LIGHTNESS) {
        histogram = m_lightnessHistogram.data();
        channel = 0;
    } else {
        histogram = m_channelsHistogram.data();
        channel = m_virtualChannels[m_activeChannel].pixelIndex();
        isCMYK = m_colorSpace->colorModelId() == CMYKAColorModelID;
    }

    // Output colors
    const KoColorSpace *outputColorsColorSpace = KoColorSpaceRegistry::instance()->colorSpace(GrayAColorModelID.id(),
                                                                                              m_colorSpace->colorDepthId().id());
    const KoColor shadowsColor = m_autoLevelsWidget->outputShadowsColor().convertedTo(outputColorsColorSpace);
    const KoColor highlightsColor = m_autoLevelsWidget->outputHighlightsColor().convertedTo(outputColorsColorSpace);
    const KoColor midtonesColor = m_autoLevelsWidget->outputMidtonesColor().convertedTo(outputColorsColorSpace);
    QVector<float> shadowsNormalizedColor(outputColorsColorSpace->channelCount());
    QVector<float> highlightsNormalizedColor(outputColorsColorSpace->channelCount());
    QVector<float> midtonesNormalizedColor(outputColorsColorSpace->channelCount());
    outputColorsColorSpace->normalisedChannelsValue(shadowsColor.data(), shadowsNormalizedColor);
    outputColorsColorSpace->normalisedChannelsValue(highlightsColor.data(), highlightsNormalizedColor);
    outputColorsColorSpace->normalisedChannelsValue(midtonesColor.data(), midtonesNormalizedColor);
    const qreal shadowsOutput = isCMYK ? 1.0 - highlightsNormalizedColor[0] : shadowsNormalizedColor[0];
    const qreal highlightsOutput = isCMYK ? 1.0 - shadowsNormalizedColor[0] : highlightsNormalizedColor[0];
    const qreal midtonesOutput = midtonesNormalizedColor[0];

    QVector<KisAutoLevels::ChannelHistogram> channelsHistograms{{histogram, channel}};

    *m_activeLevelsCurve =
        KisAutoLevels::adjustMonochromaticContrast(
            {histogram, channel},
            channelsHistograms,
            m_autoLevelsWidget->shadowsClipping() / 100.0,
            m_autoLevelsWidget->highlightsClipping() / 100.0,
            m_autoLevelsWidget->maximumInputBlackAndWhiteOffset() / 100.0,
            m_autoLevelsWidget->midtonesAdjustmentMethod(),
            m_autoLevelsWidget->midtonesAdjustmentAmount() / 100.0,
            {shadowsOutput},
            {highlightsOutput},
            {midtonesOutput}
        )[0];
        
    updateWidgets();
    emit sigConfigurationItemChanged();
}

void KisLevelsConfigWidget::slot_autoLevelsWidgetAllChannels_parametersChanged()
{
    // histograms
    QVector<KisAutoLevels::ChannelHistogram> channelsHistograms;
    for (const VirtualChannelInfo &virtualChannelInfo : m_virtualChannels) {
        if (virtualChannelInfo.type() == VirtualChannelInfo::REAL && !virtualChannelInfo.isAlpha()) {
            channelsHistograms.append({m_channelsHistogram.data(), virtualChannelInfo.pixelIndex()});
        }
    }

    // Output colors
    QVector<qreal> shadowsOutput, highlightsOutput, midtonesOutput;
    const KoColor shadowsColor = m_autoLevelsWidget->outputShadowsColor().convertedTo(m_colorSpace);
    const KoColor highlightsColor = m_autoLevelsWidget->outputHighlightsColor().convertedTo(m_colorSpace);
    const KoColor midtonesColor = m_autoLevelsWidget->outputMidtonesColor().convertedTo(m_colorSpace);
    QVector<float> shadowsNormalizedColor(m_colorSpace->channelCount());
    QVector<float> highlightsNormalizedColor(m_colorSpace->channelCount());
    QVector<float> midtonesNormalizedColor(m_colorSpace->channelCount());
    m_colorSpace->normalisedChannelsValue(shadowsColor.data(), shadowsNormalizedColor);
    m_colorSpace->normalisedChannelsValue(highlightsColor.data(), highlightsNormalizedColor);
    m_colorSpace->normalisedChannelsValue(midtonesColor.data(), midtonesNormalizedColor);
    for (const KisAutoLevels::ChannelHistogram &channelHistogram : channelsHistograms) {
        shadowsOutput.append(shadowsNormalizedColor[channelHistogram.channel]);
        highlightsOutput.append(highlightsNormalizedColor[channelHistogram.channel]);
        midtonesOutput.append(midtonesNormalizedColor[channelHistogram.channel]);
    }

    // get levels parameters
    QVector<KisLevelsCurve> levelsCurves;
    if (m_autoLevelsWidget->shadowsAndHighlightsAdjustementMethod() == KisAutoLevels::ShadowsAndHighlightsAdjustmentMethod_MonochromaticContrast) {
        levelsCurves =
            KisAutoLevels::adjustPerChannelContrast(
                channelsHistograms,
                m_autoLevelsWidget->shadowsClipping() / 100.0,
                m_autoLevelsWidget->highlightsClipping() / 100.0,
                m_autoLevelsWidget->maximumInputBlackAndWhiteOffset() / 100.0,
                m_autoLevelsWidget->midtonesAdjustmentMethod(),
                m_autoLevelsWidget->midtonesAdjustmentAmount() / 100.0,
                shadowsOutput,
                highlightsOutput,
                midtonesOutput
            );
        // get min black point and max white point
        qreal minBlackPoint = 1.0;
        qreal maxWhitePoint = 0.0;
        for (const KisLevelsCurve &levelsCurve : levelsCurves) {
            minBlackPoint = qMin(levelsCurve.inputBlackPoint(), minBlackPoint);
            maxWhitePoint = qMax(levelsCurve.inputWhitePoint(), maxWhitePoint);
        }
        for (KisLevelsCurve &levelsCurve : levelsCurves) {
            levelsCurve.setInputBlackPoint(minBlackPoint);
            levelsCurve.setInputWhitePoint(maxWhitePoint);
        }
    } else {
        levelsCurves =
            KisAutoLevels::adjustPerChannelContrast(
                channelsHistograms,
                m_autoLevelsWidget->shadowsClipping() / 100.0,
                m_autoLevelsWidget->highlightsClipping() / 100.0,
                m_autoLevelsWidget->maximumInputBlackAndWhiteOffset() / 100.0,
                m_autoLevelsWidget->midtonesAdjustmentMethod(),
                m_autoLevelsWidget->midtonesAdjustmentAmount() / 100.0,
                shadowsOutput,
                highlightsOutput,
                midtonesOutput
            );
    }

    // Set parameters
    for (int i = 0, j = 0; i < m_virtualChannels.size(); ++i) {
        if (m_virtualChannels[i].type() == VirtualChannelInfo::REAL && !m_virtualChannels[i].isAlpha()) {
            m_levelsCurves[i] = levelsCurves[j];
            ++j;
        }
    }
    updateWidgets();
    emit sigConfigurationItemChanged();
}
