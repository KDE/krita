/* This file is part of the KDE project
 * Copyright (c) 2010 Jan Hambrecht <jaham@gmx.net>
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

#include "MorphologyEffectConfigWidget.h"
#include "MorphologyEffect.h"
#include "KoFilterEffect.h"

#include <QSpinBox>
#include <klocalizedstring.h>

#include <QGridLayout>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>

MorphologyEffectConfigWidget::MorphologyEffectConfigWidget(QWidget *parent)
    : KoFilterEffectConfigWidgetBase(parent)
    , m_effect(0)
{
    QGridLayout *g = new QGridLayout(this);

    m_operator = new QButtonGroup(this);
    QRadioButton *erode = new QRadioButton(i18n("Erode"), this);
    QRadioButton *dilate = new QRadioButton(i18n("Dilate"), this);
    m_operator->addButton(erode, MorphologyEffect::Erode);
    m_operator->addButton(dilate, MorphologyEffect::Dilate);
    g->addWidget(new QLabel(i18n("Operator:"), this), 0, 0);
    g->addWidget(erode, 0, 1);
    g->addWidget(dilate, 0, 2);

    g->addWidget(new QLabel(i18n("Radius x:"), this), 1, 0);
    m_radiusX = new QDoubleSpinBox(this);
    m_radiusX->setRange(0.0, 100);
    m_radiusX->setSingleStep(0.5);
    g->addWidget(m_radiusX, 1, 1, 1, 2);

    g->addWidget(new QLabel(i18n("Radius y:"), this), 2, 0);
    m_radiusY = new QDoubleSpinBox(this);
    m_radiusY->setRange(0.0, 100);
    m_radiusY->setSingleStep(0.5);
    g->addWidget(m_radiusY, 2, 1, 1, 2);

    setLayout(g);

    connect(m_operator, SIGNAL(buttonClicked(int)), this, SLOT(operatorChanged(int)));
    connect(m_radiusX, SIGNAL(valueChanged(double)), this, SLOT(radiusXChanged(double)));
    connect(m_radiusY, SIGNAL(valueChanged(double)), this, SLOT(radiusYChanged(double)));
}

bool MorphologyEffectConfigWidget::editFilterEffect(KoFilterEffect *filterEffect)
{
    m_effect = dynamic_cast<MorphologyEffect *>(filterEffect);
    if (!m_effect) {
        return false;
    }

    m_operator->blockSignals(true);
    m_operator->button(m_effect->morphologyOperator())->setChecked(true);
    m_operator->blockSignals(false);
    m_radiusX->blockSignals(true);
    m_radiusX->setValue(m_effect->morphologyRadius().x() * 100);
    m_radiusX->blockSignals(false);
    m_radiusY->blockSignals(true);
    m_radiusY->setValue(m_effect->morphologyRadius().y() * 100);
    m_radiusY->blockSignals(false);

    return true;
}

void MorphologyEffectConfigWidget::operatorChanged(int id)
{
    if (!m_effect) {
        return;
    }

    switch (id) {
    case MorphologyEffect::Erode:
        m_effect->setMorphologyOperator(MorphologyEffect::Erode);
        break;
    case MorphologyEffect::Dilate:
        m_effect->setMorphologyOperator(MorphologyEffect::Dilate);
        break;
    }
    emit filterChanged();
}

void MorphologyEffectConfigWidget::radiusXChanged(double x)
{
    if (!m_effect) {
        return;
    }

    QPointF radius = m_effect->morphologyRadius();
    if (radius.x() != x) {
        m_effect->setMorphologyRadius(QPointF(x * 0.01, radius.y()));
    }

    emit filterChanged();
}

void MorphologyEffectConfigWidget::radiusYChanged(double y)
{
    if (!m_effect) {
        return;
    }

    QPointF radius = m_effect->morphologyRadius();
    if (radius.y() != y) {
        m_effect->setMorphologyRadius(QPointF(radius.x(), y * 0.01));
    }

    emit filterChanged();
}
