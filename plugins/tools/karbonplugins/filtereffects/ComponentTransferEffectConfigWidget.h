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

#ifndef COMPONENTTRANSFEREFFECTCONFIGWIDGET_H
#define COMPONENTTRANSFEREFFECTCONFIGWIDGET_H

#include "KoFilterEffectConfigWidgetBase.h"
#include "ComponentTransferEffect.h"

class KoFilterEffect;
class QDoubleSpinBox;
class KComboBox;
class KLineEdit;
class QStackedWidget;

class ComponentTransferEffectConfigWidget : public KoFilterEffectConfigWidgetBase
{
    Q_OBJECT
public:
    explicit ComponentTransferEffectConfigWidget(QWidget *parent = 0);

    /// reimplemented from KoFilterEffectConfigWidgetBase
    virtual bool editFilterEffect(KoFilterEffect *filterEffect);

private Q_SLOTS:
    void slopeChanged(double slope);
    void interceptChanged(double intercept);
    void amplitudeChanged(double amplitude);
    void exponentChanged(double exponent);
    void offsetChanged(double offset);
    void functionChanged(int index);
    void channelSelected(int channel);
    void tableValuesChanged();
    void discreteValuesChanged();
private:
    void updateControls();

    ComponentTransferEffect *m_effect;
    KComboBox *m_function;
    QStackedWidget *m_stack;
    KLineEdit *m_tableValues;
    KLineEdit *m_discreteValues;
    QDoubleSpinBox *m_slope;
    QDoubleSpinBox *m_intercept;
    QDoubleSpinBox *m_amplitude;
    QDoubleSpinBox *m_exponent;
    QDoubleSpinBox *m_offset;
    ComponentTransferEffect::Channel m_currentChannel;
};

#endif // COMPONENTTRANSFEREFFECTCONFIGWIDGET_H
