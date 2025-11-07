/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2025 Agata Cacko <cacko.azh@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_COLOR_SAMPLER_PREVIEW_PREVIEW_H
#define KIS_COLOR_SAMPLER_PREVIEW_PREVIEW_H

#include <QLabel>

#include <kritaui_export.h>


/**
 * @brief A widget that allows to select a combination of auto levels parameters
 */
class KRITAUI_EXPORT KisColorSamplerPreviewPreview : public QLabel
{
    Q_OBJECT

public:
    KisColorSamplerPreviewPreview(QWidget *parent);


    void setDiameter(int diameter);
    void setOutlineEnabled(bool enabled);
    void setThickness(qreal thickness);


    void paintEvent(QPaintEvent *e) override;




private:
    int m_diameter {200};
    qreal m_thickness {0.1};
    bool m_outlineEnabled {true};


    QColor m_sampledColorTop {QColor::fromRgb(67, 183, 120)};
    QColor m_sampledColorBottom {QColor::fromRgb(67, 154, 120)};
    QColor m_outlineColor {QColor::fromRgbF(0.5, 0.5, 0.5)};

    static constexpr qreal m_outlineThickness {1.0};
    static constexpr qreal m_roundingMargin {3};
    static constexpr qreal m_verticalOutlineMargins {10.0};
    static constexpr qreal m_crossLength {8.0};
    static constexpr qreal m_crossSpace {2.0};
    // how much margin of the widget background color is around
    // to distinguish this widget from interactable ones
    static constexpr int m_horizontalWidgetMargins {20};

};

#endif // KIS_COLOR_SAMPLER_PREVIEW_PREVIEW_H
