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

#include "CompositeEffectConfigWidget.h"
#include "CompositeEffect.h"
#include "KoFilterEffect.h"

#include <QSpinBox>
#include <kcombobox.h>
#include <klocalizedstring.h>

#include <QGridLayout>
#include <QLabel>

CompositeEffectConfigWidget::CompositeEffectConfigWidget(QWidget *parent)
    : KoFilterEffectConfigWidgetBase(parent)
    , m_effect(0)
{
    QGridLayout *g = new QGridLayout(this);

    g->addWidget(new QLabel(i18n("Operation"), this), 0, 0);

    m_operation = new KComboBox(this);
    m_operation->addItem("Over");
    m_operation->addItem("In");
    m_operation->addItem("Out");
    m_operation->addItem("Atop");
    m_operation->addItem("Xor");
    m_operation->addItem("Arithmetic");
    g->addWidget(m_operation, 0, 1);

    m_arithmeticWidget = new QWidget(this);
    QGridLayout *arithmeticLayout = new QGridLayout(m_arithmeticWidget);
    for (int i = 0; i < 4; ++i) {
        m_k[i] = new QDoubleSpinBox(m_arithmeticWidget);
        arithmeticLayout->addWidget(new QLabel(QString("k%1").arg(i + 1)), i / 2, (2 * i) % 4);
        arithmeticLayout->addWidget(m_k[i], i / 2, (2 * i + 1) % 4);
        connect(m_k[i], SIGNAL(valueChanged(double)), this, SLOT(valueChanged()));
    }
    m_arithmeticWidget->setContentsMargins(0, 0, 0, 0);
    g->addWidget(m_arithmeticWidget, 1, 0, 1, 2);
    g->addItem(new QSpacerItem(0, 1, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 2, 0);

    connect(m_operation, SIGNAL(currentIndexChanged(int)), this, SLOT(operationChanged(int)));
}

bool CompositeEffectConfigWidget::editFilterEffect(KoFilterEffect *filterEffect)
{
    m_effect = dynamic_cast<CompositeEffect *>(filterEffect);
    if (!m_effect) {
        return false;
    }

    m_operation->blockSignals(true);
    m_operation->setCurrentIndex(m_effect->operation());
    m_operation->blockSignals(false);

    const qreal *k = m_effect->arithmeticValues();
    for (int i = 0; i < 4; ++i) {
        m_k[i]->blockSignals(true);
        m_k[i]->setValue(k[i]);
        m_k[i]->blockSignals(false);
    }
    m_arithmeticWidget->setVisible(m_effect->operation() == CompositeEffect::Arithmetic);

    return true;
}

void CompositeEffectConfigWidget::operationChanged(int index)
{
    m_arithmeticWidget->setVisible(index == 6);
    if (m_effect) {
        m_effect->setOperation(static_cast<CompositeEffect::Operation>(index));
        emit filterChanged();
    }
}

void CompositeEffectConfigWidget::valueChanged()
{
    if (!m_effect) {
        return;
    }

    qreal k[4] = {0};
    for (int i = 0; i < 4; ++i) {
        k[i] = m_k[i]->value();
    }

    m_effect->setArithmeticValues(k);
    emit filterChanged();
}
