/* This file is part of the KDE project
 * Copyright (c) 2009-2010 Jan Hambrecht <jaham@gmx.net>
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

#include "ColorMatrixEffectConfigWidget.h"
#include "ColorMatrixEffect.h"
#include "KoFilterEffect.h"
#include "MatrixDataModel.h"

#include <QSpinBox>
#include <kcombobox.h>
#include <klocalizedstring.h>

#include <QGridLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QTableView>
#include <QHeaderView>

ColorMatrixEffectConfigWidget::ColorMatrixEffectConfigWidget(QWidget *parent)
    : KoFilterEffectConfigWidgetBase(parent)
    , m_effect(0)
{
    QGridLayout *g = new QGridLayout(this);

    m_type = new KComboBox(this);
    m_type->addItem(i18n("Apply color matrix"));
    m_type->addItem(i18n("Saturate colors"));
    m_type->addItem(i18n("Rotate hue"));
    m_type->addItem(i18n("Luminance to alpha"));
    g->addWidget(m_type, 0, 0);

    m_stack = new QStackedWidget(this);
    m_stack->setContentsMargins(0, 0, 0, 0);
    g->addWidget(m_stack, 1, 0);

    m_matrixModel = new MatrixDataModel(this);

    QTableView *matrixWidget = new QTableView(m_stack);
    matrixWidget->setModel(m_matrixModel);
    matrixWidget->horizontalHeader()->hide();
    matrixWidget->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    matrixWidget->verticalHeader()->hide();
    matrixWidget->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    m_stack->addWidget(matrixWidget);

    QWidget *saturateWidget = new QWidget(m_stack);
    QGridLayout *saturateLayout = new QGridLayout(saturateWidget);
    saturateLayout->addWidget(new QLabel(i18n("Saturate value"), saturateWidget), 0, 0);
    m_saturate = new QDoubleSpinBox(saturateWidget);
    m_saturate->setRange(0.0, 1.0);
    m_saturate->setSingleStep(0.05);
    saturateLayout->addWidget(m_saturate, 0, 1);
    saturateLayout->addItem(new QSpacerItem(0, 1, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 1, 0);
    saturateWidget->setLayout(saturateLayout);
    m_stack->addWidget(saturateWidget);

    QWidget *hueRotateWidget = new QWidget(m_stack);
    QGridLayout *hueRotateLayout = new QGridLayout(hueRotateWidget);
    hueRotateLayout->addWidget(new QLabel(i18n("Angle"), hueRotateWidget), 0, 0);
    m_hueRotate = new QDoubleSpinBox(hueRotateWidget);
    m_hueRotate->setRange(0.0, 360.0);
    m_hueRotate->setSingleStep(1.0);
    hueRotateLayout->addWidget(m_hueRotate, 0, 1);
    hueRotateLayout->addItem(new QSpacerItem(0, 1, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 1, 0);
    hueRotateWidget->setLayout(hueRotateLayout);
    m_stack->addWidget(hueRotateWidget);

    QWidget *luminanceWidget = new QWidget(m_stack);
    m_stack->addWidget(luminanceWidget);

    setLayout(g);

    connect(m_type, SIGNAL(currentIndexChanged(int)), m_stack, SLOT(setCurrentIndex(int)));
    connect(m_type, SIGNAL(currentIndexChanged(int)), this, SLOT(typeChanged(int)));
    connect(m_saturate, SIGNAL(valueChanged(double)), this, SLOT(saturateChanged(double)));
    connect(m_hueRotate, SIGNAL(valueChanged(double)), this, SLOT(hueRotateChanged(double)));
    connect(m_matrixModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(matrixChanged()));
}

bool ColorMatrixEffectConfigWidget::editFilterEffect(KoFilterEffect *filterEffect)
{
    m_effect = dynamic_cast<ColorMatrixEffect *>(filterEffect);
    if (!m_effect) {
        return false;
    }

    m_type->blockSignals(true);

    switch (m_effect->type()) {
    case ColorMatrixEffect::Matrix:
        m_type->setCurrentIndex(0);
        m_matrixModel->setMatrix(m_effect->colorMatrix(), m_effect->colorMatrixRowCount(), m_effect->colorMatrixColumnCount());
        break;
    case ColorMatrixEffect::Saturate:
        m_type->setCurrentIndex(1);
        m_saturate->blockSignals(true);
        m_saturate->setValue(m_effect->saturate());
        m_saturate->blockSignals(false);
        break;
    case ColorMatrixEffect::HueRotate:
        m_type->setCurrentIndex(2);
        m_hueRotate->blockSignals(true);
        m_hueRotate->setValue(m_effect->hueRotate());
        m_hueRotate->blockSignals(false);
        break;
    case ColorMatrixEffect::LuminanceAlpha:
        m_type->setCurrentIndex(3);
        break;
    }

    m_type->blockSignals(false);
    m_stack->setCurrentIndex(m_type->currentIndex());

    return true;
}

void ColorMatrixEffectConfigWidget::matrixChanged()
{
    if (!m_effect) {
        return;
    }

    m_effect->setColorMatrix(m_matrixModel->matrix());
    emit filterChanged();
}

void ColorMatrixEffectConfigWidget::saturateChanged(double saturate)
{
    if (!m_effect) {
        return;
    }

    m_effect->setSaturate(saturate);
    emit filterChanged();
}

void ColorMatrixEffectConfigWidget::hueRotateChanged(double angle)
{
    if (!m_effect) {
        return;
    }

    m_effect->setHueRotate(angle);
    emit filterChanged();
}

void ColorMatrixEffectConfigWidget::typeChanged(int index)
{
    if (!m_effect) {
        return;
    }

    if (index == ColorMatrixEffect::Matrix) {
        m_effect->setColorMatrix(m_matrixModel->matrix());
    } else if (index == ColorMatrixEffect::Saturate) {
        m_effect->setSaturate(m_saturate->value());
    } else if (index == ColorMatrixEffect::HueRotate) {
        m_effect->setHueRotate(m_hueRotate->value());
    } else {
        m_effect->setLuminanceAlpha();
    }
    emit filterChanged();
}
