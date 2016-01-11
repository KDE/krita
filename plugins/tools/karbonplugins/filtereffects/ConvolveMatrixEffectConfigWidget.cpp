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

#include "ConvolveMatrixEffectConfigWidget.h"
#include "ConvolveMatrixEffect.h"
#include "KoFilterEffect.h"
#include "MatrixDataModel.h"

#include <klocalizedstring.h>
#include <kcombobox.h>
#include <QDialog>

#include <QGridLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include <QTableView>
#include <QHeaderView>
#include <KConfigGroup>

ConvolveMatrixEffectConfigWidget::ConvolveMatrixEffectConfigWidget(QWidget *parent)
    : KoFilterEffectConfigWidgetBase(parent)
    , m_effect(0)
{
    QGridLayout *g = new QGridLayout(this);

    m_edgeMode = new KComboBox(this);
    m_edgeMode->addItem(i18n("Duplicate"));
    m_edgeMode->addItem(i18n("Wrap"));
    m_edgeMode->addItem(i18n("None"));
    g->addWidget(new QLabel(i18n("Edge mode:"), this), 0, 0);
    g->addWidget(m_edgeMode, 0, 1, 1, 3);

    m_orderX = new QSpinBox(this);
    m_orderX->setRange(1, 30);
    m_orderY = new QSpinBox(this);
    m_orderY->setRange(1, 30);
    g->addWidget(new QLabel(i18n("Kernel size:"), this), 1, 0);
    g->addWidget(m_orderX, 1, 1);
    g->addWidget(new QLabel("X", this), 1, 2, Qt::AlignHCenter);
    g->addWidget(m_orderY, 1, 3);

    m_targetX = new QSpinBox(this);
    m_targetX->setRange(0, 30);
    m_targetY = new QSpinBox(this);
    m_targetY->setRange(0, 30);
    g->addWidget(new QLabel(i18n("Target point:"), this), 2, 0);
    g->addWidget(m_targetX, 2, 1);
    g->addWidget(new QLabel("X", this), 2, 2, Qt::AlignHCenter);
    g->addWidget(m_targetY, 2, 3);

    m_divisor = new QDoubleSpinBox(this);
    m_bias = new QDoubleSpinBox(this);
    g->addWidget(new QLabel(i18n("Divisor:"), this), 3, 0);
    g->addWidget(m_divisor, 3, 1);
    g->addWidget(new QLabel(i18n("Bias:"), this), 3, 2);
    g->addWidget(m_bias, 3, 3);

    m_preserveAlpha = new QCheckBox(i18n("Preserve alpha"), this);
    g->addWidget(m_preserveAlpha, 4, 1, 1, 3);

    QPushButton *kernelButton = new QPushButton(i18n("Edit kernel"), this);
    g->addWidget(kernelButton, 5, 0, 1, 4);

    setLayout(g);

    connect(m_edgeMode, SIGNAL(currentIndexChanged(int)), this, SLOT(edgeModeChanged(int)));
    connect(m_orderX, SIGNAL(valueChanged(int)), this, SLOT(orderChanged(int)));
    connect(m_orderY, SIGNAL(valueChanged(int)), this, SLOT(orderChanged(int)));
    connect(m_targetX, SIGNAL(valueChanged(int)), this, SLOT(targetChanged(int)));
    connect(m_targetY, SIGNAL(valueChanged(int)), this, SLOT(targetChanged(int)));
    connect(m_divisor, SIGNAL(valueChanged(double)), this, SLOT(divisorChanged(double)));
    connect(m_bias, SIGNAL(valueChanged(double)), this, SLOT(biasChanged(double)));
    connect(kernelButton, SIGNAL(clicked(bool)), this, SLOT(editKernel()));
    connect(m_preserveAlpha, SIGNAL(toggled(bool)), this, SLOT(preserveAlphaChanged(bool)));

    m_matrixModel = new MatrixDataModel(this);
}

bool ConvolveMatrixEffectConfigWidget::editFilterEffect(KoFilterEffect *filterEffect)
{
    m_effect = dynamic_cast<ConvolveMatrixEffect *>(filterEffect);
    if (!m_effect) {
        return false;
    }

    m_edgeMode->blockSignals(true);
    m_edgeMode->setCurrentIndex(m_effect->edgeMode());
    m_edgeMode->blockSignals(false);
    m_orderX->blockSignals(true);
    m_orderX->setValue(m_effect->order().x());
    m_orderX->blockSignals(false);
    m_orderY->blockSignals(true);
    m_orderY->setValue(m_effect->order().y());
    m_orderY->blockSignals(false);
    m_targetX->blockSignals(true);
    m_targetX->setMaximum(m_orderX->value());
    m_targetX->setValue(m_effect->target().x() + 1);
    m_targetX->blockSignals(false);
    m_targetY->blockSignals(true);
    m_targetY->setMaximum(m_orderY->value());
    m_targetY->setValue(m_effect->target().y() + 1);
    m_targetY->blockSignals(false);
    m_divisor->blockSignals(true);
    m_divisor->setValue(m_effect->divisor());
    m_divisor->blockSignals(false);
    m_bias->blockSignals(true);
    m_bias->setValue(m_effect->bias());
    m_bias->blockSignals(false);
    m_preserveAlpha->blockSignals(true);
    m_preserveAlpha->setChecked(m_effect->isPreserveAlphaEnabled());
    m_preserveAlpha->blockSignals(false);

    return true;
}

void ConvolveMatrixEffectConfigWidget::edgeModeChanged(int id)
{
    if (!m_effect) {
        return;
    }

    switch (id) {
    case ConvolveMatrixEffect::Duplicate:
        m_effect->setEdgeMode(ConvolveMatrixEffect::Duplicate);
        break;
    case ConvolveMatrixEffect::Wrap:
        m_effect->setEdgeMode(ConvolveMatrixEffect::Wrap);
        break;
    case ConvolveMatrixEffect::None:
        m_effect->setEdgeMode(ConvolveMatrixEffect::None);
        break;
    }
    emit filterChanged();
}

void ConvolveMatrixEffectConfigWidget::orderChanged(int)
{
    if (!m_effect) {
        return;
    }

    QPoint newOrder(m_orderX->value(), m_orderY->value());
    QPoint oldOrder = m_effect->order();
    if (newOrder != oldOrder) {
        m_effect->setOrder(newOrder);
        emit filterChanged();
    }

    m_targetX->setMaximum(newOrder.x());
    m_targetY->setMaximum(newOrder.y());
}

void ConvolveMatrixEffectConfigWidget::targetChanged(int)
{
    if (!m_effect) {
        return;
    }

    QPoint newTarget(m_targetX->value() - 1, m_targetY->value() - 1);
    QPoint oldTarget = m_effect->target();
    if (newTarget != oldTarget) {
        m_effect->setTarget(newTarget);
        emit filterChanged();
    }
}

void ConvolveMatrixEffectConfigWidget::divisorChanged(double divisor)
{
    if (!m_effect) {
        return;
    }

    if (divisor != m_effect->divisor()) {
        m_effect->setDivisor(divisor);
        emit filterChanged();
    }
}

void ConvolveMatrixEffectConfigWidget::biasChanged(double bias)
{
    if (!m_effect) {
        return;
    }

    if (bias != m_effect->bias()) {
        m_effect->setBias(bias);
        emit filterChanged();
    }
}

void ConvolveMatrixEffectConfigWidget::preserveAlphaChanged(bool checked)
{
    if (!m_effect) {
        return;
    }

    m_effect->enablePreserveAlpha(checked);
    emit filterChanged();
}

void ConvolveMatrixEffectConfigWidget::editKernel()
{
    if (!m_effect) {
        return;
    }

    QVector<qreal> oldKernel = m_effect->kernel();
    QPoint kernelSize = m_effect->order();
    m_matrixModel->setMatrix(oldKernel, kernelSize.y(), kernelSize.x());
    connect(m_matrixModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(kernelChanged()));

    QPointer<QDialog> dlg = new QDialog(this);
    QTableView *table = new QTableView(dlg);
    table->setModel(m_matrixModel);
    table->horizontalHeader()->hide();
    table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->hide();
    table->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    dlg->setLayout(mainLayout);
    mainLayout->addWidget(table);
    if (dlg->exec() == QDialog::Accepted) {
        m_effect->setKernel(m_matrixModel->matrix());
        emit filterChanged();
    } else {
        m_effect->setKernel(oldKernel);
    }
    delete dlg;

    disconnect(m_matrixModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(kernelChanged()));
}

void ConvolveMatrixEffectConfigWidget::kernelChanged()
{
    if (!m_effect) {
        return;
    }

    m_effect->setKernel(m_matrixModel->matrix());
    emit filterChanged();
}
