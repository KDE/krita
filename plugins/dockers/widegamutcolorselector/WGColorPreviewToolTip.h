/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef WGCOLORPREVIEWTOOLTIP_H
#define WGCOLORPREVIEWTOOLTIP_H

#include <QIcon>
#include <QWidget>

/* Code based on KisColorPreviewPopup in kis_color_selector_base.cpp
 * from Advanced Color Selector.
 * Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at> */

class WGColorPreviewToolTip : public QWidget
{
    Q_OBJECT
public:
    explicit WGColorPreviewToolTip(QWidget *parent = nullptr);

    void show(const QWidget *focus = nullptr)
    {
        updatePosition(focus);
        QWidget::show();
    }

    void updatePosition(const QWidget *focus);

    void setCurrentColor(const QColor& color)
    {
        m_color = color;
        update();
    }

    void setPreviousColor(const QColor& color)
    {
        m_previousColor = color;
        update();
    }

    void setLastUsedColor(const QColor& color)
    {
        m_lastUsedColor = color;
        update();
    }
    static qreal estimateBrightness(QColor col);

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    QColor m_color;
    QColor m_previousColor;
    QColor m_lastUsedColor;
    QIcon m_brushIcon;
};

#endif // WGCOLORPREVIEWTOOLTIP_H
