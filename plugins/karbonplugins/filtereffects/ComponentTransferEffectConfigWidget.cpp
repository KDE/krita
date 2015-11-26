/* This file is part of the KDE project
 * Copyright (c) 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ComponentTransferEffectConfigWidget.h"
#include "KoFilterEffect.h"

#include <QSpinBox>
#include <kcombobox.h>
#include <klineedit.h>
#include <klocalizedstring.h>

#include <QGridLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QRadioButton>
#include <QButtonGroup>

const qreal ValueStep = 0.1;

ComponentTransferEffectConfigWidget::ComponentTransferEffectConfigWidget(QWidget *parent)
    : KoFilterEffectConfigWidgetBase(parent)
    , m_effect(0)
    , m_currentChannel(ComponentTransferEffect::ChannelR)
{
    QGridLayout *g = new QGridLayout(this);

    QButtonGroup *group = new QButtonGroup(this);

    QRadioButton *butR = new QRadioButton("R", this);
    QRadioButton *butG = new QRadioButton("G", this);
    QRadioButton *butB = new QRadioButton("B", this);
    QRadioButton *butA = new QRadioButton("A", this);
    g->addWidget(butR, 0, 0);
    g->addWidget(butG, 0, 1);
    g->addWidget(butB, 0, 2);
    g->addWidget(butA, 0, 3);
    group->addButton(butR, ComponentTransferEffect::ChannelR);
    group->addButton(butG, ComponentTransferEffect::ChannelG);
    group->addButton(butB, ComponentTransferEffect::ChannelB);
    group->addButton(butA, ComponentTransferEffect::ChannelA);
    butR->setChecked(true);

    g->addWidget(new QLabel(i18n("Function"), this), 1, 0, 1, 2);
    m_function = new KComboBox(this);
    m_function->addItem(i18n("Identity"));
    m_function->addItem(i18n("Table"));
    m_function->addItem(i18n("Discrete"));
    m_function->addItem(i18n("Linear"));
    m_function->addItem(i18n("Gamma"));
    g->addWidget(m_function, 1, 2, 1, 2);

    m_stack = new QStackedWidget(this);
    m_stack->setContentsMargins(0, 0, 0, 0);
    g->addWidget(m_stack, 2, 0, 1, 4);

    // Identity widget
    m_stack->addWidget(new QWidget(this));

    // Table widget
    QWidget *tableWidget = new QWidget(m_stack);
    QGridLayout *tableLayout = new QGridLayout(tableWidget);
    tableLayout->addWidget(new QLabel(i18n("Values"), tableWidget), 0, 0);
    m_tableValues = new KLineEdit(tableWidget);
    tableLayout->addWidget(m_tableValues, 0, 1);
    tableLayout->setContentsMargins(0, 0, 0, 0);
    tableLayout->addItem(new QSpacerItem(0, 1, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 1, 0);
    m_stack->addWidget(tableWidget);

    // Discrete widget
    QWidget *discreteWidget = new QWidget(m_stack);
    QGridLayout *discreteLayout = new QGridLayout(discreteWidget);
    discreteLayout->addWidget(new QLabel(i18n("Values"), discreteWidget), 0, 0);
    m_discreteValues = new KLineEdit(discreteWidget);
    discreteLayout->addWidget(m_discreteValues, 0, 1);
    discreteLayout->setContentsMargins(0, 0, 0, 0);
    discreteLayout->addItem(new QSpacerItem(0, 1, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 1, 0);
    m_stack->addWidget(discreteWidget);

    // Linear widget
    QWidget *linearWidget = new QWidget(m_stack);
    QGridLayout *linearLayout = new QGridLayout(linearWidget);
    linearLayout->addWidget(new QLabel(i18n("Slope"), linearWidget), 0, 0);
    m_slope = new QDoubleSpinBox(linearWidget);
    m_slope->setRange(m_slope->minimum(), m_slope->maximum());
    m_slope->setSingleStep(ValueStep);
    linearLayout->addWidget(m_slope, 0, 1);
    linearLayout->addWidget(new QLabel(i18n("Intercept")), 1, 0);
    m_intercept = new QDoubleSpinBox(linearWidget);
    m_intercept->setRange(m_intercept->minimum(), m_intercept->maximum());
    m_intercept->setSingleStep(ValueStep);
    linearLayout->addWidget(m_intercept, 1, 1);
    linearLayout->addItem(new QSpacerItem(0, 1, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 2, 0);
    linearLayout->setContentsMargins(0, 0, 0, 0);
    linearWidget->setLayout(linearLayout);
    m_stack->addWidget(linearWidget);

    QWidget *gammaWidget = new QWidget(m_stack);
    QGridLayout *gammaLayout = new QGridLayout(gammaWidget);
    gammaLayout->addWidget(new QLabel(i18n("Amplitude"), gammaWidget), 0, 0);
    m_amplitude = new QDoubleSpinBox(gammaWidget);
    m_amplitude->setRange(m_amplitude->minimum(), m_amplitude->maximum());
    m_amplitude->setSingleStep(ValueStep);
    gammaLayout->addWidget(m_amplitude, 0, 1);
    gammaLayout->addWidget(new QLabel(i18n("Exponent"), gammaWidget), 1, 0);
    m_exponent = new QDoubleSpinBox(gammaWidget);
    m_exponent->setRange(m_exponent->minimum(), m_exponent->maximum());
    m_exponent->setSingleStep(ValueStep);
    gammaLayout->addWidget(m_exponent, 1, 1);
    gammaLayout->addWidget(new QLabel(i18n("Offset"), gammaWidget), 2, 0);
    m_offset = new QDoubleSpinBox(gammaWidget);
    m_offset->setRange(m_offset->minimum(), m_offset->maximum());
    m_offset->setSingleStep(ValueStep);
    gammaLayout->addWidget(m_offset, 2, 1);
    gammaLayout->addItem(new QSpacerItem(0, 1, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 3, 0);
    gammaLayout->setContentsMargins(0, 0, 0, 0);
    gammaWidget->setLayout(gammaLayout);
    m_stack->addWidget(gammaWidget);

    setLayout(g);

    connect(m_function, SIGNAL(currentIndexChanged(int)), m_stack, SLOT(setCurrentIndex(int)));
    connect(m_function, SIGNAL(currentIndexChanged(int)), this, SLOT(functionChanged(int)));
    connect(m_tableValues, SIGNAL(editingFinished()), this, SLOT(tableValuesChanged()));
    connect(m_discreteValues, SIGNAL(editingFinished()), this, SLOT(discreteValuesChanged()));
    connect(m_slope, SIGNAL(valueChanged(double)), this, SLOT(slopeChanged(double)));
    connect(m_intercept, SIGNAL(valueChanged(double)), this, SLOT(interceptChanged(double)));
    connect(m_amplitude, SIGNAL(valueChanged(double)), this, SLOT(amplitudeChanged(double)));
    connect(m_exponent, SIGNAL(valueChanged(double)), this, SLOT(exponentChanged(double)));
    connect(m_offset, SIGNAL(valueChanged(double)), this, SLOT(offsetChanged(double)));
    connect(group, SIGNAL(buttonClicked(int)), this, SLOT(channelSelected(int)));
}

void ComponentTransferEffectConfigWidget::updateControls()
{
    m_function->blockSignals(true);

    QString values;

    switch (m_effect->function(m_currentChannel)) {
    case ComponentTransferEffect::Identity:
        m_function->setCurrentIndex(0);
        break;
    case ComponentTransferEffect::Table:
        m_function->setCurrentIndex(1);
        m_tableValues->blockSignals(true);
        Q_FOREACH (qreal v, m_effect->tableValues(m_currentChannel)) {
            values += QString("%1;").arg(v);
        }
        m_tableValues->setText(values);
        m_tableValues->blockSignals(false);
        break;
    case ComponentTransferEffect::Discrete:
        m_function->setCurrentIndex(2);
        m_discreteValues->blockSignals(true);
        Q_FOREACH (qreal v, m_effect->tableValues(m_currentChannel)) {
            values += QString("%1;").arg(v);
        }
        m_discreteValues->setText(values);
        m_discreteValues->blockSignals(false);
        break;
    case ComponentTransferEffect::Linear:
        m_function->setCurrentIndex(3);
        m_slope->blockSignals(true);
        m_slope->setValue(m_effect->slope(m_currentChannel));
        m_slope->blockSignals(false);
        m_intercept->blockSignals(true);
        m_intercept->setValue(m_effect->intercept(m_currentChannel));
        m_intercept->blockSignals(false);
        break;
    case ComponentTransferEffect::Gamma:
        m_function->setCurrentIndex(4);
        m_amplitude->blockSignals(true);
        m_amplitude->setValue(m_effect->amplitude(m_currentChannel));
        m_amplitude->blockSignals(false);
        m_exponent->blockSignals(true);
        m_exponent->setValue(m_effect->exponent(m_currentChannel));
        m_exponent->blockSignals(false);
        m_offset->blockSignals(true);
        m_offset->setValue(m_effect->offset(m_currentChannel));
        m_offset->blockSignals(false);
        break;
    }

    m_function->blockSignals(false);
    m_stack->setCurrentIndex(m_function->currentIndex());
}

bool ComponentTransferEffectConfigWidget::editFilterEffect(KoFilterEffect *filterEffect)
{
    m_effect = dynamic_cast<ComponentTransferEffect *>(filterEffect);
    if (!m_effect) {
        return false;
    }

    updateControls();

    return true;
}

void ComponentTransferEffectConfigWidget::slopeChanged(double slope)
{
    if (!m_effect) {
        return;
    }

    m_effect->setSlope(m_currentChannel, slope);
    emit filterChanged();
}

void ComponentTransferEffectConfigWidget::interceptChanged(double intercept)
{
    if (!m_effect) {
        return;
    }

    m_effect->setIntercept(m_currentChannel, intercept);
    emit filterChanged();
}

void ComponentTransferEffectConfigWidget::amplitudeChanged(double amplitude)
{
    if (!m_effect) {
        return;
    }

    m_effect->setAmplitude(m_currentChannel, amplitude);
    emit filterChanged();
}

void ComponentTransferEffectConfigWidget::exponentChanged(double exponent)
{
    if (!m_effect) {
        return;
    }

    m_effect->setExponent(m_currentChannel, exponent);
    emit filterChanged();
}

void ComponentTransferEffectConfigWidget::offsetChanged(double offset)
{
    if (!m_effect) {
        return;
    }

    m_effect->setOffset(m_currentChannel, offset);
    emit filterChanged();
}

void ComponentTransferEffectConfigWidget::tableValuesChanged()
{
    QStringList values = m_tableValues->text().split(';', QString::SkipEmptyParts);
    QList<qreal> tableValues;
    Q_FOREACH (const QString &v, values) {
        tableValues.append(v.toDouble());
    }
    m_effect->setTableValues(m_currentChannel, tableValues);
    emit filterChanged();
}

void ComponentTransferEffectConfigWidget::discreteValuesChanged()
{
    QStringList values = m_discreteValues->text().split(';', QString::SkipEmptyParts);
    QList<qreal> tableValues;
    Q_FOREACH (const QString &v, values) {
        tableValues.append(v.toDouble());
    }
    m_effect->setTableValues(m_currentChannel, tableValues);
    emit filterChanged();
}

void ComponentTransferEffectConfigWidget::functionChanged(int index)
{
    if (!m_effect) {
        return;
    }

    m_effect->setFunction(m_currentChannel, static_cast<ComponentTransferEffect::Function>(index));

    emit filterChanged();
}

void ComponentTransferEffectConfigWidget::channelSelected(int channel)
{
    m_currentChannel = static_cast<ComponentTransferEffect::Channel>(channel);
    updateControls();
}
