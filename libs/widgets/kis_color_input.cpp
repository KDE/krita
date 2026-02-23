/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *  SPDX-FileCopyrightText: 2015 Moritz Molch <kde@moritzmolch.de>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_color_input.h"

#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>
#endif

#include <cmath>

#include <kis_debug.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

#include <klocalizedstring.h>

#include <KoChannelInfo.h>
#include <KoColor.h>
#include <KoColorSlider.h>
#include <KoColorSpace.h>
#include <KisHsvColorSlider.h>
#include <KoColorConversions.h>
#include <KisSpinBoxI18nHelper.h>

#include "kis_double_parse_spin_box.h"
#include "kis_int_parse_spin_box.h"
#include "kis_signals_blocker.h"

KisColorInput::KisColorInput(QWidget* parent, const KoChannelInfo* channelInfo, KoColor* color, KoColorDisplayRendererInterface *displayRenderer, bool usePercentage) :
    QWidget(parent), m_channelInfo(channelInfo), m_color(color), m_displayRenderer(displayRenderer),
    m_usePercentage(usePercentage)
{
}

void KisColorInput::init()
{
    QHBoxLayout* m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0,0,0,0);
    m_layout->setSpacing(1);

    QLabel* m_label = new QLabel(i18n("%1:", m_channelInfo->name()), this);
    m_layout->addWidget(m_label);

    m_colorSlider = new KoColorSlider(Qt::Horizontal, this, m_displayRenderer);
    m_colorSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_layout->addWidget(m_colorSlider);

    QWidget* m_input = createInput();
    m_layout->addWidget(m_input);
}

KisIntegerColorInput::KisIntegerColorInput(QWidget* parent, const KoChannelInfo* channelInfo, KoColor* color, KoColorDisplayRendererInterface *displayRenderer, bool usePercentage) :
    KisColorInput(parent, channelInfo, color, displayRenderer, usePercentage)
{
    init();
}

void KisIntegerColorInput::setValue(int v)
{
    quint8* data = m_color->data() + m_channelInfo->pos();
    switch (m_channelInfo->channelValueType()) {
    case KoChannelInfo::UINT8:
        *(reinterpret_cast<quint8*>(data)) = v;
        break;
    case KoChannelInfo::UINT16:
        *(reinterpret_cast<quint16*>(data)) = v;
        break;
    case KoChannelInfo::UINT32:
        *(reinterpret_cast<quint32*>(data)) = v;
        break;
    default:
        Q_ASSERT(false);
    }
    emit(updated());
}

void KisIntegerColorInput::update()
{
    KoColor min = *m_color;
    KoColor max = *m_color;
    quint8* data = m_color->data() + m_channelInfo->pos();
    quint8* dataMin = min.data() + m_channelInfo->pos();
    quint8* dataMax = max.data() + m_channelInfo->pos();
    m_intNumInput->blockSignals(true);
    m_colorSlider->blockSignals(true);
    switch (m_channelInfo->channelValueType()) {
    case KoChannelInfo::UINT8: {
        quint8 value = *(reinterpret_cast<quint8*>(data));
        m_intNumInput->setValue(m_usePercentage ? round(value * 100.0 / 0xFF) : value);
        m_colorSlider->setValue(value);
        *(reinterpret_cast<quint8*>(dataMin)) = 0x0;
        *(reinterpret_cast<quint8*>(dataMax)) = 0xFF;
        break;
        }
    case KoChannelInfo::UINT16: {
        quint16 value = *(reinterpret_cast<quint16*>(data));
        m_intNumInput->setValue(m_usePercentage ? round(value * 100.0 / 0xFFFF) : value);
        m_colorSlider->setValue(value);
        *(reinterpret_cast<quint16*>(dataMin)) = 0x0;
        *(reinterpret_cast<quint16*>(dataMax)) = 0xFFFF;
        break;
    }
    case KoChannelInfo::UINT32: {
        quint32 value = *(reinterpret_cast<quint32*>(data));
        m_intNumInput->setValue(m_usePercentage ? round(value * 100.0 / 0xFFFF'FFFF) : value);
        m_colorSlider->setValue(value);
        *(reinterpret_cast<quint32*>(dataMin)) = 0x0;
        *(reinterpret_cast<quint32*>(dataMax)) = 0xFFFF'FFFF;
        break;
    }
    default:
        Q_ASSERT(false);
    }
    m_colorSlider->setColors(min, max);
    m_intNumInput->blockSignals(false);
    m_colorSlider->blockSignals(false);
}

QWidget* KisIntegerColorInput::createInput()
{
    m_intNumInput = new KisIntParseSpinBox(this);
    m_intNumInput->setMinimum(0);
    m_colorSlider->setMinimum(0);

    if (m_usePercentage) {
        KisSpinBoxI18nHelper::setText(m_intNumInput, i18nc("{n} is the number value, % is the percent sign", "{n}%"));
    }

    updateMaximums();

    connect(m_colorSlider, SIGNAL(valueChanged(int)), this, SLOT(onColorSliderChanged(int)));
    connect(m_intNumInput, SIGNAL(valueChanged(int)), this, SLOT(onNumInputChanged(int)));
    return m_intNumInput;
}

void KisIntegerColorInput::updateMaximums()
{
    m_intNumInput->clearFocus(); // make sure focus doesn't interfere with updating

    Q_ASSERT(m_channelInfo->channelValueType() == KoChannelInfo::UINT8 ||
             m_channelInfo->channelValueType() == KoChannelInfo::UINT16 ||
             m_channelInfo->channelValueType() == KoChannelInfo::UINT32);

    m_intNumInput->blockSignals(true); // prevent clamping from triggering a value update
    m_intNumInput->setMaximum(m_usePercentage ? 100 : (1 << 8 * m_channelInfo->size()) - 1);
    m_colorSlider->setMaximum((1 << 8 * m_channelInfo->size()) - 1);
    m_intNumInput->blockSignals(false);
}

void KisIntegerColorInput::setPercentageWise(bool val)
{
    m_usePercentage = val;

    m_intNumInput->clearFocus(); // make sure focus doesn't interfere with updating

    if (m_usePercentage) {
        KisSpinBoxI18nHelper::setText(m_intNumInput, i18nc("{n} is the number value, % is the percent sign", "{n}%"));
    } else {
        m_intNumInput->setPrefix("");
        m_intNumInput->setSuffix("");
    }

    updateMaximums();
}

void KisIntegerColorInput::onColorSliderChanged(int val)
{
    m_intNumInput->blockSignals(true);

    Q_ASSERT(m_channelInfo->channelValueType() == KoChannelInfo::UINT8 ||
             m_channelInfo->channelValueType() == KoChannelInfo::UINT16 ||
             m_channelInfo->channelValueType() == KoChannelInfo::UINT32);

    m_intNumInput->setValue(m_usePercentage ? round(val * 100.0 / ((1 << 8 * m_channelInfo->size()) - 1))
                                            : val);

    m_intNumInput->blockSignals(false);
    setValue(val);
}

void KisIntegerColorInput::onNumInputChanged(int val)
{
    m_colorSlider->blockSignals(true);

    Q_ASSERT(m_channelInfo->channelValueType() == KoChannelInfo::UINT8 ||
             m_channelInfo->channelValueType() == KoChannelInfo::UINT16 ||
             m_channelInfo->channelValueType() == KoChannelInfo::UINT32);

    m_colorSlider->setValue(m_usePercentage ? val / 100.0 * ((1 << 8 * m_channelInfo->size()) - 1)
                                            : val);

    m_colorSlider->blockSignals(false);
    setValue(m_usePercentage ? val / 100.0 * ((1 << 8 * m_channelInfo->size()) - 1)
                             : val);
}

KisFloatColorInput::KisFloatColorInput(QWidget* parent, const KoChannelInfo* channelInfo, KoColor* color, KoColorDisplayRendererInterface *displayRenderer, bool usePercentage) :
    KisColorInput(parent, channelInfo, color, displayRenderer, usePercentage)
{
    init();
}

void KisFloatColorInput::setValue(double v)
{
    quint8* data = m_color->data() + m_channelInfo->pos();
    switch (m_channelInfo->channelValueType()) {
#ifdef HAVE_OPENEXR
    case KoChannelInfo::FLOAT16:
        *(reinterpret_cast<half*>(data)) = v;
        break;
#endif
    case KoChannelInfo::FLOAT32:
        *(reinterpret_cast<float*>(data)) = v;
        break;
    default:
        Q_ASSERT(false);
    }
    emit(updated());
}

QWidget* KisFloatColorInput::createInput()
{
    m_dblNumInput = new KisDoubleParseSpinBox(this);
    m_dblNumInput->setMinimum(0);
    m_dblNumInput->setMaximum(1.0);
    m_dblNumInput->setSingleStep(0.01);
    connect(m_colorSlider, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
    connect(m_dblNumInput, SIGNAL(valueChanged(double)), this, SLOT(setValue(double)));
    m_dblNumInput->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    m_dblNumInput->setMinimumWidth(60);
    m_dblNumInput->setMaximumWidth(60);
    
    quint8* data = m_color->data() + m_channelInfo->pos();
    qreal value = 1.0;

    switch (m_channelInfo->channelValueType()) {
#ifdef HAVE_OPENEXR
    case KoChannelInfo::FLOAT16:
        value = *(reinterpret_cast<half*>(data));
        break;
#endif
    case KoChannelInfo::FLOAT32:
        value = *(reinterpret_cast<float*>(data));
        break;
    default:
        Q_ASSERT(false);
    }
    m_dblNumInput->setValue(value);

    return m_dblNumInput;
}

void KisFloatColorInput::sliderChanged(int i)
{
    const qreal floatRange = m_maxValue - m_minValue;
    m_dblNumInput->setValue(m_minValue + (i / 255.0) * floatRange);
}

void KisFloatColorInput::update()
{
    KoColor min = *m_color;
    KoColor max = *m_color;
    quint8* data = m_color->data() + m_channelInfo->pos();
    quint8* dataMin = min.data() + m_channelInfo->pos();
    quint8* dataMax = max.data() + m_channelInfo->pos();

    qreal value = 1.0;
    m_minValue = m_displayRenderer->minVisibleFloatValue(m_channelInfo);
    m_maxValue = m_displayRenderer->maxVisibleFloatValue(m_channelInfo);
    m_dblNumInput->blockSignals(true);
    m_colorSlider->blockSignals(true);

    switch (m_channelInfo->channelValueType()) {
#ifdef HAVE_OPENEXR
    case KoChannelInfo::FLOAT16:
        value = *(reinterpret_cast<half*>(data));
        m_minValue = qMin(value, m_minValue);
        m_maxValue = qMax(value, m_maxValue);
        *(reinterpret_cast<half*>(dataMin)) = m_minValue;
        *(reinterpret_cast<half*>(dataMax)) = m_maxValue;
        break;
#endif
    case KoChannelInfo::FLOAT32:
        value = *(reinterpret_cast<float*>(data));
        m_minValue = qMin(value, m_minValue);
        m_maxValue = qMax(value, m_maxValue);
        *(reinterpret_cast<float*>(dataMin)) = m_minValue;
        *(reinterpret_cast<float*>(dataMax)) = m_maxValue;
        break;
    default:
        Q_ASSERT(false);
    }

    if (m_minValue != m_dblNumInput->minimum()) {
        m_dblNumInput->setMinimum(m_minValue);
    }
    if (m_maxValue != m_dblNumInput->maximum()) {
        m_dblNumInput->setMaximum(m_maxValue);
    }

    // ensure at least 3 significant digits are always shown
    int newPrecision = 2 + qMax(qreal(0.0), std::ceil(-std::log10(m_maxValue)));
    if (newPrecision != m_dblNumInput->decimals()) {
        m_dblNumInput->setDecimals(newPrecision);
        m_dblNumInput->updateGeometry();
    }
    m_dblNumInput->setValue(value);

    m_colorSlider->setColors(min, max);

    const qreal floatRange = m_maxValue - m_minValue;
    m_colorSlider->setValue((value - m_minValue) / floatRange * 255);
    m_dblNumInput->blockSignals(false);
    m_colorSlider->blockSignals(false);
}

KisHexColorInput::KisHexColorInput(QWidget* parent, KoColor* color, KoColorDisplayRendererInterface *displayRenderer, bool usePercentage, bool usePreview) :
    KisColorInput(parent, 0, color, displayRenderer, usePercentage)
{
    QHBoxLayout* m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0,0,0,0);
    m_layout->setSpacing(1);

    QLabel* m_label = new QLabel(i18n("Color name:"), this);
    m_label->setMinimumWidth(50);
    m_layout->addWidget(m_label);

    QWidget* m_input = createInput();
    m_input->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    if(usePreview) {
        m_colorPreview = new QLabel("");
        m_colorPreview->setMinimumWidth(30);
        m_layout->addWidget(m_colorPreview);
    }

    m_layout->addWidget(m_input);
}

void KisHexColorInput::setValue()
{
    QString valueString = m_hexInput->text();
    valueString.remove(QChar('#'));

    QList<KoChannelInfo*> channels = m_color->colorSpace()->channels();
    channels = KoChannelInfo::displayOrderSorted(channels);
    Q_FOREACH (KoChannelInfo* channel, channels) {
        if (channel->channelType() == KoChannelInfo::COLOR) {
            Q_ASSERT(channel->channelValueType() == KoChannelInfo::UINT8);
            quint8* data = m_color->data() + channel->pos();

            int value = valueString.left(2).toInt(0, 16);
            *(reinterpret_cast<quint8*>(data)) = value;
            valueString.remove(0, 2);
        }
    }
    emit(updated());
}

void KisHexColorInput::update()
{
    QString hexString("#");

    QList<KoChannelInfo*> channels = m_color->colorSpace()->channels();
    channels = KoChannelInfo::displayOrderSorted(channels);
    Q_FOREACH (KoChannelInfo* channel, channels) {
        if (channel->channelType() == KoChannelInfo::COLOR) {
            Q_ASSERT(channel->channelValueType() == KoChannelInfo::UINT8);
            quint8* data = m_color->data() + channel->pos();
            hexString.append(QString("%1").arg(*(reinterpret_cast<quint8*>(data)), 2, 16, QChar('0')));
        }
    }
    m_hexInput->setText(hexString);
    if( m_colorPreview) {
        m_colorPreview->setStyleSheet(QString("background-color: %1").arg(m_displayRenderer->toQColor(*m_color).name()));
    }
}

QWidget* KisHexColorInput::createInput()
{
    m_hexInput = new QLineEdit(this);
    m_hexInput->setAlignment(Qt::AlignRight);

    int digits = 2*m_color->colorSpace()->colorChannelCount();
    QString pattern = QString("#?[a-fA-F0-9]{%1,%2}").arg(digits).arg(digits);
    m_hexInput->setValidator(new QRegularExpressionValidator(QRegularExpression(pattern), this));
    connect(m_hexInput, SIGNAL(editingFinished()), this, SLOT(setValue()));
    return m_hexInput;
}


KisHsvColorInput::KisHsvColorInput(QWidget *parent, KoColor *color)
    : QWidget(parent)
    , m_color(color)
    , m_hSlider(nullptr)
    , m_sSlider(nullptr)
    , m_xSlider(nullptr)
    , m_hInput(nullptr)
    , m_sInput(nullptr)
    , m_xInput(nullptr)
    , m_h(0)
    , m_s(0)
    , m_x(0)
    , m_mixMode(KisHsvColorSlider::MIX_MODE::HSV)
{

    const QStringList labelNames = QStringList({
        i18nc("@label:slider Abbreviation for 'Hue'", "H:"),
        i18nc("@label:slider Abbreviation for 'Saturation'", "S:"),
        QString(/* x will get initialized later in setMixMode */) });
    qreal maxValues[3] = { 360, 100, 100 };

    QGridLayout *slidersLayout = new QGridLayout(this);
    slidersLayout->setContentsMargins(0,0,0,0);
    slidersLayout->setHorizontalSpacing(1); // less space around the sliders

    for (int i = 0; i < 3; i++) {
        // Label
        QLabel *label = new QLabel(labelNames[i], this);
        slidersLayout->addWidget(label, i, 0);

        // Slider itself
        KisHsvColorSlider *slider = new KisHsvColorSlider(Qt::Horizontal, this);
        slider->setMixMode(m_mixMode);
        slider->setMinimum(0);
        slider->setMaximum(maxValues[i]);
        slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        slidersLayout->addWidget(slider, i, 1);

        // Input box
        KisDoubleParseSpinBox *input = new KisDoubleParseSpinBox(this);
        input->setMinimum(0);
        input->setMaximum(maxValues[i]);

        input->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        input->setMinimumWidth(60);
        input->setMaximumWidth(60);

        slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        slidersLayout->addWidget(input, i, 2);

        switch (i) {
        case 0:
            m_hSlider = slider;
            m_hInput = input;
        case 1:
            m_sSlider = slider;
            m_sInput = input;
        case 2:
            m_xLabel = label; // Save the HSX label so we can update it
            m_xSlider = slider;
            m_xInput = input;
        }
    }

    // Connect slots
    connect(m_hSlider, SIGNAL(valueChanged(int)), this, SLOT(hueSliderChanged(int)));
    connect(m_hInput, SIGNAL(valueChanged(double)), this, SLOT(setHue(double)));
    connect(m_sSlider, SIGNAL(valueChanged(int)), this, SLOT(saturationSliderChanged(int)));
    connect(m_sInput, SIGNAL(valueChanged(double)), this, SLOT(setSaturation(double)));
    connect(m_xSlider, SIGNAL(valueChanged(int)), this, SLOT(valueSliderChanged(int)));
    connect(m_xInput, SIGNAL(valueChanged(double)), this, SLOT(setValue(double)));

    setMixMode(KisHsvColorSlider::MIX_MODE::HSV);

    // Set initial values
    QColor c = m_color->toQColor();
    getHsxF(c, &m_h, &m_s, &m_x);

    m_hInput->setValue(m_h);
    m_sInput->setValue(m_s);
    m_xInput->setValue(m_x);

    // Update sliders
    QColor minC, maxC;
    minC.setHsvF(0, 1, 1);
    maxC.setHsvF(1, 1, 1);
    m_hSlider->setColors(minC, maxC);
    m_hSlider->setCircularHue(true);

    recolorSliders();
}

void KisHsvColorInput::setMixMode(KisHsvColorSlider::MIX_MODE mixMode) {

    switch (mixMode) {
	case KisHsvColorSlider::MIX_MODE::HSL:
        m_xLabel->setText(i18nc("@label:slider Abbreviation for 'Lightness' of HSL color model", "L:"));
		break;
	case KisHsvColorSlider::MIX_MODE::HSY:
        m_xLabel->setText(i18nc("@label:slider Abbreviation for 'Luma' of HSY color model", "Y:"));
		break;
	case KisHsvColorSlider::MIX_MODE::HSI:
        m_xLabel->setText(i18nc("@label:slider Abbreviation for 'Intensity' of HSI color model", "I:"));
		break;
	default: // fallthrough
	case KisHsvColorSlider::MIX_MODE::HSV:
        m_xLabel->setText(i18nc("@label:slider Abbreviation for 'Value' of HSV color model", "V:"));
		break;
	}

	QColor c = m_color->toQColor();
	m_mixMode = mixMode;
	getHsxF(c, &m_h, &m_s, &m_x);

	m_sSlider->setMixMode(m_mixMode);
	m_xSlider->setMixMode(m_mixMode);

    sendUpdate();
}

void KisHsvColorInput::sendUpdate()
{
    {
        KisSignalsBlocker blocker(
            m_hSlider, m_sSlider, m_xSlider,
            m_hInput, m_sInput, m_xInput
        );
        m_hSlider->setValue(m_h * 360);
        m_sSlider->setValue(m_s * 100);
        m_xSlider->setValue(m_x * 100);

		m_hInput->setValue(m_h * 360);
		m_sInput->setValue(m_s * 100);
		m_xInput->setValue(m_x * 100);
    }

    recolorSliders();

	QColor c;
    fillColor(c);

    m_color->fromQColor(c);
    emit(updated());
}

void KisHsvColorInput::setHue(double x)
{
    x = qBound(0.0, x, 360.0);

    m_h = x / 360;
    sendUpdate();
}

void KisHsvColorInput::setSaturation(double x)
{
    x = qBound(0.0, x, 100.0);

    m_s = x / 100;
    sendUpdate();
}

void KisHsvColorInput::setValue(double x)
{
    x = qBound(0.0, x, 100.0);

    m_x = x / 100;
    sendUpdate();
}

void KisHsvColorInput::hueSliderChanged(int i)
{
    m_hInput->setValue(i);
}

void KisHsvColorInput::saturationSliderChanged(int i)
{
    m_sInput->setValue(i);
}

void KisHsvColorInput::valueSliderChanged(int i)
{
    m_xInput->setValue(i);
}

void KisHsvColorInput::recolorSliders() {
    // Update sliders
    QColor minC, maxC;

    minC.setHsvF(m_h, 0, m_x);
    maxC.setHsvF(m_h, 1, m_x);
    m_sSlider->setColors(minC, maxC);

    minC.setHsvF(m_h, m_s, 0);
    maxC.setHsvF(m_h, m_s, 1);
    m_xSlider->setColors(minC, maxC);
}

void KisHsvColorInput::update()
{
    KisSignalsBlocker blocker(
        m_hInput, m_sInput, m_xInput,
        m_hSlider, m_sSlider, m_xSlider
    );

    // Check if it is the same color we have
    QColor current;

    fillColor(current);

    QColor theirs = m_color->toQColor();

    // Truncate to integer for this check
    if (!(current.red() == theirs.red() && current.green() == theirs.green() && current.blue() == theirs.blue())) {
        // Apply the update
        qreal theirH;
        getHsxF(theirs, &theirH, &m_s, &m_x);

        // Don't jump the Hue slider around to 0 if it is currently on 360
        const qreal EPSILON = 1e-6;
        if (!((1.0 - m_h) < EPSILON && (theirH - 0.0) < EPSILON)) {
            m_h = theirH;
        }

        m_hInput->setValue(m_h * 360);
        m_sInput->setValue(m_s * 100);
        m_xInput->setValue(m_x * 100);

        recolorSliders();

        // Update slider positions
        m_hSlider->setValue(m_h * 360);
        m_sSlider->setValue(m_s * 100);
        m_xSlider->setValue(m_x * 100);
    }
}

void KisHsvColorInput::fillColor(QColor& c) {
    fillColor(c, m_h, m_s, m_x);
}

void KisHsvColorInput::fillColor(QColor& c, const qreal& h, const qreal& s, const qreal& x)
{
	switch (m_mixMode) {
	case KisHsvColorSlider::MIX_MODE::HSL:
		c.setHslF(h, s, x);
		break;

	case KisHsvColorSlider::MIX_MODE::HSY: {
		qreal r, g, b;
		HSYToRGB(h, s, x, &r, &g, &b);

		// Clamp
		r = qBound(0.0, r, 1.0);
		g = qBound(0.0, g, 1.0);
		b = qBound(0.0, b, 1.0);

		c.setRgbF(r, g, b);
		break;
	}

	case KisHsvColorSlider::MIX_MODE::HSI: {
		qreal r, g, b;
		HSIToRGB(h, s, x, &r, &g, &b);
		c.setRgbF(r, g, b);
		break;
	}

	default: // fallthrough
	case KisHsvColorSlider::MIX_MODE::HSV:
		c.setHsvF(h, s, x);
		break;
	}
}

void KisHsvColorInput::getHsxF(const QColor& color, qreal* h, qreal* s, qreal* x)
{
    qreal tempH;
#if (QT_VERSION > QT_VERSION_CHECK(6, 0, 0))
    float fS = *s;
    float fX = *x;
#endif
    switch (m_mixMode) {
    case KisHsvColorSlider::MIX_MODE::HSL:
    {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        color.getHslF(&tempH, s, x);
#else
        float th;
        color.getHslF(&th, &fS, &fX);
        tempH = th;
        *s = fS;
        *x = fX;
#endif
    }
		break;

	case KisHsvColorSlider::MIX_MODE::HSY: {
        RGBToHSY(color.redF(), color.greenF(), color.blueF(), &tempH, s, x);
		break;
	}

	case KisHsvColorSlider::MIX_MODE::HSI: {
        RGBToHSI(color.redF(), color.greenF(), color.blueF(), &tempH, s, x);
		break;
	}

	default: // fallthrough
	case KisHsvColorSlider::MIX_MODE::HSV:
    {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
		color.getHsvF(&tempH, s, x);
#else
        float th;
        color.getHsvF(&th, &fS, &fX);
        tempH = th;
        *s = fS;
        *x = fX;
#endif
    }
		break;
	}

    if (tempH >= 0.0 && tempH <= 1.0) {
        *h = tempH;
    }
}
