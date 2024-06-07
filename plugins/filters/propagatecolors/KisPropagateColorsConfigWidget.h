/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PROPAGATE_COLORS_CONFIG_WIDGET_H
#define KIS_PROPAGATE_COLORS_CONFIG_WIDGET_H

#include <kis_config_widget.h>

#include <QScopedPointer>

class KisPropagateColorsConfigWidget : public KisConfigWidget
{
    Q_OBJECT
public:
    KisPropagateColorsConfigWidget(QWidget *parent);
    ~KisPropagateColorsConfigWidget() override;

    KisPropertiesConfigurationSP configuration() const override;
    void setConfiguration(const KisPropertiesConfigurationSP config) override;

private:
    class Private;
    QScopedPointer<Private> m_d;
};

#endif
