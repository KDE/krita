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

#include "OffsetEffectConfigWidget.h"
#include "OffsetEffect.h"
#include "KoFilterEffect.h"
#include <QSpinBox>
#include <klocalizedstring.h>
#include <QGridLayout>
#include <QLabel>

const qreal OffsetLimit = 100.0;

OffsetEffectConfigWidget::OffsetEffectConfigWidget(QWidget *parent)
    : KoFilterEffectConfigWidgetBase(parent)
    , m_effect(0)
{
    QGridLayout *g = new QGridLayout(this);

    g->addWidget(new QLabel(i18n("dx"), this), 0, 0);
    m_offsetX = new QDoubleSpinBox(this);
    m_offsetX->setRange(-OffsetLimit, OffsetLimit);
    m_offsetX->setSingleStep(1.0);
    g->addWidget(m_offsetX, 0, 1);

    g->addWidget(new QLabel(i18n("dy"), this), 0, 2);
    m_offsetY = new QDoubleSpinBox(this);
    m_offsetY->setRange(-OffsetLimit, OffsetLimit);
    m_offsetY->setSingleStep(1.0);
    g->addWidget(m_offsetY, 0, 3);
    setLayout(g);

    connect(m_offsetX, SIGNAL(valueChanged(double)), this, SLOT(offsetChanged(double)));
    connect(m_offsetY, SIGNAL(valueChanged(double)), this, SLOT(offsetChanged(double)));
}

bool OffsetEffectConfigWidget::editFilterEffect(KoFilterEffect *filterEffect)
{
    m_effect = dynamic_cast<OffsetEffect *>(filterEffect);
    if (!m_effect) {
        return false;
    }

    m_offsetX->blockSignals(true);
    m_offsetY->blockSignals(true);
    m_offsetX->setValue(m_effect->offset().x() * 100.0);
    m_offsetY->setValue(m_effect->offset().y() * 100.0);
    m_offsetX->blockSignals(false);
    m_offsetY->blockSignals(false);

    return true;
}

void OffsetEffectConfigWidget::offsetChanged(double /*offset*/)
{
    if (!m_effect) {
        return;
    }

    m_effect->setOffset(0.01 * QPointF(m_offsetX->value(), m_offsetY->value()));
    emit filterChanged();
}
