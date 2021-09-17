/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisAutoLevelsWidget.h"

KisAutoLevelsWidget::KisAutoLevelsWidget(QWidget *parent)
    : QWidget(parent)
{
    m_ui.setupUi(this);

    m_ui.sliderShadowsClipping->setSuffix(i18n("%"));
    m_ui.sliderShadowsClipping->setRange(0.0, 10.0, 2);
    m_ui.sliderShadowsClipping->setValue(0.1);
    m_ui.sliderShadowsClipping->setSingleStep(0.1);

    m_ui.sliderHighlightsClipping->setSuffix(m_ui.sliderShadowsClipping->suffix());
    m_ui.sliderHighlightsClipping->setRange(0.0, 10.0, 2);
    m_ui.sliderHighlightsClipping->setValue(0.1);
    m_ui.sliderHighlightsClipping->setSingleStep(0.1);

    m_ui.sliderShadowsAndLightsMaximumOffset->setSuffix(m_ui.sliderShadowsClipping->suffix());
    m_ui.sliderShadowsAndLightsMaximumOffset->setRange(0.0, 100.0, 2);
    m_ui.sliderShadowsAndLightsMaximumOffset->setValue(100.0);

    m_ui.sliderMidtonesAdjustmentAmount->setSuffix(m_ui.sliderShadowsClipping->suffix());
    m_ui.sliderMidtonesAdjustmentAmount->setRange(0.0, 100.0, 2);
    m_ui.sliderMidtonesAdjustmentAmount->setValue(50.0);

    connect(m_ui.comboBoxShadowsAndLightsMethod, SIGNAL(currentIndexChanged(int)), SIGNAL(parametersChanged()));
    connect(m_ui.sliderShadowsClipping, SIGNAL(valueChanged(qreal)), SIGNAL(parametersChanged()));
    connect(m_ui.sliderHighlightsClipping, SIGNAL(valueChanged(qreal)), SIGNAL(parametersChanged()));
    connect(m_ui.sliderShadowsAndLightsMaximumOffset, SIGNAL(valueChanged(qreal)), SIGNAL(parametersChanged()));
    connect(m_ui.comboBoxMidtonesMethod, SIGNAL(currentIndexChanged(int)), SIGNAL(parametersChanged()));
    connect(m_ui.sliderMidtonesAdjustmentAmount, SIGNAL(valueChanged(qreal)), SIGNAL(parametersChanged()));
    connect(m_ui.buttonShadowsColor, SIGNAL(changed(const KoColor&)), SIGNAL(parametersChanged()));
    connect(m_ui.buttonHighlightsColor, SIGNAL(changed(const KoColor&)), SIGNAL(parametersChanged()));
    connect(m_ui.buttonMidtonesColor, SIGNAL(changed(const KoColor&)), SIGNAL(parametersChanged()));
}

KisAutoLevelsWidget::~KisAutoLevelsWidget()
{}

KisAutoLevels::ShadowsAndHighlightsAdjustmentMethod KisAutoLevelsWidget::shadowsAndHighlightsAdjustementMethod() const
{
    return static_cast<KisAutoLevels::ShadowsAndHighlightsAdjustmentMethod>(m_ui.comboBoxShadowsAndLightsMethod->currentIndex());
}

qreal KisAutoLevelsWidget::shadowsClipping() const
{
    return m_ui.sliderShadowsClipping->value();
}

qreal KisAutoLevelsWidget::highlightsClipping() const
{
    return m_ui.sliderHighlightsClipping->value();
}

qreal KisAutoLevelsWidget::maximumInputBlackAndWhiteOffset() const
{
    return m_ui.sliderShadowsAndLightsMaximumOffset->value();
}

KisAutoLevels::MidtonesAdjustmentMethod KisAutoLevelsWidget::midtonesAdjustmentMethod() const
{
    return static_cast<KisAutoLevels::MidtonesAdjustmentMethod>(m_ui.comboBoxMidtonesMethod->currentIndex());
}

qreal KisAutoLevelsWidget::midtonesAdjustmentAmount() const
{
    return m_ui.sliderMidtonesAdjustmentAmount->value();
}

KoColor KisAutoLevelsWidget::outputShadowsColor() const
{
    return m_ui.buttonShadowsColor->color();
}

KoColor KisAutoLevelsWidget::outputHighlightsColor() const
{
    return m_ui.buttonHighlightsColor->color();
}

KoColor KisAutoLevelsWidget::outputMidtonesColor() const
{
    return m_ui.buttonMidtonesColor->color();
}

void KisAutoLevelsWidget::setShadowsAndHighlightsAdjustementMethod(KisAutoLevels::ShadowsAndHighlightsAdjustmentMethod newMethod)
{
    m_ui.comboBoxShadowsAndLightsMethod->setCurrentIndex(static_cast<int>(newMethod));
}

void KisAutoLevelsWidget::setShadowsClipping(qreal newShadowsClipping)
{
    m_ui.sliderShadowsClipping->setValue(newShadowsClipping);
}

void KisAutoLevelsWidget::setHighlightsClipping(qreal newHighlightsClipping)
{
    m_ui.sliderHighlightsClipping->setValue(newHighlightsClipping);
}

void KisAutoLevelsWidget::setMaximumInputBlackAndWhiteOffset(qreal newMaximumInputBlackAndWhiteOffset)
{
    m_ui.sliderShadowsAndLightsMaximumOffset->setValue(newMaximumInputBlackAndWhiteOffset);
}

void KisAutoLevelsWidget::setMidtonesAdjustmentMethod(KisAutoLevels::MidtonesAdjustmentMethod newMethod)
{
    m_ui.comboBoxMidtonesMethod->setCurrentIndex(static_cast<int>(newMethod));
}

void KisAutoLevelsWidget::setMidtonesAdjustmentAmount(qreal newMidtonesAdjustmentAmount)
{
    m_ui.sliderMidtonesAdjustmentAmount->setValue(newMidtonesAdjustmentAmount);
}

void KisAutoLevelsWidget::setShadowsColor(const KoColor &newShadowsColor)
{
    m_ui.buttonShadowsColor->setColor(newShadowsColor);
}

void KisAutoLevelsWidget::setHighlightsColor(const KoColor &newHighlightsColor)
{
    m_ui.buttonHighlightsColor->setColor(newHighlightsColor);
}

void KisAutoLevelsWidget::setMidtonesColor(const KoColor &newMidtonesColor)
{
    m_ui.buttonMidtonesColor->setColor(newMidtonesColor);
}

void KisAutoLevelsWidget::lockShadowsAndHighlightsAdjustementMethod()
{
    m_ui.comboBoxShadowsAndLightsMethod->setEnabled(false);
}

void KisAutoLevelsWidget::unlockShadowsAndHighlightsAdjustementMethod()
{
    m_ui.comboBoxShadowsAndLightsMethod->setEnabled(true);
}
