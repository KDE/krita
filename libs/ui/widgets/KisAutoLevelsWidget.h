/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_AUTO_LEVELS_WIDGET_H
#define KIS_AUTO_LEVELS_WIDGET_H

#include <QScopedPointer>
#include <QWidget>

#include <KoColor.h>
#include <KisAutoLevels.h>
#include <kritaui_export.h>

#include "ui_KisAutoLevelsWidget.h"

/**
 * @brief A widget that allows to select a combination of auto levels parameters
 */
class KRITAUI_EXPORT KisAutoLevelsWidget : public QWidget
{
    Q_OBJECT

public:
    KisAutoLevelsWidget(QWidget *parent);
    ~KisAutoLevelsWidget() override;

    /**
     * @brief Get the method used to adjust the contrast
     */
    KisAutoLevels::ShadowsAndHighlightsAdjustmentMethod shadowsAndHighlightsAdjustementMethod() const;
    /**
     * @brief Get the normalized percentage used to clip the shadows
     */
    qreal shadowsClipping() const;
    /**
     * @brief Get the normalized percentage used to clip the highlights
     */
    qreal highlightsClipping() const;
    /**
     * @brief Get the normalized maximum value that the black/white point can be
     * moved from the default position of 0.0/1.0
     */
    qreal maximumInputBlackAndWhiteOffset() const;
    /**
     * @brief Get the method used to adjust the midtones
     */
    KisAutoLevels::MidtonesAdjustmentMethod midtonesAdjustmentMethod() const;
    /**
     * @brief Get how much the reference midtone point is deviated from the
     * center of the histogram to the median/mean (depending on the midtone
     * adjustment method selected)
     */
    qreal midtonesAdjustmentAmount() const;
    /**
     * @brief Get the color used to move the shadows towards
     */
    KoColor outputShadowsColor() const;
    /**
     * @brief Get the color used to move the highlights towards
     */
    KoColor outputHighlightsColor() const;
    /**
     * @brief Get the color used to move the midtones towards
     */
    KoColor outputMidtonesColor() const;

public Q_SLOTS:
    /**
     * @brief Set the method used to adjust the contrast
     */
    void setShadowsAndHighlightsAdjustementMethod(KisAutoLevels::ShadowsAndHighlightsAdjustmentMethod newMethod);
    /**
     * @brief Set the normalized percentage used to clip the shadows
     */
    void setShadowsClipping(qreal newShadowsClipping);
    /**
     * @brief Set the normalized percentage used to clip the highlights
     */
    void setHighlightsClipping(qreal newHighlightsClipping);
    /**
     * @brief Set the normalized maximum value that the black/white point can be
     * moved from the default position of 0.0/1.0
     */
    void setMaximumInputBlackAndWhiteOffset(qreal newMaximumInputBlackAndWhiteOffset);
    /**
     * @brief Set the method used to adjust the midtones
     */
    void setMidtonesAdjustmentMethod(KisAutoLevels::MidtonesAdjustmentMethod newMethod);
    /**
     * @brief Set how much the reference midtone point is deviated from the
     * center of the histogram to the median/mean (depending on the midtone
     * adjustment method selected)
     */
    void setMidtonesAdjustmentAmount(qreal newMidtonesAdjustmentAmount);
    /**
     * @brief Set the color used to move the shadows towards
     */
    void setShadowsColor(const KoColor &newShadowsColor);
    /**
     * @brief Set the color used to move the highlights towards
     */
    void setHighlightsColor(const KoColor &newHighlightsColor);
    /**
     * @brief Set the color used to move the midtones towards
     */
    void setMidtonesColor(const KoColor &newMidtonesColor);
    /**
     * @brief Disables the contrast method combo box. Use this when only one of
     * the methods makes sense
     */
    void lockShadowsAndHighlightsAdjustementMethod();
    /**
     * @brief Enables the contrast method combo box, allowing the user to choose
     * any of the methods
     */
    void unlockShadowsAndHighlightsAdjustementMethod();

Q_SIGNALS:
    void parametersChanged();

private:
    Ui_KisAutoLevelsWidget m_ui;
};

#endif
