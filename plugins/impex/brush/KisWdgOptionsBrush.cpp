/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2019 Iván SantaMaría <ghevan@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisWdgOptionsBrush.h"

#include <KisViewManager.h>
#include <kis_image.h>
#include <KoProperties.h>
#include <KisDocument.h>

KisWdgOptionsBrush::KisWdgOptionsBrush(QWidget *parent)
    : KisConfigWidget(parent)
    , m_currentDimensions(0)
    , m_layersCount(0)
    , m_view(0)
{
    setupUi(this);
    connect(this->brushStyle, SIGNAL(currentIndexChanged(int)), SLOT(slotEnableSelectionMethod(int)));
    connect(this->dimensionSpin, SIGNAL(valueChanged(int)), SLOT(slotActivateDimensionRanks()));

    slotEnableSelectionMethod(brushStyle->currentIndex());

    BrushPipeSelectionModeHelper *bp;
    for (int i = 0; i < this->dimensionSpin->maximum(); i++) {
        bp = new BrushPipeSelectionModeHelper(0, i);
        connect(bp, SIGNAL(sigRankChanged(int)), SLOT(slotRecalculateRanks(int)));
        dimRankLayout->addWidget(bp);
    }

    slotActivateDimensionRanks();
}

void KisWdgOptionsBrush::setConfiguration(const KisPropertiesConfigurationSP cfg)
{
    spacingWidget->setSpacing(false, cfg->getDouble("spacing"));
    if (nameLineEdit->text().isEmpty()) {
        nameLineEdit->setText(cfg->getString("name"));
    }
    colorAsMask->setChecked(cfg->getBool("mask"));
    brushStyle->setCurrentIndex(cfg->getInt("brushStyle"));
    dimensionSpin->setValue(cfg->getInt("dimensions"));

    QLayoutItem *item;
    BrushPipeSelectionModeHelper *bp;
    for (int i = 0; i < dimensionSpin->maximum(); ++i) {
        if ((item = dimRankLayout->itemAt(i)) != 0) {
            bp = dynamic_cast<BrushPipeSelectionModeHelper*>(item->widget());
            bp->cmbSelectionMode.setCurrentIndex(cfg->getInt("selectionMode" + QString::number(i)));
            bp->rankSpinBox.setValue(cfg->getInt("rank" + QString::number(i)));
        }
    }
}

KisPropertiesConfigurationSP KisWdgOptionsBrush::configuration() const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("spacing", spacingWidget->spacing());
    cfg->setProperty("name", nameLineEdit->text());
    cfg->setProperty("mask", colorAsMask->isChecked());
    cfg->setProperty("brushStyle", brushStyle->currentIndex());
    cfg->setProperty("dimensions", dimensionSpin->value());

    QLayoutItem *item;
    BrushPipeSelectionModeHelper *bp;
    for (int i = 0; i < dimensionSpin->maximum(); ++i) {
        if ((item = dimRankLayout->itemAt(i)) != 0) {
            bp = dynamic_cast<BrushPipeSelectionModeHelper*>(item->widget());
            cfg->setProperty("selectionMode" + QString::number(i), bp->cmbSelectionMode.currentIndex());
            cfg->setProperty("rank" + QString::number(i),  bp->rankSpinBox.value());
        }
    }

    return cfg;
}

void KisWdgOptionsBrush::setView(KisViewManager *view)
{
    if (view) {
        m_view = view;
        KoProperties properties;
        properties.setProperty("visible", true);
        m_layersCount = m_view->image()->root()->childNodes(QStringList("KisLayer"), properties).count();

        slotRecalculateRanks();
    }
}

void KisWdgOptionsBrush::slotEnableSelectionMethod(int value)
{
    if (value == 0) {
        animStyleGroup->setEnabled(false);
    } else {
        animStyleGroup->setEnabled(true);
    }
}

void KisWdgOptionsBrush::slotActivateDimensionRanks()
{
    QLayoutItem *item;
    BrushPipeSelectionModeHelper *bp;
    int dim = this->dimensionSpin->value();
    if (dim >= m_currentDimensions) {
        for (int i = m_currentDimensions; i < dim; ++i) {
            if ((item = dimRankLayout->itemAt(i)) != 0) {
                bp = dynamic_cast<BrushPipeSelectionModeHelper*>(item->widget());
                bp->setEnabled(true);
                bp->show();
            }
        }
    }
    else {
        for (int i = m_currentDimensions -1; i >= dim; --i) {
            if ((item = dimRankLayout->itemAt(i)) != 0) {
               bp = dynamic_cast<BrushPipeSelectionModeHelper*>(item->widget());
               bp->setEnabled(false);
               bp->hide();
            }
        }
    }
    m_currentDimensions = dim;
}

void KisWdgOptionsBrush::slotRecalculateRanks(int rankDimension)
{
    int rankSum = 0;
    int maxDim = this->dimensionSpin->maximum();

    QVector<BrushPipeSelectionModeHelper *> bp;
    QLayoutItem *item;

    for (int i = 0; i < maxDim; ++i) {
        if ((item = dimRankLayout->itemAt(i)) != 0) {
            bp.push_back(dynamic_cast<BrushPipeSelectionModeHelper*>(item->widget()));
            rankSum += bp.at(i)->rankSpinBox.value();
        }
    }

    BrushPipeSelectionModeHelper *currentBrushHelper;
    BrushPipeSelectionModeHelper *callerBrushHelper = bp.at(rankDimension);
    QVectorIterator<BrushPipeSelectionModeHelper*> bpIterator(bp);

    while (rankSum > m_layersCount && bpIterator.hasNext()) {
        currentBrushHelper = bpIterator.next();

        if (currentBrushHelper != callerBrushHelper) {
            int currentValue = currentBrushHelper->rankSpinBox.value();
            currentBrushHelper->rankSpinBox.setValue(currentValue -1);
            rankSum -= currentValue;
        }
    }

    if (rankSum > m_layersCount) {
        callerBrushHelper->rankSpinBox.setValue(m_layersCount);
    }

    if (rankSum == 0) {
        bp.at(0)->rankSpinBox.setValue(m_layersCount);
        return;
    }
}
