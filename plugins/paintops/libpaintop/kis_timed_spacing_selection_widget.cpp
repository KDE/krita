#include "kis_timed_spacing_selection_widget.h"

const qreal MINIMUM_RATE = 1.0;
const qreal MAXIMUM_RATE = 1000.0;
const int RATE_NUM_DECIMALS = 2;
const qreal RATE_EXPONENT_RATIO = 1.0;
const qreal RATE_SINGLE_STEP = 0.01;
const qreal DEFAULT_RATE = 10.0;

KisTimedSpacingSelectionWidget::KisTimedSpacingSelectionWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    // Set up constraints and default value for the rate slider.
    sliderRate->setRange(MINIMUM_RATE, MAXIMUM_RATE, RATE_NUM_DECIMALS);
    sliderRate->setExponentRatio(RATE_EXPONENT_RATIO);
    sliderRate->setSingleStep(RATE_SINGLE_STEP);
    sliderRate->setValue(DEFAULT_RATE);

    connect(checkBoxEnabled, SIGNAL(toggled(bool)), SLOT(slotEnabledChanged(bool)));
    connect(sliderRate, SIGNAL(valueChanged(qreal)), SLOT(slotRateChanged(qreal)));
}

void KisTimedSpacingSelectionWidget::setTimedSpacing(bool enabled, qreal rate)
{
    checkBoxEnabled->setChecked(enabled);
    sliderRate->setValue(rate);
}

bool KisTimedSpacingSelectionWidget::isTimedSpacingEnabled() const
{
    return checkBoxEnabled->isChecked();
}

qreal KisTimedSpacingSelectionWidget::rate() const
{
    return sliderRate->value();
}

void KisTimedSpacingSelectionWidget::slotEnabledChanged(bool enabled)
{
    sliderRate->setEnabled(enabled);

    emit sigTimedSpacingChanged();
}

void KisTimedSpacingSelectionWidget::slotRateChanged(qreal rate)
{
    emit sigTimedSpacingChanged();
}
