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

enum BrushStyle {
    Regular,
    Animated
};

KisWdgOptionsBrush::KisWdgOptionsBrush(QWidget *parent)
    : KisConfigWidget(parent)
    , m_currentDimensions(0)
    , m_layersCount(0)
    , m_view(0)
{
    setupUi(this);
    connect(this->brushStyle, SIGNAL(currentIndexChanged(int)), SLOT(slotEnableSelectionMethod(int)));
    connect(this->dimensionSpin, SIGNAL(valueChanged(int)), SLOT(slotActivateDimensionRanks()));
    connect(this->colorAsMask, SIGNAL(toggled(bool)), this->preserveAlpha, SLOT(setEnabled(bool)));

    connect(this->spacingWidget, SIGNAL(sigSpacingChanged()), this, SIGNAL(sigConfigurationUpdated()));
    connect(this->nameLineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(sigConfigurationUpdated()));
    connect(this->colorAsMask, SIGNAL(toggled(bool)), this, SIGNAL(sigConfigurationUpdated()));
    connect(this->preserveAlpha, SIGNAL(toggled(bool)), this, SIGNAL(sigConfigurationUpdated()));
    connect(this->brushStyle, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
    connect(this->dimensionSpin, SIGNAL(valueChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
    //todo: connect ranks to sigConfigurationUpdated?

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
    spacingWidget->setSpacing(false, cfg->getDouble(KEY_SPACING));
    if (!cfg->getString(KEY_NAME).isEmpty()) {
        nameLineEdit->setText(cfg->getString(KEY_NAME));
    }
    colorAsMask->setChecked(cfg->getBool(KEY_MASK));
    preserveAlpha->setChecked(cfg->getBool(KEY_PRESERVE_ALPHA));
    brushStyle->setCurrentIndex(cfg->getInt(KEY_BRUSH_STYLE));
    dimensionSpin->setValue(cfg->getInt(KEY_DIMENSIONS));

    QLayoutItem *item;
    BrushPipeSelectionModeHelper *bp;
    for (int i = 0; i < dimensionSpin->maximum(); ++i) {
        if ((item = dimRankLayout->itemAt(i)) != 0) {
            bp = dynamic_cast<BrushPipeSelectionModeHelper*>(item->widget());
            bp->cmbSelectionMode.setCurrentIndex(bp->cmbSelectionMode.findData(cfg->getInt(KEY_SELECTION_MODE + QString::number(i))));
            bp->rankSpinBox.setValue(cfg->getInt(KEY_RANK + QString::number(i)));
        }
    }
}

KisPropertiesConfigurationSP KisWdgOptionsBrush::configuration() const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty(KEY_SPACING, spacingWidget->spacing());
    cfg->setProperty(KEY_NAME, nameLineEdit->text());
    cfg->setProperty(KEY_MASK, colorAsMask->isChecked());
    cfg->setProperty(KEY_PRESERVE_ALPHA, preserveAlpha->isChecked());
    cfg->setProperty(KEY_BRUSH_STYLE, brushStyle->currentIndex());
    cfg->setProperty(KEY_DIMENSIONS, dimensionSpin->value());

    QLayoutItem *item;
    BrushPipeSelectionModeHelper *bp;
    for (int i = 0; i < dimensionSpin->maximum(); ++i) {
        if ((item = dimRankLayout->itemAt(i)) != 0) {
            bp = dynamic_cast<BrushPipeSelectionModeHelper*>(item->widget());
            cfg->setProperty(KEY_SELECTION_MODE + QString::number(i), bp->cmbSelectionMode.currentData());
            cfg->setProperty(KEY_RANK + QString::number(i),  bp->rankSpinBox.value());
        }
    }

    return cfg;
}

void KisWdgOptionsBrush::setView(KisViewManager *view)
{
    if (view) {
        m_view = view;
        if (m_view->image()) {
            KoProperties properties;
            properties.setProperty("visible", true);
            m_layersCount = m_view->image()->root()->childNodes(QStringList("KisLayer"), properties).count();
        } else {
            m_layersCount = 0;
        }
        framesNumLbl->setText(i18nc("@label %1 number of animation frames", "Frames: %1", m_layersCount));

        slotRecalculateRanks();
    }
}

void KisWdgOptionsBrush::slotEnableSelectionMethod(int value)
{
    animStyleGroup->setEnabled(value == BrushStyle::Animated);
}

void KisWdgOptionsBrush::slotActivateDimensionRanks()
{
    QLayoutItem *item;
    BrushPipeSelectionModeHelper *bp;
    int dim = this->dimensionSpin->value();

    for (int i = 0; i < qMax(dim, m_currentDimensions); i++) {
        bool enabled = i < dim;
        if ((item = dimRankLayout->itemAt(i))) {
            bp = dynamic_cast<BrushPipeSelectionModeHelper*>(item->widget());
            bp->setEnabled(enabled);
            bp->setVisible(enabled);
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
            currentBrushHelper->rankSpinBox.setValue(currentValue - 1);
            rankSum -= currentValue;
        }
    }

    if (rankSum > m_layersCount) {
        callerBrushHelper->rankSpinBox.setValue(m_layersCount);
    }

    if (rankSum == 0) {
        bp.at(0)->rankSpinBox.setValue(m_layersCount);
    }
}
