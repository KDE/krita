/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_GRADIENT_COLOR_EDITOR_H
#define KIS_GRADIENT_COLOR_EDITOR_H

#include <QWidget>
#include <QScopedPointer>

#include <KisGradientWidgetsUtils.h>
#include <KoColor.h>
#include <kritaui_export.h>

class KRITAUI_EXPORT KisGradientColorEditor : public QWidget
{
    Q_OBJECT

public:
    KisGradientColorEditor(QWidget *parent = nullptr);
    KisGradientColorEditor(const KisGradientColorEditor &other);
    ~KisGradientColorEditor();

    qreal position() const;
    KisGradientWidgetsUtils::ColorType colorType() const;
    bool transparent() const;
    KoColor color() const;
    qreal opacity() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public Q_SLOTS:
    void setPosition(qreal position);
    void setColorType(KisGradientWidgetsUtils::ColorType type);
    void setTransparent(bool checked);
    void setColor(KoColor color);
    void setOpacity(qreal opacity);

    void setUseTransParentCheckBox(bool use);
    void setUsePositionSlider(bool use);
    void setPositionSliderEnabled(bool enabled);
    
Q_SIGNALS:
    void positionChanged(qreal position);
    void colorTypeChanged(KisGradientWidgetsUtils::ColorType type);
    void transparentToggled(bool checked);
    void colorChanged(KoColor color);
    void opacityChanged(qreal opacity);

private:
    class Private;
    QScopedPointer<Private> m_d;
};

#endif
